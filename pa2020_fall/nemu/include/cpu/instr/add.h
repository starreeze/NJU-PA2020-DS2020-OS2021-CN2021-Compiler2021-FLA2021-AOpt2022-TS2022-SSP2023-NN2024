#ifndef __INSTR_ADD_H__
#define __INSTR_ADD_H__
/*
Put the declarations of `add' instructions here.
*/
make_instr_func(add_r2rm_b);
make_instr_func(add_r2rm_v);
make_instr_func(add_rm2r_b);
make_instr_func(add_rm2r_v);
make_instr_func(add_i2rm_b);
make_instr_func(add_i2rm_v);
make_instr_func(add_i2rm_bv);
make_instr_func(add_i2r_b);
make_instr_func(add_i2r_v);
make_instr_func(add_i2r_bv);

make_instr_func(add_al);
make_instr_func(add_eax);
#endif
