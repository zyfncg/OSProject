section .text
global  my_print
my_print:
	push  ebp
	mov   ebp,esp

	mov   ecx,[ebp+8]
	mov   edx,[ebp+12]
	mov   eax,4
	mov   ebx,1
	int 80h	
	
	mov   eax,[ebp+12]
	;leave
	mov esp,ebp
	pop ebp
	
	
	ret	
global func
func:
	mov   eax,[esp+4]
	ret
