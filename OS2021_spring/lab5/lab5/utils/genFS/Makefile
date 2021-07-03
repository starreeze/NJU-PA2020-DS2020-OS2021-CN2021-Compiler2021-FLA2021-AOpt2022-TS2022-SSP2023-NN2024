CC = gcc

CFLAGS = -g -m32 -O0

CFILES = $(shell find ./ -name "*.c")
OBJS = $(CFILES:.c=.o)

genFS: $(CFILES)
	$(CC) $(CFLAGS) -o genFS $(CFILES)

clean:
	@#rm -rf $(OBJS) genFS fs.bin
	rm -rf $(OBJS) genFS
