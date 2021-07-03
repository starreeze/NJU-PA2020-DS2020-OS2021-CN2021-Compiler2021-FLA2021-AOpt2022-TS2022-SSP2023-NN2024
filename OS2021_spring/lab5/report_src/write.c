if (size <= 0)
{
	pcb[current].regs.eax = 0;
	return;
}
if (quotient < inode.blockCount)
	readBlock(&sBlock, &inode, quotient, buffer);
while (i < size)
{
	buffer[(remainder + i) % sBlock.blockSize] = str[i];
	++i;
	if ((remainder + i) % sBlock.blockSize == 0)
	{
		if (quotient + j == inode.blockCount)
		{
			ret = allocBlock(&sBlock, gDesc, &inode, file[sf->ecx - MAX_DEV_NUM].inodeOffset);
			if (ret == -1)
			{
				inode.size = inode.blockCount * sBlock.blockSize;
				diskWrite(&inode, sizeof(Inode), 1, file[sf->ecx - MAX_DEV_NUM].inodeOffset);
				pcb[current].regs.eax = inode.size - file[sf->ecx - MAX_DEV_NUM].offset;
				file[sf->ecx - MAX_DEV_NUM].offset = inode.size;
				return;
			}
		}
		writeBlock(&sBlock, &inode, quotient + j, buffer);
		++j;
		if (quotient + j < inode.blockCount)
			readBlock(&sBlock, &inode, quotient + j, buffer);
	}
}
if (quotient + j == inode.blockCount)
{
	ret = allocBlock(&sBlock, gDesc, &inode, file[sf->ecx - MAX_DEV_NUM].inodeOffset);
	if (ret == -1)
	{
		inode.size = inode.blockCount * sBlock.blockSize;
		diskWrite(&inode, sizeof(Inode), 1, file[sf->ecx - MAX_DEV_NUM].inodeOffset);
		pcb[current].regs.eax = inode.size - file[sf->ecx - MAX_DEV_NUM].offset;
		file[sf->ecx - MAX_DEV_NUM].offset = inode.size;
		return;
	}
}