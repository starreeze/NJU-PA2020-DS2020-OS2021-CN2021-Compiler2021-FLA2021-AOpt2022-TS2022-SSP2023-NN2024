#include "semantic.h"
#include <unordered_set>

static bool inside_block(const InterCode& code) {
	return code.type == ASSIGN || code.type == ARITH || code.type == DEC || code.type == READ || code.type == WRITE || code.type == ARG || code.type == CALL;
}

static Oprand imm_calc(const Oprand& op1, const Oprand& op, const Oprand& op2) {
	int num1 = atoi(op1.info.c_str()), num2 = atoi(op2.info.c_str()), res;
	switch (op.info[0]) {
	case '+':
		res = num1 + num2; break;
	case '-':
		res = num1 - num2; break;
	case '*':
		res = num1 * num2; break;
	case '/':
		res = num1 / num2; break;
	default:
		assert(0);
	}
	return Oprand(to_string(res), IMMEDIATE);
}

static bool src_assign_prop(const Oprand& prevDst, Oprand prevSrc, Oprand& curSrc) {
	// replace current src according to the last assign
	bool res = false;
	// nagetive if take value, e.g. (&&a) = 2; (*&*a) = -1
	int addressDepth = curSrc.address - curSrc.value - prevDst.address + prevDst.value + prevSrc.address - prevSrc.value;
	// only optimize if 'a' is tempvar, otherwise 'a = &b' cannot be removed
	if (abs(addressDepth) <= 1 && prevDst.is_temp()) {
		if (addressDepth == 1) {
			prevSrc.address = true; prevSrc.value = false;
		}
		else if (addressDepth == -1) {
			prevSrc.value = true; prevSrc.address = false;
		}
		else {
			prevSrc.value = false; prevSrc.address = false;
		}
		curSrc = prevSrc;
		res = true;
	}
	return res;
}
#define PROCESS_SRC_ASSIGN(src) { \
	auto p = dst2code.equal_range(src); \
	for (auto it = p.first; it != p.second; ++it) { \
		It pcode = it->second; \
		if (pcode->type == ASSIGN && src_assign_prop(it->first, pcode->oprands[1], src)) { \
			res = true; break; \
		} \
	} \
}
static bool variable_propagation(It start, It end) {
	// also do immediate number calculation; return true if optimized something
	// current: ASSIGN, ARITH, ARG, RETURN, IF_GOTO; previous: ASSIGN, ARITH
	unordered_multimap<Oprand, It, OpHash> dst2code; // only ASSIGN, ARITH (already optimized)
	bool res = false;
	for (++start; start != end; ++start) {
		auto& ops = start->oprands;
		switch (start->type) {
		case ASSIGN: { // previous can be ASSIGN, ARITH
			assert(ops.size() == 2);
			auto p = dst2code.equal_range(ops[1]);
			for (auto it = p.first; it != p.second; ++it) {
				It pcode = it->second; // pointer to intercode with specified dst(ops[1])
				if (pcode->type == ASSIGN) {
					// 'a = &b; c = a;' --> 'a = &b; c = &b'
					// '*a = &b; c = *a;' --> '*a = &b; c = &b'
					// 'a = *b; c = &a;' --> 'a = *b; c = b;'
					//  |dst |src    |ops[1]
					if (src_assign_prop(it->first, pcode->oprands[1], ops[1])) {
						res = true; break;
					}
				}
				else if (pcode->type == ARITH) {
					// 'a = &b + *d; c = a;' --> 'c = &b + *d' (no other form of 'a' or 'c' allowed)
					// only optimize if 'a' is tempvar, otherwise 'a = &b + *d' cannot be removed
					if (it->first.address || it->first.value || ops[1].address || ops[1].value || !it->first.is_temp() || ops[0].value || ops[0].address)
						continue;
					Oprand dst = ops[0];
					*start = *pcode;
					start->oprands[0] = dst;
					res = true; break;
				}
				else    assert(0);
			}
			// 'a = &b; ...; *a = c' --> '...; b = c'
			p = dst2code.equal_range(ops[0]);
			for (auto it = p.first; it != p.second; ++it) {
				It pcode = it->second;
				if (ops[0].value && pcode->oprands[1].address && !ops[1].address && !ops[1].value && !it->first.address && !it->first.value) {
					ops[0] = pcode->oprands[1];
					ops[0].address = false;
					intercodes.erase(pcode);
					dst2code.erase(it);
					res = true; break;
				}
			}
			dst2code.insert({ start->oprands[0], start });
			break; }
		case ARITH: // below: previous can only be ASSIGN
			assert(ops.size() == 4);
			PROCESS_SRC_ASSIGN(ops[1]);
			PROCESS_SRC_ASSIGN(ops[3]);
			if (ops[1].type == IMMEDIATE && ops[3].type == IMMEDIATE) {
				Oprand result = imm_calc(ops[1], ops[2], ops[3]);
				*start = { {ops[0], result}, ASSIGN };
			}
			dst2code.insert({ start->oprands[0], start });
			break;
		case ARG:
		case RETURN:
		case WRITE:
			assert(ops.size() == 1);
			PROCESS_SRC_ASSIGN(ops[0]);
			break;
		case IF_GOTO:
			assert(ops.size() == 4);
			PROCESS_SRC_ASSIGN(ops[0]);
			PROCESS_SRC_ASSIGN(ops[2]);
			break;
		}
	}
	return res;
}

static void common_subexp(It start, It end) {
	// complexity: N^2
	for (It i = next(start); i != end; ++i) {
		if (i->type != ARITH)
			continue;
		auto& opsi = i->oprands;
		assert(opsi.size() == 4);
		// search for cse in previous
		for (It j = next(start); j != i; ++j) {
			auto& opsj = j->oprands;
			if (!(j->type == ARITH && opsj[1].congruent(opsi[1]) && opsj[2].congruent(opsi[2]) && opsj[3].congruent(opsi[3])))
				continue;
			// erase & replace
			Oprand dsti = opsi[0];
			i = prev(intercodes.erase(i));
			for (It k = next(i); k != end; ++k)
				for (auto& op : k->oprands)
					if (op == dsti)
						op.info = opsj[0].info;
			break;
		}
	}
}

static bool deadcode_elimination(It start, It end) {
	bool res = false;
	unordered_set<Oprand, OpHash> srcs;
	for (--end; end != start; --end) {
		auto& ops = end->oprands;
		switch (end->type) {
		case ASSIGN:
		case ARITH: {
		   // tempvar will not cross basic blocks
			if (ops[0].is_temp() && !ops[0].value && srcs.find(ops[0]) == srcs.end()) {
				// not used and is tempvar and not array ref
				end = intercodes.erase(end);
				res = true; break;
			}
			if (ops[0].value) // x is useful if there is '*x = a' after
				srcs.insert(ops[0]);
			if (end->type == ARITH)
				srcs.insert(ops[3]);
			srcs.insert(ops[1]);
			break; }
		case ARG:
		case RETURN:
		case WRITE:
			srcs.insert(ops[0]);
			break;
		case IF_GOTO:
			srcs.insert(ops[0]);
			srcs.insert(ops[2]);
			break;
		}
	}
	return res;
}

extern "C" void optimize_intercode() {
	It start = intercodes.begin(), end = start;
	// basic block partition: (start, end) OPEN interval
	while (end != intercodes.end()) {
		for (; end != intercodes.end() && !inside_block(*end); ++end);
		start = end; // first code in block
		for (++end; end != intercodes.end() && inside_block(*end); ++end);
		// last code : LABEL, GOTO or RETURN
		if (end != intercodes.end())    ++end;
		--start;
		// optimize for basic block (start, end)
		variable_propagation(start, end);
		// for (auto& code : intercodes)
		// 	code.print();
		// fflush(stdout);
		deadcode_elimination(start, end);
		common_subexp(start, end);
		start = end;
	}
}
