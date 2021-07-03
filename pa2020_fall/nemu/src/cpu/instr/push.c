#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op() 
{
	operand_read(&opr_src);
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
}

make_instr_impl_1op(push, r, b)
make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, rm, b)
make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)

make_instr_func(pusha) {
    assert(data_size == 32); 
	uint32_t tmp = cpu.esp; 
    cpu.esp -= 8*4; 
    vaddr_write(cpu.esp + 7*4, SREG_SS, 4, cpu.eax); 
    vaddr_write(cpu.esp + 6*4, SREG_SS, 4, cpu.ecx);
    vaddr_write(cpu.esp + 5*4, SREG_SS, 4, cpu.edx);
    vaddr_write(cpu.esp + 4*4, SREG_SS, 4, cpu.ebx);
    vaddr_write(cpu.esp + 3*4, SREG_SS, 4, tmp);
    vaddr_write(cpu.esp + 2*4, SREG_SS, 4, cpu.ebp);
    vaddr_write(cpu.esp + 1*4, SREG_SS, 4, cpu.esi);
    vaddr_write(cpu.esp + 0*4, SREG_SS, 4, cpu.edi); 
    return 1; 
}
