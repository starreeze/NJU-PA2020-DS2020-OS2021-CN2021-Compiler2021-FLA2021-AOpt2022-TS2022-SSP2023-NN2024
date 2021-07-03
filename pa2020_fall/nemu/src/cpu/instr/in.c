#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/
make_instr_func(in_i2a_b) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.data_size = 8;
    imm.addr = eip + 1;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    cpu.gpr[0]._8[0] = pio_read(imm.val, 1);
    return 2;
}

make_instr_func(in_i2a_v) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.data_size = 8;
    imm.addr = eip + 1;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    if(data_size == 16)
        cpu.gpr[0]._16 = pio_read(imm.val, 2);
    else    cpu.eax = pio_read(imm.val, 4);
    return 2;
}

make_instr_func(in_d2a_b) {
    print_asm_0("in", "b", 1);
    cpu.eax = pio_read(cpu.edx, 1);
    return 1;
}

make_instr_func(in_d2a_v) {
    print_asm_0("in", (data_size == 16) ? "w" : "l", 1);
    cpu.eax = pio_read(cpu.edx, data_size / 8);
    return 1;
}
