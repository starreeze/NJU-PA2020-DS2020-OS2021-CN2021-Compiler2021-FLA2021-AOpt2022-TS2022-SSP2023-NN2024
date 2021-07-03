100918:	66 0f be d2          	movsbw %dl,%dx
10091c:	80 ce 0c             	or     $0xc,%dh
10091f:	8d 04 9b             	lea    (%ebx,%ebx,4),%eax
100922:	c1 e0 04             	shl    $0x4,%eax
100925:	03 07                	add    (%edi),%eax
100927:	01 c0                	add    %eax,%eax
100929:	05 00 80 0b 00       	add    $0xb8000,%eax
10092e:	66 89 10             	mov    %dx,(%eax)
100931:	8b 07                	mov    (%edi),%eax
100933:	40                   	inc    %eax
100934:	89 07                	mov    %eax,(%edi)
100936:	83 f8 50             	cmp    $0x50,%eax
100939:	74 12                	je     10094d <syscallPrint+0x7d>
10093b:	cd 20                	int    $0x20
10093d:	46                   	inc    %esi
10093e:	3b 75 d4             	cmp    -0x2c(%ebp),%esi
100941:	74 40                	je     100983 <syscallPrint+0xb3>
100943:	26 8a 16             	mov    %es:(%esi),%dl
100946:	8b 19                	mov    (%ecx),%ebx
100948:	80 fa 0a             	cmp    $0xa,%dl
10094b:	75 cb                	jne    100918 <syscallPrint+0x48>
10094d:	43                   	inc    %ebx
10094e:	89 19                	mov    %ebx,(%ecx)
100950:	c7 07 00 00 00 00    	movl   $0x0,(%edi)
100956:	83 fb 19             	cmp    $0x19,%ebx
100959:	75 e0                	jne    10093b <syscallPrint+0x6b>
10095b:	c7 01 18 00 00 00    	movl   $0x18,(%ecx)
100961:	89 4d cc             	mov    %ecx,-0x34(%ebp)
100964:	8b 5d d0             	mov    -0x30(%ebp),%ebx
100967:	e8 cc fc ff ff       	call   100638 <scrollScreen>
10096c:	8b 4d cc             	mov    -0x34(%ebp),%ecx
10096f:	eb ca                	jmp    10093b <syscallPrint+0x6b>