void printf(const char *format, ...)
{
    int i = 0; // format index
    char buffer[MAX_BUFFER_SIZE];
    int count = 0;                    // buffer index
    void *paraList = (void *)&format; // address of format in stack
    int decimal = 0;
    uint32_t hexadecimal = 0;
    char *string = 0;
    char character = 0;
    for (; format[i] && count <= MAX_BUFFER_SIZE; ++i)
    {
        buffer[count] = format[i];
        count++;
        //TODO in lab2
        if (format[i] == '%')
        {
            --count;
            paraList += sizeof(format);
            switch (format[++i])
            {
            case 'c':
                character = *(char *)paraList;
                buffer[count++] = character;
                break;
            case 's':
                string = *(char **)paraList;
                count = str2Str(string, buffer, (uint32_t)MAX_BUFFER_SIZE, count);
                break;
            case 'x':
                hexadecimal = *(uint32_t *)paraList;
                count = hex2Str(hexadecimal, buffer, (uint32_t)MAX_BUFFER_SIZE, count);
                break;
            case 'd':
                decimal = *(int *)paraList;
                count = dec2Str(decimal, buffer, (uint32_t)MAX_BUFFER_SIZE, count);
                break;
            case '%':
                paraList -= sizeof(format);
                ++count;
            }
        }
    }
    if (count != 0)
        syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);
}