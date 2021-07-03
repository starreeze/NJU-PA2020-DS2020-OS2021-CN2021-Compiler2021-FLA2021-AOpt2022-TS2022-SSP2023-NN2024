typedef struct
{
    int size;
    char buf[MAX_INPUT_SIZE];
} InputBuf;
void clearBuf(InputBuf *buf)
{
	buf->size = 0;
}
void insertBuf(InputBuf *buf, char c)
{
	buf->buf[buf->size++] = c;
	if (buf->size > MAX_INPUT_SIZE)
	{
		log("Input buf overflow!\n");
		assert(0);
	}
}
int deleteBack(InputBuf *buf)
{
	if (buf->size)
	{
		--buf->size;
		return 1;
	}
	return 0;
}