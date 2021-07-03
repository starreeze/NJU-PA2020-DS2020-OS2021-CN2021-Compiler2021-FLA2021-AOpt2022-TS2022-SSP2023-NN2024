#ifndef __INSTR_SUB_H__
#define __INSTR_SUB_H__
/*
Put the declarations of `sub' instructions here.
*/
make_instr_func(sub_r2rm_b);
make_instr_func(sub_r2rm_v);
make_instr_func(sub_rm2r_b);
make_instr_func(sub_rm2r_v);
make_instr_func(sub_i2rm_b);
make_instr_func(sub_i2rm_v);
make_instr_func(sub_i2rm_bv);
make_instr_func(sub_i2r_b);
make_instr_func(sub_i2r_v);
make_instr_func(sub_i2r_bv);

make_instr_func(sub_al);
make_instr_func(sub_eax);
#endif
