; G8RTOS_SchedulerASM.s
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc

	LDR R0, RunningPtr	 ; currently running thread
	LDR R1, [R0] 		 ; R1 = value of RunningPtr
	LDR SP, [R1]		 ; new thread SP; SP = RunningPtr->stack_pointer;
	POP {R4-R11}         ; restore regs r4-11
	POP {R0-R3}          ; restore regs r0-3
	POP {R12}
	ADD SP, SP, #4       ; discard LR from initial stack
	POP {LR}             ; start location
	ADD SP, SP, #4  	 ; discard PSR
	CPSIE I              ; Enable interrupts at processor level
	BX LR                ; start first thread

	.endasmfunc

; PendSV_Handler
;   - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:
	
	.asmfunc

	CPSID I 			;  Prevent interrupt during switch
	PUSH {R4-R11} 		;  Save remaining regs r4-11
	LDR R0, RunningPtr  ;  R0=pointer to RunningPtr, old thread
	LDR R1, [R0] 		;  R1 = RunningPtr
	STR SP, [R1] 		;  Save SP into TCB
	PUSH {R0,LR}		;  Push R0 and Link Register
	BL G8RTOS_Scheduler ;  Branch to G8RTOS_Scheduler for current pointers next
	POP {R0,LR}			;  Pop R0 and Link Register
	LDR R1, [R0] 		;  R1 = RunPt, new thread
	LDR SP, [R1] 		;  new thread SP; SP = RunningPtr->stack_pointer;
	POP {R4-R11} 		;  restore regs r4-11
	CPSIE I 			;  tasks run with interrupts enabled
	BX LR 				;  restore R0-R3,R12,LR,PC,PSR

	.endasmfunc
	
	; end of the asm file
	.align
	.end
