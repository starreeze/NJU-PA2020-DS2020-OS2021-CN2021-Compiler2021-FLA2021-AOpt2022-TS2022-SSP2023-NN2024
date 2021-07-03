#include "cpu/instr.h"
//#define DEBUGINC
/*
Put the implementations of `inc' instructions here.
*/
static void instr_execute_1op() 
{
    #ifdef DEBUGINC
	printf("0x%x\n", paddr_read(0x7ffffe0, 4));
	#endif
    
	operand_read(&opr_src);
	bool carry = cpu.eflags.CF;
	opr_src.val = alu_add(opr_src.val, 1, opr_src.data_size);
	cpu.eflags.CF = carry;
	operand_write(&opr_src);
	
	#ifdef DEBUGINC
	printf("0x%x\n", paddr_read(0x7ffffe0, 4));
	#endif
}

make_instr_impl_1op(inc, r, b)
make_instr_impl_1op(inc, r, v)
make_instr_impl_1op(inc, rm, b)
make_instr_impl_1op(inc, rm, v)