enableInterrupt();
for (int j = 0; j < 0x100000; j++)
{
	*(uint8_t *)(j + (i + 1) * 0x100000) = *(uint8_t *)(j + (current + 1) * 0x100000);
	if (!(j % 0x1000))
		asm volatile("int $0x20");
}
disableInterrupt();