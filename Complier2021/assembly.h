#ifndef ASM_H
#define ASM_H
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "semantic.h"
using namespace std;

/****** rules on how to use reg:
 * V0, V1 for return value
 * A, T, S for tempvars
 * parameters on stack
 * all regs caller-saved (CALL is an independent bb)
 ****** rules on stack structure:
 * ...
 * saved tmp
 * old ra
 * param k
 * ...
 * param 1
 * old fp		<- fp
 * tempvar 1
 * ...
 * tempvar k
 *				<- sp
*/

enum Reg {
	ZERO, AT,
	V0, V1,
	A0, A1, A2, A3,
	T0, T1, T2, T3, T4, T5, T6, T7,
	S0, S1, S2, S3, S4, S5, S6, S7,
	T8, T9,
	K0, K1,
	GP, SP, FP, RA,
	NONE = -1
};
// A0, A1 ... is the same as T,S since we use stack to pass on parameters
const pair<int, int> reg_use = { 4, 26 };
const string reg_name[] = {
	"$0",  "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
	"$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5",
	"$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};

extern vector<string> asmCodes;
#define GEN asmCodes.push_back
#define IS_NUM(str) (str[0] >= '0' && str[0] <= '9')

class VarInfo : public unordered_map<string, pair<Reg, int>> {
#ifndef NDEBUG
public:
	pair<Reg, int>& operator[](const string& s) {
		return at(s);
	}
#endif
};

class VarTable { // record for tempvars & reg allocation; there should be one in each scope
	vector<string> regs = vector<string>(36); // reg -> variable name; null if free
	// variable name -> register, memory location relative to FP (0 if only on reg since 0 should always be old FP)
	VarInfo vars;
	// allocate a new reg for var; replace one if full; allocate memory if non-tmp;
	// create asm code in the process
	Reg allocate_reg(const string& var, It current, It end, bool isNewVar) {
		int i = reg_use.first, res;
		for (; i < reg_use.second; ++i)
			if (regs[i].empty())
				break;
		if (i != reg_use.second)
			res = i;
		else { // find a reg that won't be used soon, O(n)
			int startDiff = reg_use.first;
			vector<int> dist(reg_use.second - startDiff, 0x7fffffff);
			++current;
			for (int count = 1; current != end; ++current, ++count)
				for (int i = 1; i < current->oprands.size(); ++i) {
					auto& op = current->oprands[i];
					Reg reg = NONE;
					if (vars.find(op.info) != vars.end())
						reg = vars[op.info].first;
					if (reg != NONE)
						dist[reg - startDiff] = min(dist[reg - startDiff], count);
				}
			res = max_element(dist.begin(), dist.end()) - dist.begin() + startDiff;
		}
		regs[res] = var;
		if (!isNewVar) // memory has been allocated before
			GEN("lw " + reg_name[res] + ", " + to_string(vars[var].second) + "($fp)");
		else if (var.back() != '\'' && !vars[var].second) {
			// non-tmp variable: allocate memory at the same time
			GEN("addi $sp, $sp, -4");
			vars[var].second = -(fpDiff += 4);
		}
		return vars[var].first = Reg(res);
	}
public:
	int fpDiff = 0;  // the difference of the address of LAST tempvar and FP (positive)
	VarTable() {
		vars.insert({ "_", {NONE, 0} });
		vars.insert({ "__", {NONE, 0} });
	}
	// allocate memory for a block (including array or struct), return HEAD address
	int allocate_mem(const string& size) {
		GEN("addi $sp, $sp, -" + size);
		return -(fpDiff += atoi(size.c_str()));
	}
	// set memory location for a var
	void set_memloc(const string& var, int memloc) {
		assert(vars.find(var) == vars.end());
		vars.insert({ var, {NONE, memloc} });
	}
	// get the allocated reg for var, NONE if nonexist
	Reg query_reg(const string& var) {
		assert((vars.find(var) != vars.end()));
		assert(vars[var].first != NONE);
		return vars[var].first;
	}
	// get the allocated reg for var or allocate a new one
	Reg get_reg(const string& var, It current, It end, bool isNewVar) {
		assert(!IS_NUM(var));
		if (vars.find(var) == vars.end())
			vars.insert({ var, {NONE, 0} });
		if (vars[var].first != NONE)
			return vars[var].first;
		return allocate_reg(var, current, end, isNewVar);
	}
	// create a tmp reg t, e.g. *x = *y -> t = *y; *x = t. At most 2 tmpReg: x = *y + *z
	Reg create_tmp_reg(It current, It end) {
		assert(vars["__"].first == NONE);
		if (vars["_"].first == NONE)
			return vars["_"].first = allocate_reg("_", current, end, true);
		return vars["__"].first = allocate_reg("__", current, end, true);
	}
	void delete_tmp_reg() {
		Reg& tmp = vars["_"].first;
		if (tmp != NONE) {
			regs[tmp].clear();
			tmp = NONE;
		}
		Reg& tmp2 = vars["__"].first;
		if (tmp2 != NONE) {
			regs[tmp2].clear();
			tmp2 = NONE;
		}
	}
	// query the allocated memaddr for var
	int query_mem(const string& var, Reg reg) {
		assert((vars.find(var) != vars.end())); // the entry should've been created in get_reg
		assert(var.back() != '\''); // tmpvar has no memory location
		assert(vars[var].second); // non-tmp must has a memory location
		return vars[var].second;
	}
	// store data to V1 & free reg
	void free_reg(Reg reg) {
		assert(regs[V1].empty());
		string& var = regs[reg];
		if (var.size()) {
			vars[var].first = V1;
			regs[V1] = var;
			// do not clear regs[reg] to ensure reg won't be used before restore
			GEN("move $v1, " + reg_name[reg]);
		}
	}
	void restore_reg(Reg reg) {
		string& var = regs[V1];
		if (var.size()) {
			vars[var].first = reg;
			regs[reg] = var;
			var.clear();
			GEN("move " + reg_name[reg] + ", $v1");
		}
	}
	// write back all non-tmp vars at the end of basic block & clear regs
	// only write back vars whose memory is positive if isFuncEnd == true
	void end_bb(bool isFuncEnd) {
		for (int i = reg_use.first; i < reg_use.second; ++i) {
			const string& var = regs[i];
			if (var.size()) {
				assert(vars.find(var) != vars.end());
				if (isFuncEnd && vars[var].second > 0 || !isFuncEnd && var.back() != '\'') {
					if (!vars[var].second)
						vars[var].second = allocate_mem("4");
					GEN("sw " + reg_name[i] + ", " + to_string(vars[var].second) + "($fp)");
				}
			}
		}
		clear_regs();
	}
	// save tmp and non-tmp vars without clearing regs, then save RA
	void save_all() {
		for (int i = reg_use.first; i < reg_use.second; ++i) {
			const string& var = regs[i];
			if (var.size()) {
				assert(vars.find(var) != vars.end());
				if (!vars[var].second)
					vars[var].second = allocate_mem("4");
				GEN("sw " + reg_name[i] + ", " + to_string(vars[var].second) + "($fp)");
			}
		}
		GEN("sw $ra, 0($sp)");
		GEN("addi $sp, $sp, -4");
		fpDiff += 4;
	}
	void clear_regs() {
		for (int i = reg_use.first; i < reg_use.second; ++i)
			regs[i].clear();
		for (auto& entry : vars)
			entry.second.first = NONE;
	}
};

#endif
