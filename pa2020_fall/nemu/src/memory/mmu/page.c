#include "cpu/cpu.h"
#include "memory/memory.h"

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
// 	printf("\nPlease implement page_translate()\n");
// 	fflush(stdout);
// 	assert(0);
    if(cpu.cr0.pg) {
        uint32_t pde_addr = cpu.cr3.val & 0xfffff000; //higher 20 bits in cr3
        PDE page_dir_entry = (PDE)paddr_read(pde_addr + 4 * (laddr >> 22), 4); //higher 10 bits
        assert(page_dir_entry.present == 1);
        uint32_t pte_addr = page_dir_entry.page_frame << 12;
        PTE page_table_entry = (PTE)paddr_read(pte_addr + 4 * ((laddr >> 12) & 0x3ff), 4); //middle 10 bits
        assert(page_table_entry.present == 1);
        return (page_table_entry.page_frame << 12) | (laddr & 0xfff); //lower 12 bits
    }
    else    return laddr;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
