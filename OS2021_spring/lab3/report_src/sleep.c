void syscallSleep(struct StackFrame *sf)
{
	if (sf->ecx <= 0)
		log("sleep time not positive!\n");
	else
	{
		pcb[current].state = STATE_BLOCKED;
		pcb[current].sleepTime = sf->ecx;
		asm volatile("int $0x20");
	}