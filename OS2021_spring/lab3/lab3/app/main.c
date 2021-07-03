#include "lib.h"
#include "types.h"

int data = 0;

int uEntry(void)
{
	//uint16_t selector;
	////uint16_t selector = 16;
	//asm volatile("movw %%ss, %0":"=m"(selector)); //XXX necessary or not, iret may reset ds in QEMU
	//asm volatile("movw %%ax, %%ds"::"a"(selector));

	//asm volatile("cli"); //XXX test for CPL, will cause a #GP
	//asm volatile("int $0xe"); //XXX test for SWInterrupt/exception:fault, current instruction address is pushed into kernel stack, if idt[0xe].dpl>=cpl, o.w., cause #GP
	//asm volatile("int $0x80");//XXX test for SWInterrupt, next instruction address is pushed into kernel stack, if idt[0x80].dpl>=cpl, o.w., cause #GP
	//asm volatile("int 3 ...");//XXX equivalent to int 0x3?
	//asm volatile("into ...");//XXX equivalent to int 0x4?
	//asm volatile("bound ...");//XXX equivalent to int 0x5?
	//printf("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n");
	//printf("Scroll Screen Test.\n");

	//printf("printf test begin...\n");
	//printf("Task switching test begin...\n");

	int ret = fork();
	//int ret = 0;
	int i = 8;
	if (ret == 0)
	{
		data = 2;
		while (i != 0)
		{
			i--;
			printf("Child Process: Pong %d, %d;\n", data, i);
			//sleep(1);
			sleep(128);
		}
		exit();
	}
	else if (ret != -1)
	{
		data = 1;
		while (i != 0)
		{
			i--;
			printf("Father Process: Ping %d, %d;\n", data, i);
			//sleep(1);
			sleep(128);
		}
		exit();
	}
	//data = 'H';
	//printf("%c\n", data);

	//while(1);
	//return 0;
	return 0;
}
