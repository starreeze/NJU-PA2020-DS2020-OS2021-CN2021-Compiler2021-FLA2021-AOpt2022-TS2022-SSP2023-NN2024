char retriveChar(InputBuf *buf)
{
    // retrive a single char
    asm volatile("sti");
    while (!buf->size || buf->buf[buf->size - 1] != '\n')
        waitForInterrupt();
    asm volatile("cli");
    char res = buf->buf[0];
    buf->size = 0;
    return res;
}
void retriveStr(InputBuf *buf, char *dst)
{
    // retrive until \n
    asm volatile("sti");
    while (!buf->size || buf->buf[buf->size - 1] != '\n')
        waitForInterrupt();
    asm volatile("cli");
    int i = 0;
    for (; i < buf->size && buf->buf[i] != '\n'; ++i)
        dst[i] = buf->buf[i];
    dst[i] = 0;
    buf->size = 0;
}