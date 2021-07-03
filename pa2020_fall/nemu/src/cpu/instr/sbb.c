#include "cpu/instr.h"
/*
Put the implementations of `sbb' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	opr_dest.val = alu_sbb(opr_src.val, opr_dest.val, opr_dest.data_size);
	operand_write(&opr_dest);
}

make_instr_impl_2op(sbb, r, rm, b)
make_instr_impl_2op(sbb, r, rm, v)
make_instr_impl_2op(sbb, rm, r, b)
make_instr_impl_2op(sbb, rm, r, v)
make_instr_impl_2op(sbb, i, rm, b)
make_instr_impl_2op(sbb, i, rm, bv)
make_instr_impl_2op(sbb, i, rm, v)
make_instr_impl_2op(sbb, i, r, b)
make_instr_impl_2op(sbb, i, r, v)
make_instr_impl_2op(sbb, i, r, bv)

make_instr_func(sbb_al) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = 8;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    cpu.gpr[0]._8[0] = alu_sbb(imm.val, cpu.gpr[0]._8[0], 8);
    
    return 2;
}

make_instr_func(sbb_eax) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = data_size;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    if(data_size == 32)
        cpu.eax = alu_sbb(imm.val, cpu.eax, data_size);
    else    cpu.gpr[0]._16 = alu_sbb(imm.val, cpu.gpr[0]._16, data_size);
    
    return 1 + data_size / 8;
}