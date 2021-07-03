#include "memory/mmu/cache.h"
#include <stdlib.h>
// extern uint8_t* hw_mem;
extern uint32_t hw_mem_read(paddr_t paddr, size_t len);
extern void hw_mem_write(paddr_t paddr, size_t len, uint32_t data);
char cache[1024][64];
bool valid[1024];
uint32_t mark[1024];

uint32_t get_mark(uint32_t addr) {
    return addr >> 13;
}
uint32_t get_group(uint32_t addr) {
    return (addr >> 6) & 0x7f;
}

void store(uint32_t addr) {
    int cache_pos = get_group(addr) * 8;
    int i=cache_pos;
    for(; i<cache_pos+8; ++i)
        if(!valid[i]) {
            mark[i] = get_mark(addr);
            valid[i] = true;
            addr &= ~0x3f;
            // memcpy(cache[i], hw_mem+addr, 64);
            for(int j=0; j<64; ++j) {
                uint32_t t = hw_mem_read(addr + j, 1);
                cache[i][j] = t;
            }
            break;
        }
    if(i == cache_pos+8) {
        i = (rand() % 8) + cache_pos;
        mark[i] = get_mark(addr);
        addr &= ~0x3f;
        // memcpy(cache[i], hw_mem+addr, 64);
        for(int j=0; j<64; ++j) {
            uint32_t t = hw_mem_read(addr + j, 1);
            cache[i][j] = t;
        }
    }
}

// init the cache
void init_cache()
{
	// implement me in PA 3-1
	memset(valid, 0, 1024);
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// implement me in PA 3-1
	int cache_pos = get_group(paddr) * 8;
	for(int i=0; i<8; ++i)
	    if(valid[cache_pos + i] && get_mark(paddr) == mark[cache_pos + i]) {
	        uint32_t last = paddr & 0x3f;
	        if(last + len > 0x3f) {
	            hw_mem_write(paddr, len, data);
	            memcpy(cache[cache_pos + i] + last, &data, 0x3f - last);
	            return;
	        }
	        else {
	            hw_mem_write(paddr, len, data);
	            memcpy(cache[cache_pos + i] + last, &data, len);
	            return;
	        }
	    }
	hw_mem_write(paddr, len, data);
	store(paddr);
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
	// implement me in PA 3-1
	int cache_pos = get_group(paddr) * 8;
	for(int i=0; i<8; ++i)
	    if(valid[cache_pos + i] && get_mark(paddr) == mark[cache_pos + i]) {
	        uint32_t last = paddr & 0x3f;
	        if(last + len > 0x3f)
	            return hw_mem_read(paddr, len);
	        else {
	            uint32_t r = 0;
            	memcpy(&r, cache[cache_pos + i] + last, len);
            	return r;
	        }
	    }
	store(paddr);
    return hw_mem_read(paddr, len);
}

