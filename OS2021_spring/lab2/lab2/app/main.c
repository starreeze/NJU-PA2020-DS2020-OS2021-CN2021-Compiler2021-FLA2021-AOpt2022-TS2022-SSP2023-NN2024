#include "lib.h"
#include "types.h"

int uEntry(void)
{
	uint16_t selector;
	//uint16_t selector = 16;
	asm volatile("movw %%ss, %0"
				 : "=m"(selector)); //XXX necessary or not, iret may reset ds in QEMU
	asm volatile("movw %%ax, %%ds" ::"a"(selector));

	printf("I/O test begin...\n");
	printf("the answer should be:\n");
	printf("#######################################################\n");
	printf("Hello, welcome to OSlab! I'm the body of the game.\n");
	printf("Now I will test your printf:\n");
	printf("1 + 1 = 2, 123 * 456 = 56088, 0, -1, -2147483648, -1412505855, -32768, 102030, 0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
	printf("Now I will test your getChar: ");
	printf("1 + 1 = 2\n2 * 123 = 246\n");
	printf("Now I will test your getStr: ");
	printf("Alice is stronger than Bob\nBob is weaker than Alice\n");
	printf("#######################################################\n");
	printf("your answer:\n");
	printf("=======================================================\n");
	printf("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
	printf("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
	printf("I'm the %s of %s.\n", "body", "the game");
	printf("Now I will test your printf:\n");
	printf("%d + %d = %d, %d * %d = %d, ", 1, 1, 1 + 1, 123, 456, 123 * 456);
	printf("%d, %d, %d, %d, %d, %d, ", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printf("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printf("Now I will test your getChar: ");
	printf("1 + 1 = ");
	char num = getChar();
	printf("%c * 123 = 246\n", num);
	printf("Now I will test your getStr: ");
	printf("Alice is stronger than ");
	char name[20];
	getStr(name, 20);
	printf("%s is stronger than Alice\n", name);
	printf("=======================================================\n");
	printf("Test end!!! Good luck!!!\n");

	while (1)
		;
	return 0;
}
