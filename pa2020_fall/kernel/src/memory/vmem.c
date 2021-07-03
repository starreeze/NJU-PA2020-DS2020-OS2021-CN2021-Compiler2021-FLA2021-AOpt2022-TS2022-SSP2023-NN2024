#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define NR_PT ((SCR_SIZE + PT_SIZE - 1) / PT_SIZE) // number of page tables to cover the vmem

PDE *get_updir();
PTE table[1024] align_to_page;
void create_video_mapping()
{
	// 0000 0000 00|00 1010 0000 | 0000 0000 0000
	/* TODO: create an identical mapping from virtual memory area
	 * [0xa0000, 0xb1300) to physical memeory area
	 * [0xa0000, 0xb1300) for user program. You may define
	 * some page tables to create this mapping.
	 */
	PDE* pdt = (PDE *)va_to_pa(get_updir()); // the first dir entry
	// assert(pde.present);
	PTE* pt = (PTE*)va_to_pa(table);
  pdt[0].val = make_pde(pt);
	for(uint32_t pidx = 0xa0; pidx <= 0xb1; ++pidx) {
		pt[pidx].val = make_pte(pidx << 12);
	}
	// panic("please implement me");
}

void video_mapping_write_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		buf[i] = i;
	}
}

void video_mapping_read_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		assert(buf[i] == i);
	}
}

void video_mapping_clear()
{
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}
