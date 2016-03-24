section .data
    message: db "Enter numbers : "
    message_length: equ $-message
  
section .bss
	i: 	    resw    1			;临时计数变量
    	digit:      resb    1			;输入输出时的缓存数字
    	temp:       resd    1
	count:      resb    1			;输入的参数个数
	address:    resd    1			;地址存放
	isInsert:   resb    1			;是否插入numbers
	numbers:    resb    30			;输入的参数
	result:     resd    30			;生成的结果
	
section .text
    global main
	
main:
    ;Printing prompt message
	mov eax, 4
	mov ebx, 1
	mov ecx, message
	mov edx, message_length
	int 80h
	 
	JMP getInput
	 
exit:
	mov eax, 1
	mov ebx, 0
	int 80h	

fibo:				;所求为第x项，x在dx中，值放在ebx地址中
	mov eax,1
	mov dword[temp],1
	
	CMP dx,2
	JNA fiboEND
	
	mov word[i],2
fiboLoop:
	mov ecx,dword[temp]
	add dword[temp],eax
	mov eax,ecx
	inc word[i]

	CMP dx,word[i]
	JA fiboLoop
	
fiboEND:
	mov eax,dword[temp]
	mov dword[ebx],eax
	
JMP	operateEND
	
getInput:

	mov ebx,result

	mov byte[isInsert],0
	mov byte[count],0
	mov byte[temp],0
	push ebx
JMP getint
inNextJ1:	 
	;CMP byte[digit],13
	;JE getInputEND

	CMP byte[isInsert],0
	JE continueIN
		
	pop ebx			;弹出result地址
	movzx dx,byte[temp]

JMP     fibo

operateEND:
	add ebx,4
	inc byte[count]
	push ebx
	mov byte[isInsert],0
	
continueIN:	
	CMP byte[digit],10
	JE getInputEND

    	mov byte[temp],0	 
getint:
	;Reding the digit
	mov eax, 3
	mov ebx, 0
	mov ecx, digit
	mov edx, 1
	int 80h
	
	;判断输入是否为数字，如果不是，如果为回车结束输入，如果为其他返回上一层
	CMP byte[digit],30h
	JB inNextJ1
	CMP byte[digit],3Ah
	JNB inNextJ1
	
	;Calculating the number from digits
	sub byte[digit], 30h
	
	mov al, byte[temp]
	mov bl, 10
	mul bl
	movzx bx, byte[digit]
	add ax, bx
	mov byte[temp], al
	mov byte[isInsert],1
	
JMP     getint	
	
getInputEND:	
JMP	print
	
print:
	mov ebx,result
	
outLoop:	
	mov eax,dword[ebx]
	mov dword[temp],eax
	push ebx
	mov byte[i],0
outint:
	mov eax,dword[temp]	
	mov ebx,10
	mov edx,0
	div ebx 
	
	mov dword[temp],eax
	mov byte[digit],dl
	add byte[digit],30h
	
	movzx ax,byte[digit]
	push ax
	inc byte[i]
	
	CMP dword[temp],0      ;如果temp大于0,继续除10
	JA  outint

output:
	pop ax
	mov byte[digit],al
	
	mov eax, 4
	mov ebx, 1
	mov ecx, digit
	mov edx, 1
	int 80h
    
	dec byte[i]
	CMP byte[i],0
	JA output
	
	mov byte[digit],10
	
	mov eax, 4
	mov ebx, 1
	mov ecx, digit
	mov edx, 1
	int 80h
	
	pop ebx
	add ebx,4
	sub byte[count],1
	
	CMP byte[count],0
	JA outLoop
JMP 	exit
	
