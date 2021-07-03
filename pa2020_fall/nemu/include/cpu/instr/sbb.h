#ifndef __INSTR_SBB_H__
#define __INSTR_SBB_H__
/*
Put the declarations of `sbb' instructions here.
*/
make_instr_func(sbb_r2rm_b);
make_instr_func(sbb_r2rm_v);
make_instr_func(sbb_rm2r_b);
make_instr_func(sbb_rm2r_v);
make_instr_func(sbb_i2rm_b);
make_instr_func(sbb_i2rm_v);
make_instr_func(sbb_i2rm_bv);
make_instr_func(sbb_i2r_b);
make_instr_func(sbb_i2r_v);
make_instr_func(sbb_i2r_bv);

make_instr_func(sbb_al);
make_instr_func(sbb_eax);
#endif
