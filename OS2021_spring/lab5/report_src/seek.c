uint32_t cur_off;
switch (sf->ebx)
{ // whence
case SEEK_SET:
    if (offset >= 0 && offset <= inode.size)
    {
        file[sf->ecx - MAX_DEV_NUM].offset = offset;
        pcb[current].regs.eax = 0;
    }
    else
        pcb[current].regs.eax = -1;
    break;
case SEEK_CUR:
    cur_off = file[sf->ecx - MAX_DEV_NUM].offset;
    if (cur_off + offset >= 0 && cur_off + offset <= inode.size)
    {
        file[sf->ecx - MAX_DEV_NUM].offset += offset;
        pcb[current].regs.eax = 0;
    }
    else
        pcb[current].regs.eax = -1;
    break;
case SEEK_END:
    if (offset + inode.size >= 0 && offset + inode.size <= inode.size)
    {
        file[sf->ecx - MAX_DEV_NUM].offset = offset + inode.size;
        pcb[current].regs.eax = 0;
    }
    else
        pcb[current].regs.eax = -1;
    break;
default:
    break;
}