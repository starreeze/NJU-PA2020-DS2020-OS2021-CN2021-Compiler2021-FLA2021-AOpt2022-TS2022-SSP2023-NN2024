#ifndef __INSTR_TEST_H__
#define __INSTR_TEST_H__
/*
Put the declarations of `test' instructions here.
*/
make_instr_func(test_r2rm_b);
make_instr_func(test_r2rm_v);
make_instr_func(test_rm2r_b);
make_instr_func(test_rm2r_v);
make_instr_func(test_i2rm_b);
make_instr_func(test_i2rm_v);
make_instr_func(test_i2r_b);
make_instr_func(test_i2r_v);

make_instr_func(test_al);
make_instr_func(test_eax);
#endif
