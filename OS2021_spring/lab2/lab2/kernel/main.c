#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void)
{
	// Interruption is disabled in bootloader
	log("Enter kernel main.\n");
	initSerial(); // initialize serial port
	// TODO: 做一系列初始化
	clearBuf(&inputBuf);
	// initialize idt
	initIdt();
	// iniialize 8259a
	initIntr();
	// initialize gdt, tss
	initSeg();
	initVga(); // initialize vga device
	// initialize keyboard device
	initKeyTable();
	log("Initialization successful.\n");
	loadUMain(); // load user program, enter user space

	while (1)
		;
	assert(0);
}
