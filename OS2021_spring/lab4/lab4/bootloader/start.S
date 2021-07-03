/* Protected Mode Loading Hello World APP */
.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	cli # clear interuption
	inb $0x92, %al # Fast setup A20 Line with port 0x92, necessary or not?
	orb $0x02, %al
	outb %al, $0x92
	data32 addr32 lgdt gdtDesc # loading gdtr, data32, addr32
	movl %cr0, %eax
	orb $0x01, %al
	movl %eax, %cr0 # setting cr0
	data32 ljmp $0x08, $start32 # reload code segment selector and ljmp to start32, data32

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	
	movl $0x200000, %eax # setting esp
	movl %eax, %esp
	jmp bootMain # jump to bootMain in boot.c

.p2align 2
gdt: # 8 bytes for each table entry, at least 1 entry
	.word 0,0 # empty entry
	.byte 0,0,0,0

	.word 0xffff,0 # code segment entry
	.byte 0,0x9a,0xcf,0

	.word 0xffff,0 # data segment entry
	.byte 0,0x92,0xcf,0

	.word 0xffff,0x8000 # graphics segment entry
	.byte 0x0b,0x92,0xcf,0

gdtDesc: # 6 bytes in total
	.word (gdtDesc - gdt -1) # size of the table, 2 bytes, 65536-1 bytes, 8192 entries
	.long gdt # offset, i.e. linear address of the table itself
