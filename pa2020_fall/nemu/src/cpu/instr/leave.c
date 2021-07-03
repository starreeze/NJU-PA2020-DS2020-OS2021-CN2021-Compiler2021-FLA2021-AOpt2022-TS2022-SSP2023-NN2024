#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave) {
    if(data_size == 32) {
        cpu.esp = cpu.ebp;
        cpu.ebp = vaddr_read(cpu.esp, SREG_CS, data_size / 8);
        cpu.esp += 4;
    }
    else {
        cpu.gpr[4]._16 = cpu.gpr[5]._16;
        cpu.gpr[5]._16 = vaddr_read(cpu.esp, SREG_CS, data_size / 8);
        cpu.esp += 2;
    }
    return 1;
}