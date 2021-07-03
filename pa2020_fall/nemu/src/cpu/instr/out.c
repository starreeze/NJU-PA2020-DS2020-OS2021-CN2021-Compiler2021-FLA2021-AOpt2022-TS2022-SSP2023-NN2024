#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/
make_instr_func(out_i2a_b) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.data_size = 8;
    imm.addr = eip + 1;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    pio_write(imm.val, 1, cpu.gpr[0]._8[0]);
    return 2;
}

make_instr_func(out_i2a_v) {
    OPERAND imm;
    imm.type = OPR_IMM;
    imm.data_size = 8;
    imm.addr = eip + 1;
    imm.sreg = SREG_CS;
    
    operand_read(&imm);
    if(data_size == 16)
        pio_write(imm.val, 2, cpu.gpr[0]._16);
    else    pio_write(imm.val, 4, cpu.eax);
    return 2;
}

make_instr_func(out_d2a_b) {
    print_asm_0("out", "b", 1);
    pio_write(cpu.edx, 1, cpu.eax);
    return 1;
}

make_instr_func(out_d2a_v) {
    print_asm_0("out", (data_size == 16) ? "w" : "l", 1);
    pio_write(cpu.edx, data_size / 8, cpu.eax);
    return 1;
}
