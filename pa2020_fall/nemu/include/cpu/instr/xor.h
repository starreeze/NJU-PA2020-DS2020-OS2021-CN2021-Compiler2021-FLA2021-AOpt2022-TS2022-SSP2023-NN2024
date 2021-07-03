#ifndef __INSTR_XOR_H__
#define __INSTR_XOR_H__
/*
Put the declarations of `xor' instructions here.
*/
make_instr_func(xor_r2rm_b);
make_instr_func(xor_r2rm_v);
make_instr_func(xor_rm2r_b);
make_instr_func(xor_rm2r_v);
make_instr_func(xor_i2rm_b);
make_instr_func(xor_i2rm_v);
make_instr_func(xor_i2rm_bv);
make_instr_func(xor_i2r_b);
make_instr_func(xor_i2r_v);
make_instr_func(xor_i2r_bv);

make_instr_func(xor_al);
make_instr_func(xor_eax);
#endif
