gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);