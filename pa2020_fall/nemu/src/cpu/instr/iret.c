#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/
make_instr_func(iret) {
    cpu.eflags.val = vaddr_read(cpu.esp + 8, SREG_SS, 4);
    cpu.cs.val = vaddr_read(cpu.esp + 4, SREG_SS, 2);
    cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
    cpu.esp += 12;
    return 0;
}