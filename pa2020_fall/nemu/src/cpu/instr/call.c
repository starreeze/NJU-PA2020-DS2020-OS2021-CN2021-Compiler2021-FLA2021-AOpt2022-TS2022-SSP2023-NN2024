#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/
make_instr_func(call_i_near) {
    OPERAND near;
    near.type = OPR_IMM;
    near.addr = cpu.eip + 1;
    near.data_size = data_size;
    operand_read(&near);
    
    cpu.eip += data_size/8 + 1;
    
    if(data_size == 32) {
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_CS, 4, cpu.eip);
        cpu.eip += near.val;
    }
    else {
        cpu.esp -= 2;
        vaddr_write(cpu.esp, SREG_CS, 2, cpu.eip & 0xffff);
        cpu.eip = (cpu.eip + near.val) & 0xffff;
    }
    return 0;
}

make_instr_func(call_rm_near) {
    int len = 1;
    OPERAND near;
    near.data_size = data_size;
    len += modrm_rm(eip + 1, &near);
    operand_read(&near);
    
    cpu.eip += len;
    
    if(data_size == 32) {
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_CS, 4, cpu.eip);
        cpu.eip += near.val;
    }
    else {
        cpu.esp -= 2;
        vaddr_write(cpu.esp, SREG_CS, 2, cpu.eip & 0xffff);
        cpu.eip = (cpu.eip + near.val) & 0xffff;
    }
    return 0;
}

make_instr_func(call_near_indirect) {
    int len = 1;
    OPERAND near;
    near.data_size = data_size;
    len += modrm_rm(eip + 1, &near);
    operand_read(&near);
    
    cpu.eip += len;
    
    if(data_size == 32) {
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_CS, 4, cpu.eip);
        cpu.eip = near.val;
    }
    else {
        cpu.esp -= 2;
        vaddr_write(cpu.esp, SREG_CS, 2, cpu.eip & 0xffff);
        cpu.eip = near.val & 0xffff;
    }
    return 0;
}