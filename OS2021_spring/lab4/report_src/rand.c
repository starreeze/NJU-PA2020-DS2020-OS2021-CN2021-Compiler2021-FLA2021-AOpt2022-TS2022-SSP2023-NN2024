uint32_t next = 0;
uint32_t rand()
{
	return next = next * 1103515245 + 12345;
}