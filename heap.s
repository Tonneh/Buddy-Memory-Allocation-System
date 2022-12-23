		AREA	|.text|, CODE, READONLY, ALIGN=2
		THUMB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; System Call Table
HEAP_TOP	EQU		0x20001000
HEAP_BOT	EQU		0x20004FE0
MAX_SIZE	EQU		0x00004000		; 16KB = 2^14
MIN_SIZE	EQU		0x00000020		; 32B  = 2^5
	
MCB_TOP		EQU		0x20006800      ; 2^10B = 1K Space
MCB_BOT		EQU		0x20006BFE
MCB_ENT_SZ	EQU		0x00000002		; 2B per entry
MCB_TOTAL	EQU		512				; 2^9 = 512 entries
	
INVALID		EQU		-1				; an invalid id
	
;
; Each MCB Entry
; FEDCBA9876543210
; 00SSSSSSSSS0000U					S bits are used for Heap size, U=1 Used U=0 Not Used

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Memory Control Block Initialization
; void _heap_init( )
; this routine must be called from Reset_Handler in startup_TM4C129.s
; before you invoke main( ) in driver_keil
		EXPORT	_heap_init
_heap_init
		; you must correctly set the value of each MCB block
		; complete your code
		LDR		r4, = MCB_TOP 
		MOV 	r5, #MAX_SIZE
		STR 	r5, [r4], #0x2
loop_
		LDR		r6, = MCB_BOT
		cmp 	r4, r6
		BGT		end_
		
		MOV		r5, #0x0
		STR 	r5, [r4], #0x2
		B 		loop_
end_
		BX		lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Kernel Memory Allocation
; void* _k_alloc( int size )
		EXPORT	_kalloc
_kalloc
		; complete your code
		; return value should be saved into r0
		PUSH	{lr}
		LDR		r1, = MCB_TOP
		LDR		r2, = MCB_BOT 
		LDR		r3, = MCB_ENT_SZ
		BL		_ralloc
		POP		{lr}
		MOV		r0, r8
		BX		lr

_ralloc 
		PUSH	{lr}
		; off limits, R15, R14, R13, R7
		; r0 = size
		; R1 = left
		; r2 = right 
		; r3 = ent_size 
		; R4 entire_mcb_addr_space
		; R5 half_mcb_addr_space
		; r6 midpoint 
		; R8 heap_addr	(return register)
		; r9 act entire heap size 
		; r10 act half heap size
		SUB		r4, r2, r1 
		ADD		r4, r4, r3 
		LSR		r5, r4, #1
		ADD		r6, r1, r5  
		MOV 	r8, #0 
		LSL 	r9, r4, #4
		LSL		r10, r5, #4
		
		CMP		r0, r10 					;	if ( size <= act_half_heap_size )
		BGT		greater_than 
		
		PUSH	{r1-r6, r9-r11}				; 	skip R7 and R8 and R0 (R8 is return value) 
		SUB		r2, r6, r3 					; 	midpoint_mcb_addr - mcb_ent_sz 
		BL		_ralloc 
		POP		{r1-r6, r9-r11} 
		CMP		r8, #0x0 					;	 heap_addr == 0 
		BEQ 	equal_zero 
		
		LDR		r11, [r6] 					
		AND		r11, r11, #0x01 
		CMP 	r11, #0x0 					;	 IF( array[ m2a( midpoint_mcb_addr ) ] & 0x01 ) == 0 
		BEQ		equal_zero2 
		
		B		finish_
equal_zero 									;	 if ( heap_addr == 0 ) {	
		PUSH	{r1-r6, r9-r11}
		MOV		r1, r6 					
		BL 		_ralloc 
		
		POP		{r1-r6, r9-r11} 
		B		finish_
equal_zero2 								; 	if ( ( array[ m2a( midpoint_mcb_addr ) ] & 0x01 ) == 0 )
		STRH	r10, [r6] 					; 	setting to act_half_heap_size 
		B		finish_
		
; greater 
greater_than 
		LDR		r11, [r1] 
		AND		r11, r11, #0x01 
		CMP 	r11, #0x0					;	if ( ( array[ m2a( left_mcb_addr ) ] & 0x01 ) != 0 )
		BEQ 	equal_zero3
		
		MOV		r8, #0x0
		B		finish_
equal_zero3 								
		LDRH	r11, [r1] 
		CMP		r11, r9 					;	if ( *(short *)&array[ m2a( left_mcb_addr ) ] < act_entire_heap_size )
		BGE		greater_than2 
		
		MOV		r8, #0x0
		B		finish_
greater_than2 								
		ORR		r11, r9, #0x01	
		STRH	r11, [r1] 
		
		LDR		r12, = MCB_TOP
		SUB		r11, r1, r12 				; 	left - mcb_top 
		LDR		r8, = HEAP_TOP				; 	r8 = heaptop 
		LSL		r11, r11, #0x4				; 	x16 
		ADD		r8, r8, r11					
		B		finish_
finish_
		POP		{lr}
		BX		lr
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Kernel Memory De-allocation
; void *_kfree( void *ptr )	
		EXPORT	_kfree
_kfree
		PUSH	{lr, r0}
		LDR		r11, =HEAP_TOP 
		LDR		r12, =HEAP_BOT
		
		CMP 	r0, r11
		BLT		return_null 
		
		CMP		r0, r12
		BGT		return_null 
		
		LDR		r10, =MCB_TOP
		SUB		r3, r0, r11
		LSR		r3, r3, #4 
		ADD		r0, r3, r10
		BL		_rfree 
		CMP		r0, #0x0 
		BEQ		return_null 
		
		b		done_ 
return_null 
		MOV		R0, #0x0
		b 		done_ 
done_
		POP 	{r0, lr}
		BX		lr 
			
_rfree
		PUSH	{lr}
		; complete your code
		; return value should be saved into r0
		; off limits, r7, r13-r15 
		; r0 = addr 
		; r1 = mcb_contents 
		; r2 = mcb_index 
		; r3 = mcb_disp
		; r4 = my_size 
		; r8 = return register (nvm dont need) 
		; r11 = MCB_TOP 
		; r12 = MCB_BOT
		LDR	 	r11, = MCB_TOP 
		LDR		r12, = MCB_BOT
		LDRH	r1, [r0] 
		SUB		r2, r0, r11 
		LSR		r1, r1, #0x4
		MOV		r3, r1 
		LSL		r1, r1, #0x4
		MOV		r4, r1 
		
		STRH	r1, [r0] 
		
		SDIV	r5, r2, r3 
		TST		r5, #0x1 			;	if ( ( mcb_index / mcb_disp ) % 2 == 0 ) {
		BNE 	not_even 
		
		ADD		r5, r0, r3 
		CMP		r5, r12 			;	if ( mcb_addr + mcb_disp >= mcb_bot )	
		BGE		return_zero_
		
		ADD		r6, r0, r3 
		LDRH	r6, [r6] 
		AND 	r9, r6, #0x0001 
		CMP		r9, #0x0 			; 	if ( ( mcb_buddy & 0x0001 ) == 0 )
		BNE		free_finish_ 
		
		LSR		r6, r6, #0x5 
		LSL		r6, r6, #0x5 
		CMP		r6, r4 
		BNE		free_finish_ 		;	if ( mcb_buddy == my_size ) {
		
		ADD		r10, r0, r3 	
		MOV		r9, #0x0
		STRH	r9, [r10] 
		LSL		r4, r4, #0x1 
		STRH	r4, [r0] 
		
		PUSH	{r0-r6, r9-r12} 
		BL		_rfree
		POP		{r0-r6, r9-r12} 
		
		B		free_finish_ 
		
not_even 
		SUB		r5, r0, r3 
		CMP		r5, r11 
		BLT		return_zero_ 

		LDRH	r6, [r5] 
		AND		r9, r6, #0x0001 
		CMP		r9, #0x0			;	if ( ( mcb_buddy & 0x0001 ) == 0 )
		BNE		free_finish_ 
		
		LSR		r6, r6, #0x5
		LSL		r6, r6, #0x5 
		CMP		r6, r4 				;	if ( mcb_buddy == my_size ) {
		BNE		free_finish_ 
		
		MOV		r10, #0x0 
		STRH	r10, [r0] 
		LSL		r4, r4, #0x1 
		SUB		r10, r0, r3
		STRH	r4, [r10] 
		
		PUSH	{r0-r6, r9-r12} 
		MOV		r0, r10 
		BL		_rfree
		POP		{r0-r6, r9-r12} 
		
		B		free_finish_ 
return_zero_ 
		MOV		r0, #0x0 
		
free_finish_
		POP		{lr}
		BX 		lr
		
		END