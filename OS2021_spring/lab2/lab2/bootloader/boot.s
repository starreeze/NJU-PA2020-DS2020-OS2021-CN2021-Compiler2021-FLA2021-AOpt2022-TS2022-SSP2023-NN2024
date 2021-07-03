
bootloader.elf:     file format elf32-i386


Disassembly of section .text:

00007c00 <start>:
    7c00:	8c c8                	mov    %cs,%eax
    7c02:	8e d8                	mov    %eax,%ds
    7c04:	8e c0                	mov    %eax,%es
    7c06:	8e d0                	mov    %eax,%ss
    7c08:	fa                   	cli    
    7c09:	e4 92                	in     $0x92,%al
    7c0b:	0c 02                	or     $0x2,%al
    7c0d:	e6 92                	out    %al,$0x92
    7c0f:	67 66 0f 01 15       	lgdtw  (%di)
    7c14:	64 7c 00             	fs jl  7c17 <start+0x17>
    7c17:	00 0f                	add    %cl,(%edi)
    7c19:	20 c0                	and    %al,%al
    7c1b:	0c 01                	or     $0x1,%al
    7c1d:	0f 22 c0             	mov    %eax,%cr0
    7c20:	66 ea 28 7c 00 00    	ljmpw  $0x0,$0x7c28
    7c26:	08 00                	or     %al,(%eax)

00007c28 <start32>:
    7c28:	66 b8 10 00          	mov    $0x10,%ax
    7c2c:	8e d8                	mov    %eax,%ds
    7c2e:	8e c0                	mov    %eax,%es
    7c30:	8e e0                	mov    %eax,%fs
    7c32:	8e d0                	mov    %eax,%ss
    7c34:	66 b8 18 00          	mov    $0x18,%ax
    7c38:	8e e8                	mov    %eax,%gs
    7c3a:	66 bc ff ff          	mov    $0xffff,%sp
    7c3e:	e9 c9 00 00 00       	jmp    7d0c <bootMain>
    7c43:	90                   	nop

00007c44 <gdt>:
	...
    7c4c:	ff                   	(bad)  
    7c4d:	ff 00                	incl   (%eax)
    7c4f:	00 00                	add    %al,(%eax)
    7c51:	9a cf 00 ff ff 00 00 	lcall  $0x0,$0xffff00cf
    7c58:	00 92 cf 00 ff ff    	add    %dl,-0xff31(%edx)
    7c5e:	00 80 0b 92 cf 00    	add    %al,0xcf920b(%eax)

00007c64 <gdtDesc>:
    7c64:	1f                   	pop    %ds
    7c65:	00 44 7c 00          	add    %al,0x0(%esp,%edi,2)
    7c69:	00 66 90             	add    %ah,-0x70(%esi)

00007c6c <waitDisk>:
    7c6c:	f3 0f 1e fb          	endbr32 
    7c70:	ba f7 01 00 00       	mov    $0x1f7,%edx
    7c75:	8d 76 00             	lea    0x0(%esi),%esi
    7c78:	ec                   	in     (%dx),%al
    7c79:	25 c0 00 00 00       	and    $0xc0,%eax
    7c7e:	83 f8 40             	cmp    $0x40,%eax
    7c81:	75 f5                	jne    7c78 <waitDisk+0xc>
    7c83:	c3                   	ret    

00007c84 <readSect>:
    7c84:	f3 0f 1e fb          	endbr32 
    7c88:	55                   	push   %ebp
    7c89:	89 e5                	mov    %esp,%ebp
    7c8b:	53                   	push   %ebx
    7c8c:	8b 5d 0c             	mov    0xc(%ebp),%ebx
    7c8f:	b9 f7 01 00 00       	mov    $0x1f7,%ecx
    7c94:	89 ca                	mov    %ecx,%edx
    7c96:	ec                   	in     (%dx),%al
    7c97:	25 c0 00 00 00       	and    $0xc0,%eax
    7c9c:	83 f8 40             	cmp    $0x40,%eax
    7c9f:	75 f3                	jne    7c94 <readSect+0x10>
    7ca1:	b0 01                	mov    $0x1,%al
    7ca3:	ba f2 01 00 00       	mov    $0x1f2,%edx
    7ca8:	ee                   	out    %al,(%dx)
    7ca9:	ba f3 01 00 00       	mov    $0x1f3,%edx
    7cae:	88 d8                	mov    %bl,%al
    7cb0:	ee                   	out    %al,(%dx)
    7cb1:	89 d8                	mov    %ebx,%eax
    7cb3:	c1 f8 08             	sar    $0x8,%eax
    7cb6:	ba f4 01 00 00       	mov    $0x1f4,%edx
    7cbb:	ee                   	out    %al,(%dx)
    7cbc:	89 d8                	mov    %ebx,%eax
    7cbe:	c1 f8 10             	sar    $0x10,%eax
    7cc1:	ba f5 01 00 00       	mov    $0x1f5,%edx
    7cc6:	ee                   	out    %al,(%dx)
    7cc7:	89 d8                	mov    %ebx,%eax
    7cc9:	c1 f8 18             	sar    $0x18,%eax
    7ccc:	83 c8 e0             	or     $0xffffffe0,%eax
    7ccf:	ba f6 01 00 00       	mov    $0x1f6,%edx
    7cd4:	ee                   	out    %al,(%dx)
    7cd5:	b0 20                	mov    $0x20,%al
    7cd7:	89 ca                	mov    %ecx,%edx
    7cd9:	ee                   	out    %al,(%dx)
    7cda:	ba f7 01 00 00       	mov    $0x1f7,%edx
    7cdf:	90                   	nop
    7ce0:	ec                   	in     (%dx),%al
    7ce1:	25 c0 00 00 00       	and    $0xc0,%eax
    7ce6:	83 f8 40             	cmp    $0x40,%eax
    7ce9:	75 f5                	jne    7ce0 <readSect+0x5c>
    7ceb:	8b 4d 08             	mov    0x8(%ebp),%ecx
    7cee:	8d 99 00 02 00 00    	lea    0x200(%ecx),%ebx
    7cf4:	ba f0 01 00 00       	mov    $0x1f0,%edx
    7cf9:	8d 76 00             	lea    0x0(%esi),%esi
    7cfc:	ed                   	in     (%dx),%eax
    7cfd:	89 01                	mov    %eax,(%ecx)
    7cff:	83 c1 04             	add    $0x4,%ecx
    7d02:	39 d9                	cmp    %ebx,%ecx
    7d04:	75 f6                	jne    7cfc <readSect+0x78>
    7d06:	5b                   	pop    %ebx
    7d07:	5d                   	pop    %ebp
    7d08:	c3                   	ret    
    7d09:	8d 76 00             	lea    0x0(%esi),%esi

00007d0c <bootMain>:
    7d0c:	f3 0f 1e fb          	endbr32 
    7d10:	55                   	push   %ebp
    7d11:	89 e5                	mov    %esp,%ebp
    7d13:	56                   	push   %esi
    7d14:	53                   	push   %ebx
    7d15:	be 00 00 10 00       	mov    $0x100000,%esi
    7d1a:	31 db                	xor    %ebx,%ebx
    7d1c:	43                   	inc    %ebx
    7d1d:	83 ec 08             	sub    $0x8,%esp
    7d20:	53                   	push   %ebx
    7d21:	56                   	push   %esi
    7d22:	e8 5d ff ff ff       	call   7c84 <readSect>
    7d27:	81 c6 00 02 00 00    	add    $0x200,%esi
    7d2d:	83 c4 10             	add    $0x10,%esp
    7d30:	81 fb c8 00 00 00    	cmp    $0xc8,%ebx
    7d36:	75 e4                	jne    7d1c <bootMain+0x10>
    7d38:	8b 0d 18 00 10 00    	mov    0x100018,%ecx
    7d3e:	a1 1c 00 10 00       	mov    0x10001c,%eax
    7d43:	8b 90 04 00 10 00    	mov    0x100004(%eax),%edx
    7d49:	b8 00 00 10 00       	mov    $0x100000,%eax
    7d4e:	66 90                	xchg   %ax,%ax
    7d50:	8a 1c 02             	mov    (%edx,%eax,1),%bl
    7d53:	88 18                	mov    %bl,(%eax)
    7d55:	40                   	inc    %eax
    7d56:	3d 00 90 11 00       	cmp    $0x119000,%eax
    7d5b:	75 f3                	jne    7d50 <bootMain+0x44>
    7d5d:	8d 65 f8             	lea    -0x8(%ebp),%esp
    7d60:	5b                   	pop    %ebx
    7d61:	5e                   	pop    %esi
    7d62:	5d                   	pop    %ebp
    7d63:	ff e1                	jmp    *%ecx
