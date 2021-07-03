#ifndef __INSTR_AND_H__
#define __INSTR_AND_H__
/*
Put the declarations of `and' instructions here.
*/
make_instr_func(and_r2rm_b);
make_instr_func(and_r2rm_v);
make_instr_func(and_rm2r_b);
make_instr_func(and_rm2r_v);
make_instr_func(and_i2rm_b);
make_instr_func(and_i2rm_v);
make_instr_func(and_i2rm_bv);
make_instr_func(and_i2r_b);
make_instr_func(and_i2r_v);
make_instr_func(and_i2r_bv);

make_instr_func(and_al);
make_instr_func(and_eax);
#endif
