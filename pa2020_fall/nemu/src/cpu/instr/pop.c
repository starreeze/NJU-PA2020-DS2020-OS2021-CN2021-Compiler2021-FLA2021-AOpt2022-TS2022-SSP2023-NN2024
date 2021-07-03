#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op() 
{
	opr_src.val = vaddr_read(cpu.esp, SREG_CS, data_size / 8);
	cpu.esp += data_size / 8;
	operand_write(&opr_src);
}

make_instr_impl_1op(pop, r, b)
make_instr_impl_1op(pop, r, v)
make_instr_impl_1op(pop, rm, b)
make_instr_impl_1op(pop, rm, v)

make_instr_func(popa) {
    assert(data_size == 32); 
    cpu.eax = vaddr_read(cpu.esp + 7*4, SREG_SS, 4); 
	cpu.ecx = vaddr_read(cpu.esp + 6*4, SREG_SS, 4);
    cpu.edx = vaddr_read(cpu.esp + 5*4, SREG_SS, 4);
    cpu.ebx = vaddr_read(cpu.esp + 4*4, SREG_SS, 4);
	//esp is discarded
    cpu.ebp = vaddr_read(cpu.esp + 2*4, SREG_SS, 4);
    cpu.esi = vaddr_read(cpu.esp + 1*4, SREG_SS, 4);
    cpu.edi = vaddr_read(cpu.esp + 0*4, SREG_SS, 4);
	cpu.esp += 8*4;  
    return 1; 
}