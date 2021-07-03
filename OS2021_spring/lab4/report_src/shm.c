// in kernel
void syscallShm(struct StackFrame *sf)
{
	switch (sf->ecx)
	{
	case SHM_WRITE:
		shm = sf->edx;
		break;
	case SHM_READ:
		sf->eax = shm;
		break;
	default:
		break;
	}
}
// in user lib
uint32_t shm_read()
{
	return syscall(SYS_SHM, SHM_READ, 0, 0, 0, 0);
}
int shm_write(uint32_t value)
{
	return syscall(SYS_SHM, SHM_WRITE, value, 0, 0, 0);
}