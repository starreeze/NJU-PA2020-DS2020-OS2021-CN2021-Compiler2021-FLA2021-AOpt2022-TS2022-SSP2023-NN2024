#ifndef __INSTR_OR_H__
#define __INSTR_OR_H__
/*
Put the declarations of `or' instructions here.
*/
make_instr_func(or_r2rm_b);
make_instr_func(or_r2rm_v);
make_instr_func(or_rm2r_b);
make_instr_func(or_rm2r_v);
make_instr_func(or_i2rm_b);
make_instr_func(or_i2rm_v);
make_instr_func(or_i2rm_bv);
make_instr_func(or_i2r_b);
make_instr_func(or_i2r_v);
make_instr_func(or_i2r_bv);

make_instr_func(or_al);
make_instr_func(or_eax);
#endif
