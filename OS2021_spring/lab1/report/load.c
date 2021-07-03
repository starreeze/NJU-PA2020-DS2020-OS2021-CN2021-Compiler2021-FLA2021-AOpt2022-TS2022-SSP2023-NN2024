void bootMain(void)
{
	void (*program)(void) = 0x8c00;
	readSect(program, 1);
	program();
}