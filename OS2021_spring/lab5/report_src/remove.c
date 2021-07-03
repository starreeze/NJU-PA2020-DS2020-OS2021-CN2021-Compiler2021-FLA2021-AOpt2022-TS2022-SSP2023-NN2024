for (i = 0; i < MAX_DEV_NUM; ++i)
	if (dev[i].inodeOffset == destInodeOffset)
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
if (destInode.type == REGULAR_TYPE)
{
	if (stringChrR(str, '/', &size) == -1)
	{
		pcb[current].regs.eax = -1;
		return;
	}
	char parent_path[128];
	for (int i = 0; i < size + 1; ++i)
		parent_path[i] = *(str + i);
	parent_path[size + 1] = 0;
	ret = readInode(&sBlock, gDesc, &fatherInode, &fatherInodeOffset, parent_path);
	if (ret == -1)
	{
		pcb[current].regs.eax = -1;
		return;
	}
	ret = freeInode(&sBlock, gDesc, &fatherInode, fatherInodeOffset, &destInode, &destInodeOffset, str + size + 1, REGULAR_TYPE);
}
else if (destInode.type == DIRECTORY_TYPE)
{
	length = stringLen(str);
	if (str[length - 1] == '/')
	{
		cond = 1;
		str[length - 1] = 0;
	}
	if (stringChrR(str, '/', &size) == -1)
	{
		pcb[current].regs.eax = -1;
		return;
	}
	char parent_path[128];
	for (int i = 0; i < size + 1; ++i)
		parent_path[i] = *(str + i);
	parent_path[size + 1] = 0;
	ret = readInode(&sBlock, gDesc, &fatherInode, &fatherInodeOffset, parent_path);
	if (ret == -1)
	{
		if (cond == 1)
			str[length - 1] = '/';
		pcb[current].regs.eax = -1;
		return;
	}
	ret = freeInode(&sBlock, gDesc, &fatherInode, fatherInodeOffset, &destInode, &destInodeOffset, str + size + 1, DIRECTORY_TYPE);
	if (cond == 1)
		str[length - 1] = '/';
}
if (ret == -1)
{
	pcb[current].regs.eax = -1;
	return;
}
pcb[current].regs.eax = 0;
return;