void log(const char *str)
{
    for (int i = 0; i < 100 && str[i]; ++i)
        putChar(str[i]);
}

void logint(uint32_t num)
{
    if (!num)
        putChar('0');
    char buf[12];
    int i = 0;
    for (; num; ++i)
    {
        buf[i] = (char)(num % 10 + '0');
        num /= 10;
    }
    while (--i >= 0)
        putChar(buf[i]);
}
