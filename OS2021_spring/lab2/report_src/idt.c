setIntr(idt + 0x21, SEG_KCODE, (uint32_t)irqKeyboard, DPL_KERN);
setIntr(idt + 0x80, SEG_KCODE, (uint32_t)irqSyscall, DPL_USER);