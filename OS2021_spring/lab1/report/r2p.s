cli
inb $0x92, %al
orb $2, %al
outb %al, $0x92
data32 addr32 lgdt gdtDesc
data32 mov %cr0, %eax
orb $1, %al
data32 mov %eax, %cr0
data32 addr32 ljmp $0x08, $start32

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