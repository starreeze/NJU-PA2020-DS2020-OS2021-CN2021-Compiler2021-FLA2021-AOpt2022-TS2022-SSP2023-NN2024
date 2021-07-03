void syscallReadStdIn(struct StackFrame *sf)
{
	if (dev[STD_IN].value < 0)
	{
		sf->eax = -1;
		return;
	}
	--dev[STD_IN].value;
	push_dev(current, STD_IN);
	pcb[current].state = STATE_BLOCKED;
	asm volatile("int $0x20");
	int sel = sf->ds;
	char *str = (char *)sf->edx;
	int size = sf->ebx;
	int i = 0;
	char character;
	asm volatile("movw %0, %%es" ::"m"(sel));
	while (i < size - 1 && bufferHead != bufferTail)
	{
		character = getChar(keyBuffer[bufferHead]);
		bufferHead = (bufferHead + 1) % MAX_KEYBUFFER_SIZE;
		if (character != 0)
		{
			stdout_char(character);
			asm volatile("movb %0, %%es:(%1)" ::"r"(character), "r"(str + i));
			++i;
		}
	}
	asm volatile("movb $0, %%es:(%0)" ::"r"(str + i));
	sf->eax = i;
}