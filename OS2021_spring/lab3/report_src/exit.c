void syscallExit(struct StackFrame *sf)
{
	pcb[current].state = STATE_DEAD;
	asm volatile("int $0x20");
}