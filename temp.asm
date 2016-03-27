outLoop:	
	mov eax,dword[ebx]
	mov dword[temp],eax
	push ebx
	mov byte[i],0
outint:
	mov eax,dword[ebx+8]	
	mov edx,0
	div dword[divisor]
	mov dword[ebx+8],eax

	CMP edx,10
	JB  clcMid
	mov edx,0
	
clcMid:	
	mov eax,dword[ebx+4]
	div dword[divisor]
	mov dword[ebx+4],eax

	CMP edx,10
	JB  clcLow
	mov edx,0
	
clcLow:	
	mov eax.dword[ebx]
	div dword[divisor]
	mov dword[ebx],eax
	
	mov byte[digit],dl
	add byte[digit],30h
	
	movzx ax,byte[digit]
	push ax
	inc byte[i]
	
	mov edx,0
	
	CMP dword[ebx+8],0      ;如果存的数大于0,继续除10
	JA  outint
	CMP dword[ebx+4],0      ;如果存的数大于0,继续除10
	JA  clcMid
	CMP dword[ebx],0      ;如果存的数大于0,继续除10
	JA  clcLow