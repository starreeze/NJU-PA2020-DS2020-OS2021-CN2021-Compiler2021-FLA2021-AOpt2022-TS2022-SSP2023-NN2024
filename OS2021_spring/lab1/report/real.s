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