if (size <= 0)
{
	pcb[current].regs.eax = 0;
	return;
}
if (size > inode.size - file[sf->ecx - MAX_DEV_NUM].offset)
	size = inode.size - file[sf->ecx - MAX_DEV_NUM].offset;
readBlock(&sBlock, &inode, quotient, buffer);
++j;
while (i < size)
{
	str[i] = buffer[(remainder + i) % sBlock.blockSize];
	++i;
	if ((remainder + i) % sBlock.blockSize == 0)
	{
		readBlock(&sBlock, &inode, quotient + j, buffer);
		++j;
	}
}
pcb[current].regs.eax = size;
file[sf->ecx - MAX_DEV_NUM].offset += size;