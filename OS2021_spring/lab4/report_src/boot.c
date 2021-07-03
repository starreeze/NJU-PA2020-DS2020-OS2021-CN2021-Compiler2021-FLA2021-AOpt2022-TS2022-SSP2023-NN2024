void bootMain(void)
{
	int i = 0;
	// int phoff = 0x34;
	int offset = 0x1000;
	unsigned int elf = 0x100000;
	void (*kMainEntry)(void);
	kMainEntry = (void (*)(void))0x100000;

	for (i = 0; i < 200; i++)
	{
		readSect((void *)(elf + i * 512), 1 + i);
	}

	kMainEntry = (void (*)(void))((struct ELFHeader *)elf)->entry;
	// phoff = ((struct ELFHeader *)elf)->phoff;
	// offset = ((struct ProgramHeader *)(elf + phoff))->off;

	for (i = 0; i < 200 * 512; i++)
	{
		*(unsigned char *)(elf + i) = *(unsigned char *)(elf + i + offset);
	}

	kMainEntry();
}