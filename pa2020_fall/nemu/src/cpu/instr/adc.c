#include "cpu/instr.h"
/*
Put the implementations of `adc' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	opr_dest.val = alu_adc(opr_src.val, opr_dest.val, opr_dest.data_size);
	operand_write(&opr_dest);
}

make_instr_impl_2op(adc, r, rm, b)
make_instr_impl_2op(adc, r, rm, v)
make_instr_impl_2op(adc, rm, r, b)
make_instr_impl_2op(adc, rm, r, v)
make_instr_impl_2op(adc, i, rm, b)
make_instr_impl_2op(adc, i, rm, bv)
make_instr_impl_2op(adc, i, rm, v)
make_instr_impl_2op(adc, i, r, b)
make_instr_impl_2op(adc, i, r, v)
make_instr_impl_2op(adc, i, r, bv)

make_instr_func(adc_al) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = 8;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    cpu.gpr[0]._8[0] = alu_adc(imm.val, cpu.gpr[0]._8[0], 8);
    
    return 2;
}
make_instr_func(adc_eax) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    imm.data_size = data_size;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    if(data_size == 32)
        cpu.eax = alu_adc(imm.val, cpu.eax, data_size);
    else    cpu.gpr[0]._16 = alu_adc(imm.val, cpu.gpr[0]._16, data_size);
    
    return 1 + data_size / 8;
}