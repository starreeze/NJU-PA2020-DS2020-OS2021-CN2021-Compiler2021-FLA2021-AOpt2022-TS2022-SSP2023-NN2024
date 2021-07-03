#ifndef __INSTR_ADC_H__
#define __INSTR_ADC_H__
/*
Put the declarations of `adc' instructions here.
*/
make_instr_func(adc_r2rm_b);
make_instr_func(adc_r2rm_v);
make_instr_func(adc_rm2r_b);
make_instr_func(adc_rm2r_v);
make_instr_func(adc_i2rm_b);
make_instr_func(adc_i2rm_v);
make_instr_func(adc_i2rm_bv);
make_instr_func(adc_i2r_b);
make_instr_func(adc_i2r_v);
make_instr_func(adc_i2r_bv);

make_instr_func(adc_al);
make_instr_func(adc_eax);
#endif
