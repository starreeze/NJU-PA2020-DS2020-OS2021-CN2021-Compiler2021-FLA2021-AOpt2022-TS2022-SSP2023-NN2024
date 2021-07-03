for (i = 0; i < size; i++)
{
	/* print and update curser */
#ifdef TEST_COMPETE_DISPLAY
	asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
#endif