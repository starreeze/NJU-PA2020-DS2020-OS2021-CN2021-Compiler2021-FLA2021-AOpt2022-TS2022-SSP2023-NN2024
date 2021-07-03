void log(const char *str)
{
    for (int i = 0; i < 100 && str[i]; ++i)
        putChar(str[i]);
}