#include "x86.h"
#include "device.h"
#include "fs.h"

void kEntry(void) {

	// Interruption is disabled in bootloader

	initSerial();// initialize serial port
	initIdt(); // initialize idt
	initIntr(); // iniialize 8259a
	initSeg(); // initialize gdt, tss
	initVga(); // initialize vga device
	initTimer(); // initialize timer device
	initKeyTable(); // initialize keyboard device
	initSem(); // initialize semaphore list, i.e., semaphore descriptor
	initFS();  // initialize filesystem superBlock & groupDesc
	initDev(); // initialize device list, i.e., device descriptor
	initFile(); // initialize file list, i.e., file descriptor
	initProc(); // initialize pcb & load user program
}
