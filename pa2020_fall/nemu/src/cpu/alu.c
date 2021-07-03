#include "cpu/cpu.h"

uint32_t sign_ext(uint32_t x, size_t data_size)
{
        assert(data_size == 16 || data_size == 8 || data_size == 32);
        switch (data_size)
        {
        case 8:
                return (int32_t)((int8_t)(x & 0xff));
        case 16:
                return (int32_t)((int16_t)(x & 0xffff));
        default:
                return (int32_t)x;
        }
}

uint64_t sign_ext_64(uint32_t x, size_t data_size)
{
        assert(data_size == 16 || data_size == 8 || data_size == 32);
        switch (data_size)
        {
        case 8:
                return (int64_t)((int8_t)(x & 0xff));
        case 16:
                return (int64_t)((int16_t)(x & 0xffff));
        default:
                return (int64_t)((int32_t)x);
        }
}

bool sig(uint32_t x, size_t data_size) {
    return (x >> (data_size-1)) & 1;
}

typedef struct {
    unsigned a:1; 
    unsigned b:1; 
    unsigned c:1; 
    unsigned d:1; 
    unsigned e:1; 
    unsigned f:1; 
    unsigned g:1; 
    unsigned h:1;
} _byte;

size_t count1(char r) {
    _byte* p = (_byte*) &r;
    return p->a + p->b + p->c + p->d + p->e + p->f + p->g + p->h;
}

void set_flags(uint32_t r, size_t data_size) {
    //set result related flags: PF,ZF,SF
    cpu.eflags.SF = sig(r, data_size);
    cpu.eflags.ZF = !r;
    cpu.eflags.PF = !(count1((char)r) & 1);
}

uint32_t trun(uint32_t x, size_t s) {
    return s == 32 ? x : x & ((1 << s) - 1);
}

uint32_t add(uint32_t src, uint32_t dest, size_t data_size, bool* cf, bool* of) {
    src = trun(src, data_size); dest = trun(dest, data_size);
    uint32_t r = trun(src + dest, data_size);
    *cf = r < src;
    *of = sig(src, data_size) == sig(dest, data_size) && sig(src, data_size) != sig(r, data_size);
    return r;
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
    bool cf, of;
    uint32_t r = add(src, dest, data_size, &cf, &of);
    cpu.eflags.CF = cf;
    cpu.eflags.OF = of;
    set_flags(r, data_size);
    return r;
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
    bool cf1, of1, cf2, of2;
    uint32_t t = add(src, dest, data_size, &cf1, &of1);
    uint32_t r = add(t, cpu.eflags.CF, data_size, &cf2, &of2);
    cpu.eflags.CF = cf1 ^ cf2; cpu.eflags.OF = of1 ^ of2;
    set_flags(r, data_size);
    return r;
#endif
}

uint32_t sub(uint32_t src, uint32_t dest, size_t data_size, bool* cf, bool* of) {
    src = ~src;
    bool cf1, of1, cf2, of2;
    uint32_t t = add(src, dest, data_size, &cf1, &of1);
    uint32_t r = add(t, 1, data_size, &cf2, &of2);
    *cf = !(cf1 ^ cf2); *of = of1 ^ of2;
    return r;
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
    bool cf, of;
    uint32_t r = sub(src, dest, data_size, &cf, &of);
    cpu.eflags.CF = cf;
    cpu.eflags.OF = of;
    set_flags(r, data_size);
    return r;
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	bool cf1, of1, cf2, of2;
    uint32_t t = sub(src, dest, data_size, &cf1, &of1);
    uint32_t r = sub(cpu.eflags.CF, t, data_size, &cf2, &of2);
    cpu.eflags.CF = cf1 ^ cf2; cpu.eflags.OF = of1 ^ of2;
    set_flags(r, data_size);
    return r;
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
    src = trun(src, data_size);
    dest = trun(dest, data_size);
	uint64_t r = (uint64_t)src * (uint64_t)dest;
	cpu.eflags.CF = cpu.eflags.OF = (r >> data_size) && 1;
	return r;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	int64_t r = (int64_t)src * (int64_t)dest;
	return r;
#endif
}

uint64_t trun_64(uint64_t x, size_t s) {
    return s == 64 ? x : x & (((uint64_t)1 << s) - 1);
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
    assert(src);
	return trun(dest / src, data_size);
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
    assert(src);
	return trun(dest / src, data_size);
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
	return dest % src;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	return dest % src;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t r = trun(src & dest, data_size);
	cpu.eflags.CF = cpu.eflags.OF = 0;
	set_flags(r, data_size);
	return r;
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t r = trun(src ^ dest, data_size);
	cpu.eflags.CF = cpu.eflags.OF = 0;
	set_flags(r, data_size);
	return r;
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t r = trun(src | dest, data_size);
	cpu.eflags.CF = cpu.eflags.OF = 0;
	set_flags(r, data_size);
	return r;
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
    src = trun(src, data_size);
	uint32_t x = trun(dest, data_size);
	uint32_t r = x << src;
	cpu.eflags.CF = (x >> (data_size - src)) & 1;
	r = trun(r, data_size);
	set_flags(r, data_size);
	return r;
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
    src = trun(src, data_size);
	uint32_t x = trun(dest, data_size);
	uint32_t r = x >> src;
	cpu.eflags.CF = (x >> (src - 1)) & 1;
	r = trun(r, data_size);
	set_flags(r, data_size);
	return r;
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
    src = trun(src, data_size);
	uint32_t x = trun(dest, data_size);
	uint32_t r = x >> src;
	if(sig(x,data_size))
        r |= ((1 << src) - 1) << (data_size - src);
	cpu.eflags.CF = (x >> (src - 1)) & 1;
	r = trun(r, data_size);
	set_flags(r, data_size);
	return r;
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	return alu_shl(src, dest, data_size);
#endif
}

uint32_t alu_neg(uint32_t dest, size_t data_size) {
    dest = ~dest;
    return alu_add(1, dest, data_size);
}

uint32_t alu_not(uint32_t dest, size_t data_size) {
    return trun(~dest, data_size);
}
