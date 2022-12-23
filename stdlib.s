		AREA	|.text|, CODE, READONLY, ALIGN=2
		THUMB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void _bzero( void *s, int n )
; Parameters
;	s 		- pointer to the memory location to zero-initialize
;	n		- a number of bytes to zero-initialize
; Return value
;   none
		EXPORT	_bzero
_bzero
		; r0 = s
		; r1 = n
		PUSH {r1-r12,lr}		
		mov r2, #0 
bzero_loop_begin	
		cmp r1, #0 
		beq bzero_loop_end
		
		strb r2, [r0]
		add r0, r0, #1 
		sub r1, r1, #1 
		b bzero_loop_begin
bzero_loop_end
		POP {r1-r12,lr}	
		BX		lr



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; char* _strncpy( char* dest, char* src, int size )
; Parameters
;   dest 	- pointer to the buffer to copy to
;	src		- pointer to the zero-terminated string to copy from
;	size	- a total of n bytes
; Return value
;   dest
		EXPORT	_strncpy
_strncpy
		; r0 = dest
		; r1 = src
		; r2 = size
		PUSH {r1-r12,lr}		
strncpy_loop_begin 
		cmp r2, #0 
		beq strncpy_loop_end 
		
		ldrb r3, [r1] 
		strb r3, [r0] 
		add r1, r1, #1 
		add r0, r0, #1 
		sub r2, r2, #1 
		b strncpy_loop_begin 
strncpy_loop_end 
		POP {r1-r12,lr}	
		BX		lr
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DO NOT UPDATE THIS CODE
;
; void* _malloc( int size )
; Parameters
;	size	- #bytes to allocate
; Return value
;   void*	a pointer to the allocated space
		EXPORT	_malloc
_malloc 
		PUSH 	{r1-r12,lr}		
		MOV		r7, #0x1			; r7 specifies system call number
        SVC     #0x0				; system call
		POP 	{r1-r12,lr}
		
		BX		lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DO NOT UPDATE THIS CODE
;
; void _free( void* addr )
; Parameters
;	size	- the address of a space to deallocate
; Return value
;   none
		EXPORT	_free
_free
		PUSH 	{r1-r12,lr}		
		MOV		r7, #0x2			; r7 specifies system call number
        SVC     #0x0				; system call
		POP 	{r1-r12,lr}
		
		BX 		lr
		
		END