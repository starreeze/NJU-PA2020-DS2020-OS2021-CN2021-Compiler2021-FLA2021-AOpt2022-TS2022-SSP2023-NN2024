void syscallSemWait(struct StackFrame *sf)
{
	int i = (int)sf->edx;
	if (i < 0 || i >= MAX_SEM_NUM)
	{
		pcb[current].regs.eax = -1;
		return;
	}
	if (--sem[i].value < 0)
	{
		push_sem(current, i);
		// ++pcb[current].blocked_sems;
		pcb[current].state = STATE_BLOCKED;
		asm volatile("int $0x20");
	}
	sf->eax = 0;
}