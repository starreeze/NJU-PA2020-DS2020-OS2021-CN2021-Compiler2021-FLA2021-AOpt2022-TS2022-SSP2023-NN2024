#include "cpu/instr.h"
/*
Put the implementations of `test' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	alu_and(opr_src.val, opr_dest.val, opr_src.data_size);
}

make_instr_impl_2op(test, r, rm, b)
make_instr_impl_2op(test, r, rm, v)
make_instr_impl_2op(test, rm, r, b)
make_instr_impl_2op(test, rm, r, v)
make_instr_impl_2op(test, i, rm, b)
make_instr_impl_2op(test, i, rm, v)
make_instr_impl_2op(test, i, r, b)
make_instr_impl_2op(test, i, r, v)

make_instr_func(test_al) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = 8;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    alu_and(imm.val, cpu.gpr[0]._8[0], 8);
    
    return 2;
}
make_instr_func(test_eax) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = data_size;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    if(data_size == 32)
        alu_and(imm.val, cpu.eax, data_size);
    else    alu_and(imm.val, cpu.gpr[0]._16, data_size);
    
    return 1 + data_size / 8;
}