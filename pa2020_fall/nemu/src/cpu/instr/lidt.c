#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/
#ifdef IA32_INTR
make_instr_func(lidt) {
    OPERAND limit;
    limit.data_size = 16;
    int len = modrm_rm(eip+1, &limit);
    operand_read(&limit);
    
    OPERAND base;
    base.data_size = 32;
    base.addr = limit.addr + 2;
    base.type = limit.type;
    operand_read(&base);
    
    cpu.idtr.limit = limit.val;
    cpu.idtr.base = base.val;
    return len+1;
}
#endif