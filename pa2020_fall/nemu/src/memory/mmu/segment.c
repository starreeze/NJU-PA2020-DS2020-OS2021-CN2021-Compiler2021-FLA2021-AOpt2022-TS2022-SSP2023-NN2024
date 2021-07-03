#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
#ifdef IA32_SEG
	return cpu.segReg[sreg].base + offset;
#else
    return 0;
#endif
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */
#ifdef IA32_SEG
	SegDesc* segd = (SegDesc*)(hw_mem + cpu.gdtr.base + sreg * 8);
	cpu.segReg[sreg].base = (segd->base_31_24 << 24) | (segd->base_23_16 << 16) | segd->base_15_0;
	cpu.segReg[sreg].limit = (segd->limit_19_16 << 16) | segd->limit_15_0;
	assert(cpu.segReg[sreg].base == 0);
	assert(cpu.segReg[sreg].limit == 0xfffff);
	cpu.segReg[sreg].type = (segd->segment_type << 4) | segd->type;
	cpu.segReg[sreg].privilege_level = segd->privilege_level;
	cpu.segReg[sreg].soft_use = segd->soft_use;
#endif
}
