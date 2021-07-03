#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/
#ifdef IA32_SEG
make_instr_func(lgdt) {
    OPERAND limit;
    limit.data_size = 16;
    int len = modrm_rm(eip+1, &limit);
    operand_read(&limit);
    
    OPERAND base;
    base.data_size = 32;
    base.addr = limit.addr + 2;
    base.type = limit.type;
    operand_read(&base);
    
    cpu.gdtr.limit = limit.val;
    cpu.gdtr.base = base.val;
    return len+1;
}
#endif