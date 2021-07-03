#include "cpu/instr.h"
/*
Put the implementations of `cmp' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	alu_sub(sign_ext(opr_src.val, opr_src.data_size), opr_dest.val, opr_dest.data_size);
}

make_instr_impl_2op(cmp, r, rm, b)
make_instr_impl_2op(cmp, r, rm, v)
make_instr_impl_2op(cmp, rm, r, b)
make_instr_impl_2op(cmp, rm, r, v)
make_instr_impl_2op(cmp, i, rm, b)
make_instr_impl_2op(cmp, i, rm, bv)
make_instr_impl_2op(cmp, i, rm, v)
make_instr_impl_2op(cmp, i, r, b)
make_instr_impl_2op(cmp, i, r, v)
make_instr_impl_2op(cmp, i, r, bv)

make_instr_func(cmp_al) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = 8;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    alu_sub(imm.val, cpu.gpr[0]._8[0], 8);
    
    return 2;
}
make_instr_func(cmp_eax) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = data_size;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    if(data_size == 32)
        alu_sub(imm.val, cpu.eax, data_size);
    else    alu_sub(imm.val, cpu.gpr[0]._16, data_size);
    
    return 1 + data_size / 8;
}