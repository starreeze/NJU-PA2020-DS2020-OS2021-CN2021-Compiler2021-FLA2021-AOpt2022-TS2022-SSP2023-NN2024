if ((sf->edx >> 2) % 2 == 0)
{
	pcb[current].regs.eax = -1;
	return;
}
if ((sf->edx >> 3) % 2 == 0)
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
	ret = allocInode(&sBlock, gDesc, &fatherInode, fatherInodeOffset, &destInode, &destInodeOffset, str + size + 1, REGULAR_TYPE);
}
else
{
	length = stringLen(str);
	if (str[length - 1] == '/')
	{
		cond = 1;
		str[length - 1] = 0;
	}
	ret = stringChrR(str, '/', &size);
	if (ret == -1)
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
	ret = allocInode(&sBlock, gDesc, &fatherInode, fatherInodeOffset, &destInode, &destInodeOffset, str + size + 1, DIRECTORY_TYPE);
	if (cond == 1)
		str[length - 1] = '/';
}
if (ret == -1)
{
	pcb[current].regs.eax = -1;
	return;
}
for (i = 0; i < MAX_FILE_NUM; i++)
	if (file[i].state == 0)
	{
		file[i].state = 1;
		file[i].inodeOffset = destInodeOffset;
		file[i].offset = 0;
		file[i].flags = sf->edx;
		pcb[current].regs.eax = MAX_DEV_NUM + i;
		return;
	}
pcb[current].regs.eax = -1; // create success but no available file[]