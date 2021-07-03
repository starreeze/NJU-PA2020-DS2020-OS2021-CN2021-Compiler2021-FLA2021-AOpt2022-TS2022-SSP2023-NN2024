int i = (int)sf->ecx;
if (i < MAX_DEV_NUM || i >= MAX_DEV_NUM + MAX_FILE_NUM)
{
	pcb[current].regs.eax = -1;
	return;
}
if (file[i - MAX_DEV_NUM].state == 0)
{
	pcb[current].regs.eax = -1;
	return;
}
file[i - MAX_DEV_NUM].state = 0;
file[i - MAX_DEV_NUM].inodeOffset = 0;
file[i - MAX_DEV_NUM].offset = 0;
file[i - MAX_DEV_NUM].flags = 0;
pcb[current].regs.eax = 0;