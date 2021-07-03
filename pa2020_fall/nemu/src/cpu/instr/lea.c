#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/
extern uint32_t trun(uint32_t x, size_t s);

make_instr_func(lea_m2r_v) {
    OPERAND src, dest;
    src.type = OPR_MEM;
    src.data_size = data_size;
    src.sreg = SREG_SS;
    
    dest.type = OPR_REG;
    dest.data_size = data_size;
    dest.sreg = SREG_SS;
    
    int len = 1;
    len += modrm_r_rm(eip + 1, &dest, &src);
    
    dest.val = trun(src.addr, data_size);
    operand_write(&dest);
    
    return len;
}