#include "assembly.h"

vector<string> asmCodes{ ".globl main", ".text" };
unordered_map<string, string> sym2code = {
	{"==", "eq"},
	{"!=", "ne"},
	{">", "gt"},
	{"<", "lt"},
	{">=", "ge"},
	{"<=", "le"},
	{"+", "add"},
	{"-", "sub"},
	{"*", "mul"}
};
static VarTable* vtable = nullptr;
static int args_size = 0, param_size = 0, saved_tmp_size = 0;
#define OPREG(oprand, isNewVar) reg_name[vtable->get_reg(oprand.info, begin, end, isNewVar)]

static Reg eval_oprand(const Oprand& src, It begin, It end) {
	Reg res;
	if (src.type == IMMEDIATE) {
		res = vtable->create_tmp_reg(begin, end);
		GEN("li " + reg_name[res] + ", " + src.info);
	}
	else if (src.value) {
		res = vtable->create_tmp_reg(begin, end);
		GEN("lw " + reg_name[res] + ", 0(" + OPREG(src, false) + ")");
	}
	else if (src.address) {
		res = vtable->create_tmp_reg(begin, end);
		GEN("addi " + reg_name[res] + ", $fp, " + to_string(vtable->query_mem(src.info, vtable->query_reg(src.info))));
	}
	else    res = vtable->get_reg(src.info, begin, end, false);
	return res;
}

static void eval_oprand_assign(const Oprand& src, const string& dst, It begin, It end) {
	if (src.type == IMMEDIATE)
		GEN("li " + dst + ", " + src.info);
	else if (src.value)
		GEN("lw " + dst + ", 0(" + OPREG(src, false) + ")");
	else if (src.address)
		GEN("addi " + dst + ", $fp, " + to_string(vtable->query_mem(src.info, vtable->query_reg(src.info))));
	else    GEN("move " + dst + ", " + OPREG(src, false));
}

// remove the last ' character
static string label_name(const string& s) {
	return s.substr(0, s.size() - 1);
}
static string func_name(const string& s) {
	if (s == "main")
		return s;
	return "f_" + s;
}

static void handle_bb(It begin, It end) {
	for (; begin != end; ++begin) {
		auto& ops = begin->oprands;
		switch (begin->type) {
		case FUN:
			delete vtable;
			vtable = new VarTable;
			GEN(func_name(ops[0].info) + ":");
			GEN("sw $fp, 0($sp)");
			GEN("move $fp, $sp");
			GEN("addi $sp, $sp, -4");
			break;
		case LABEL:
			vtable->end_bb(false);
			GEN(label_name(ops[0].info) + ":");
			break;
		case GOTO:
			vtable->end_bb(false);
			GEN("j " + label_name(ops[0].info));
			break;
		case IF_GOTO: {
			Reg ra = eval_oprand(ops[0], begin, end), rb = eval_oprand(ops[2], begin, end);
			vtable->end_bb(false);
			GEN("b" + sym2code[ops[1].info] + " " + reg_name[ra] + ", " + reg_name[rb] + ", " + label_name(ops[3].info));
			break; }
		case RETURN:
			eval_oprand_assign(ops[0], "$v0", begin, end);
			vtable->end_bb(true); // store back in case of reference as arguments
			GEN("move $sp, $fp");
			GEN("lw $fp, 0($fp)");
			GEN("jr $ra");
			param_size = 0;
			break;
		case ASSIGN: {
			const auto& dst = ops[0], src = ops[1];
			assert(!dst.address);
			if (dst.value) {
				Reg srcReg = eval_oprand(src, begin, end);
				GEN("sw " + reg_name[srcReg] + ", 0(" + OPREG(dst, false) + ")");
				vtable->delete_tmp_reg();
			}
			else    eval_oprand_assign(src, OPREG(dst, true), begin, end);
			break; }
		case ARITH: { // '&' won't appear in arith; dst should not be '*'
			assert(!ops[0].value && !ops[0].address);
			string ra = reg_name[eval_oprand(ops[1], begin, end)];
			string rb = reg_name[eval_oprand(ops[3], begin, end)];
			if (ops[2].info == "/") {
				GEN("div " + ra + ", " + rb);
				GEN("mflo " + OPREG(ops[0], true));
			}
			else    GEN(sym2code[ops[2].info] + " " + OPREG(ops[0], true) + ", " + ra + ", " + rb);
			vtable->delete_tmp_reg();
			break; }
		case DEC:
			++begin;
			assert(begin->type == ASSIGN);
			GEN("addi " + OPREG(begin->oprands[0], true) + ", $fp, " + to_string(vtable->allocate_mem(ops[1].info)));
			break;
		case CALL:
			if (!args_size)
				vtable->save_all();
			vtable->clear_regs();
			GEN("jal " + func_name(ops[1].info));
			GEN("move " + OPREG(ops[0], true) + ", $v0");
			if (args_size) {
				GEN("addi $sp, $sp, " + to_string(args_size));
				vtable->fpDiff -= args_size;
				args_size = 0;
			}
			GEN("addi $sp, $sp, 4");
			vtable->fpDiff -= 4;
			GEN("lw $ra, 0($sp)");
			break;
		case ARG:
			if (!args_size)
				vtable->save_all();
			GEN("sw " + reg_name[eval_oprand(ops[0], begin, end)] + ", 0($sp)");
			vtable->delete_tmp_reg();
			GEN("addi $sp, $sp, -4");
			vtable->fpDiff += 4;
			args_size += 4;
			break;
		case PARAM:
			vtable->set_memloc(ops[0].info, param_size += 4);
			break;
		case READ:
			GEN("li $v0, 5");
			GEN("syscall");
			GEN("move " + OPREG(ops[0], true) + ", $v0");
			break;
		case WRITE:
			GEN("li $v0, 1");
			vtable->free_reg(A0);
			eval_oprand_assign(ops[0], "$a0", begin, end);
			GEN("syscall");
			vtable->restore_reg(A0);
		}
	}
}

extern "C" void gen_asm() {
	It start = intercodes.begin(), end = start;
	// basic block partition: [start, end) interval
	while (true) {
		++end;
		if (end == intercodes.end())
			break;
		if (end->type == LABEL || end->type == FUN || end->type == GOTO || end->type == IF_GOTO || end->type == RETURN) {
			handle_bb(start, next(end)); // skip one line of code since a bb is at least 2 lines
			start = next(end);
		}
	}
}

extern "C" void print_asm() {
	for (auto& code : asmCodes)
		printf("%s\n", code.c_str());
}
