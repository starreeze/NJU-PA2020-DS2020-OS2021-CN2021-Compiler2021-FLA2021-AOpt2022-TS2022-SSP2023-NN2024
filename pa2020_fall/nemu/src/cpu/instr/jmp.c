#include "cpu/instr.h"

make_instr_func(jmp_near)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = data_size;
        rel.addr = eip + 1;

        operand_read(&rel);

        int offset = sign_ext(rel.val, data_size);
        // thank Ting Xu from CS'17 for finding this bug
        print_asm_1("jmp", "", 1 + data_size / 8, &rel);

        cpu.eip += offset;

        return 1 + data_size / 8;
}

make_instr_func(jmp_near_b)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = 8;
        rel.addr = eip + 1;

        operand_read(&rel);

        print_asm_1("jmp", "", 2, &rel);
        cpu.eip += sign_ext(rel.val, 8);

        return 2;
}

// make_instr_func(jmp_far)
// {
//         OPERAND rel;
//         rel.type = OPR_IMM;
//         rel.sreg = SREG_CS;
//         rel.data_size = 32;
//         rel.addr = eip + 1;

//         operand_read(&rel);

//         print_asm_1("jmp", "", 5, &rel);

//         cpu.eip += sign_ext(rel.val, data_size);

//         return 5;
// }

make_instr_func(jmp_rm_near)
{
        OPERAND near;
        near.data_size = data_size;
        modrm_rm(eip + 1, &near);
        operand_read(&near);

        print_asm_1("jmp", "", 5, &near);
        if(data_size == 32)
            cpu.eip = near.val;
        else
            cpu.eip = near.val & 0xffff;
        return 0;
}

#ifdef IA32_SEG
make_instr_func(ljmp)
{
    OPERAND sreg, offset;
    offset.data_size = 32;
    offset.addr = eip+1;
    offset.type = OPR_IMM;
    operand_read(&offset);
    
    sreg.data_size = 16;
    sreg.addr = eip + 5;
    sreg.type = OPR_IMM;
    operand_read(&sreg);
    
    cpu.cs.val = sreg.val;
    cpu.eip = offset.val;
    return 0;
}
#endif
