void printChar(char c)
{
    uint16_t data = c | (0x0c << 8);
    int pos = (80 * displayRow + displayCol) * 2;
    asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
}