#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

// int32_t is_mmio(uint32_t paddr) {
// 	return paddr >= 0xa0000 && paddr < 0xa0000 + 320*200 ? paddr : -1;
// }

uint32_t paddr_read(paddr_t paddr, size_t len) 
{
	uint32_t ret;
#ifdef HAS_DEVICE_VGA
	int32_t mapNo = is_mmio(paddr);
	if(mapNo == -1) {
	#ifdef CACHE_ENABLED
		ret = cache_read(paddr, len);
	#else
		ret = hw_mem_read(paddr, len);
	#endif
	}
	else	ret = mmio_read(paddr, len, mapNo);
#else
	#ifdef CACHE_ENABLED
	ret = cache_read(paddr, len);
	#else
	ret = hw_mem_read(paddr, len);
	#endif
#endif
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data) 
{
#ifdef HAS_DEVICE_VGA
	int32_t mapNo = is_mmio(paddr);
	if(mapNo == -1) {
	#ifdef CACHE_ENABLED
		cache_write(paddr, len, data);
	#else
		hw_mem_write(paddr, len, data);
	#endif
	}
	else	mmio_write(paddr, len, data, mapNo);
#else
	#ifdef CACHE_ENABLED
	cache_write(paddr, len, data);
	#else
	hw_mem_write(paddr, len, data);
	#endif
#endif
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
#ifdef IA32_PAGE
	uint32_t paddr = page_translate(laddr);
	uint32_t offset = laddr & 0xfff; //lower 12 bits
	uint32_t left_len = 0x1000 - offset;
	if(len > left_len) { // cross page
	    uint32_t right_len = len - left_len;
	    uint32_t r1 = paddr_read(paddr, left_len);
	    uint32_t r2 = paddr_read(page_translate(((laddr >> 12) + 1) << 12), right_len);
	    return (r2 << (left_len * 8)) | r1; //little endian, r2 is more significant
	}
	return paddr_read(paddr, len);
#else
    return paddr_read(laddr, len);
#endif

}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
#ifdef IA32_PAGE
	uint32_t paddr = page_translate(laddr);
	uint32_t offset = laddr & 0xfff; //lower 12 bits
	uint32_t left_len = 0x1000 - offset;
	if(len > left_len) { // cross page
	    uint32_t right_len = len - left_len;
	    paddr_write(paddr, left_len, data);
	    paddr_write(page_translate(((laddr >> 12) + 1) << 12), right_len, data >> (8 * left_len));
	}
	else paddr_write(paddr, len, data);
#else
	paddr_write(laddr, len, data);
#endif
}

uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG
	return laddr_read(segment_translate(vaddr, sreg), len);
#else
    return laddr_read(vaddr, len);
#endif
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG
	return laddr_write(segment_translate(vaddr, sreg), len, data);
#else
    return laddr_write(vaddr, len, data);
#endif
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);
#ifdef CACHE_ENABLED
	init_cache();
#endif
#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
