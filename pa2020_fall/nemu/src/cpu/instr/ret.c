#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near) {
    cpu.eip = vaddr_read(cpu.esp, SREG_CS, 4);
    cpu.esp += 4;
    return 0;
}

make_instr_func(ret_neari) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.sreg = SREG_CS;
    imm.addr = eip + 1;
    imm.data_size = 16;
    operand_read(&imm);
    
    cpu.eip = vaddr_read(cpu.esp, SREG_CS, 4);
    cpu.esp += 4 + imm.val;
    return 0;
}
