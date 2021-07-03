#ifndef __INSTR_CMP_H__
#define __INSTR_CMP_H__
/*
Put the declarations of `cmp' instructions here.
*/
make_instr_func(cmp_r2rm_b);
make_instr_func(cmp_r2rm_v);
make_instr_func(cmp_rm2r_b);
make_instr_func(cmp_rm2r_v);
make_instr_func(cmp_i2rm_b);
make_instr_func(cmp_i2rm_v);
make_instr_func(cmp_i2rm_bv);
make_instr_func(cmp_i2r_b);
make_instr_func(cmp_i2r_v);
make_instr_func(cmp_i2r_bv);

make_instr_func(cmp_al);
make_instr_func(cmp_eax);
#endif
