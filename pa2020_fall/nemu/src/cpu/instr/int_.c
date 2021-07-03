#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/

make_instr_func(int_imm) {
    raise_sw_intr(vaddr_read(cpu.eip + 1, SREG_CS, 1)); 
    return 0; 
}
