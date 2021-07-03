pid_t fork()
{
	return syscall(SYS_FORK, 0, 0, 0, 0, 0);
}
int sleep(uint32_t time)
{
	return syscall(SYS_SLEEP, time, 0, 0, 0, 0);
}
int exit()
{
	return syscall(SYS_EXIT, 0, 0, 0, 0, 0);
}