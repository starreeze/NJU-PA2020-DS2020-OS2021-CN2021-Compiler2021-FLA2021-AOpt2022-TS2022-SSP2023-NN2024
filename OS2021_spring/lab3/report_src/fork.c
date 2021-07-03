void syscallFork(struct StackFrame *sf)
{
	int i = 0;
	for (; i < MAX_PCB_NUM; ++i)
		if (pcb[i].state == STATE_DEAD)
			break;
	if (i != MAX_PCB_NUM)
	{
#ifdef TEST_MULTI_IRQ
		enableInterrupt();
		for (int j = 0; j < 0x100000; j++)
		{
			*(uint8_t *)(j + (i + 1) * 0x100000) = *(uint8_t *)(j + (current + 1) * 0x100000);
			if (!(j % 0x1000))
				asm volatile("int $0x20");
		}
		disableInterrupt();
#else
		memcpy((void *)((i + 1) * 0x100000), (void *)((current + 1) * 0x100000), 0x100000);
#endif

		memcpy(&pcb[i], &pcb[current], sizeof(ProcessTable));
		// pcb[i] = pcb[current];
		pcb[i].stackTop = (uint32_t) & (pcb[i].regs);
		pcb[i].prevStackTop = (uint32_t) & (pcb[i].stackTop);
		pcb[i].state = STATE_RUNNABLE;
		pcb[i].timeCount = 0;
		pcb[i].sleepTime = 0;
		pcb[i].pid = i;

		pcb[i].regs.ss = USEL(2 + 2 * i);
		pcb[i].regs.cs = USEL(1 + 2 * i);
		pcb[i].regs.ds = USEL(2 + 2 * i);
		pcb[i].regs.es = USEL(2 + 2 * i);
		pcb[i].regs.fs = USEL(2 + 2 * i);
		pcb[i].regs.gs = USEL(2 + 2 * i);

		pcb[i].regs.eax = 0;
		pcb[current].regs.eax = i;
	}
	else
		pcb[current].regs.eax = -1;
}