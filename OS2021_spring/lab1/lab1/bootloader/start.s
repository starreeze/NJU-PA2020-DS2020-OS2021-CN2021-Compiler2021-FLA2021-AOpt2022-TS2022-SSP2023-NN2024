/* Real Mode Hello World */
/*
.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	movw $0x7d00, %ax
	movw %ax, %sp # setting stack pointer to 0x7d00

	movw $message, %bp
	movb $0x13, %ah
	movb $1, %al
	movw $14, %cx
	movb $0, %dl
	movb $0, %dh
	movw $0x000f, %bx
	int $0x10

loop:
	jmp loop

message:
	.string "Hello, World!\n"
*/

/* Protected Mode Hello World */
/*
.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	cli
	inb $0x92, %al
	orb $2, %al
	outb %al, $0x92
	data32 addr32 lgdt gdtDesc
	data32 mov %cr0, %eax
	orb $1, %al
	data32 mov %eax, %cr0
	data32 addr32 ljmp $0x08, $start32

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs

	# for(int i=0; i<24; ++i)	*(0xb8000 + i) = message[i];
	movl $0, %eax
.L1:
	movl %eax, %ebx
	shl $2, %ebx
	movl $message, %ecx
	addl %ebx, %ecx
	addl $0xb8000, %ebx
	movl (%ecx), %edx
	movl %edx, (%ebx)
	incl %eax
	cmpl $7, %eax
	jb .L1

loop32:
	jmp loop32

message:
	# Hello, World!\n\0
	.byte 0x48, 0x0f, 0x65, 0x0f, 0x6c, 0x0f, 0x6c, 0x0f, 0x6f, 0x0f, 0x2c, 0x0f, 0x20, 0x0f, 0x57, 0x0f, 0x6f, 0x0f, 0x72, 0x0f, 0x6c, 0x0f, 0x64, 0x0f, 0x21, 0x0f, 0x00, 0x0f


.p2align 2
gdt:
	# GDT definition here
	.word 0, 0
	.byte 0, 0, 0, 0

	.word 0xffff, 0
	.byte 0, 0x9a, 0xcf, 0

	.word 0xffff, 0
	.byte 0, 0x92, 0xcf, 0

	.word 0xffff, 0x8000
	.byte 0xb, 0x92, 0xcf, 0

gdtDesc:
	# gdtDesc definition here
	.word (gdtDesc - gdt -1)
	.long gdt
*/
/* Protected Mode Loading Hello World APP */

.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	# TODO: Protected Mode Here
	cli
	inb $0x92, %al
	orb $2, %al
	outb %al, $0x92
	data32 addr32 lgdt gdtDesc
	data32 mov %cr0, %eax
	orb $1, %al
	data32 mov %eax, %cr0
	data32 addr32 ljmp $0x08, $start32

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	
	movl $0x8000, %eax # setting esp
	movl %eax, %esp
	jmp bootMain # jump to bootMain in boot.c

.p2align 2
gdt:
	# GDT definition here
	.word 0, 0
	.byte 0, 0, 0, 0

	.word 0xffff, 0
	.byte 0, 0x9a, 0xcf, 0

	.word 0xffff, 0
	.byte 0, 0x92, 0xcf, 0

	.word 0xffff, 0x8000
	.byte 0xb, 0x92, 0xcf, 0

gdtDesc:
	# gdtDesc definition here
	.word (gdtDesc - gdt -1)
	.long gdt
