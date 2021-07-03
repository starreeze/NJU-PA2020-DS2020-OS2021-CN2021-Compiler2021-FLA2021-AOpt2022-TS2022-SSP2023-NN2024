if ((((sf->edx >> 3) & 1) == 0 && destInode.type == DIRECTORY_TYPE) || (((sf->edx >> 3) & 1) == 1 && destInode.type == REGULAR_TYPE))
{
	pcb[current].regs.eax = -1;
	return;
}
for (i = 0; i < MAX_FILE_NUM; ++i)
	if (file[i].inodeOffset == destInodeOffset && file[i].state == 1)
	{
		pcb[current].regs.eax = -1;
		return;
	}
for (i = 0; i < MAX_DEV_NUM; ++i)
	if (dev[i].inodeOffset == destInodeOffset && dev[i].state == 1)
	{
		pcb[current].regs.eax = i;
		return;
	}
for (i = 0; i < MAX_FILE_NUM; ++i)
	if (file[i].state == 0)
	{
		file[i].state = 1;
		file[i].flags = sf->edx;
		file[i].inodeOffset = destInodeOffset;
		file[i].offset = 0;
		pcb[current].regs.eax = MAX_DEV_NUM + i;
		return;
	}
pcb[current].regs.eax = -1;