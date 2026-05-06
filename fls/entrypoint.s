; drv_asm.s  1998.04.27
;            2002.05.23 modified register assignment 
; asm entry program for ICD33/MON33 flash command

#define SP_INI 0x0800	; sp is in end of 1KB internal RAM
#define GP_INI 0x0000	; global pointer %r8 is 0x0

	.text
	.global FLASH_ERASE
FLASH_ERASE:
	ld.w	%r5,%sp		; save SP
	xld.w	%r4,SP_INI
	ld.w	%sp,%r4		; set SP
	ld.w	%r4,%r15	; save return address
	pushn	%r8		; save registers
	ld.w	%r6,%r12
	ld.w	%r7,%r13
	ld.w	%r8,%r14
	xld.w	%r15,GP_INI	; set global pointer for safty
	xcall	flash_erase	; enter C program
	ld.w	%r10,%r4
	popn	%r8		; restore registers
	ld.w	%sp,%r5		; restore SP
	ld.w	%r15,%r4	; restore return address
	jp	%r15		; back to mini monitor

	
	.global FLASH_LOAD
FLASH_LOAD:
	ld.w	%r5,%sp		; save SP
	xld.w	%r4,SP_INI
	ld.w	%sp,%r4		; set SP
	ld.w	%r4,%r15	; save return address
	pushn	%r8		; save registers
	ld.w	%r6,%r12
	ld.w	%r7,%r13
	ld.w	%r8,%r14
	xld.w	%r15,GP_INI	; set global pointer for safty
	xcall	flash_load	; enter C program
	ld.w	%r10,%r4
	popn	%r8		; restore registers
	ld.w	%sp,%r5		; restore SP
	ld.w	%r15,%r4	; restore return address
	jp	%r15		; back to mini monitor


    .global SET_CLK_SPEED
SET_CLK_SPEED:
	ld.w	%r5,%sp		; save SP
	xld.w	%r4,0x0800
	ld.w	%sp,%r4		; set SP
	ld.w	%r4,%r15	; save return address
	pushn	%r8		; save registers
	ld.w	%r6,%r12
	ld.w	%r7,%r13
	ld.w	%r8,%r14
	xld.w	%r15,0x0000	; set global pointer for safty
	ld.w	%r10,%dbbr
	xld.b	[%r10+0x18067],%r12
	popn	%r8		; restore registers
	ld.w	%sp,%r5		; restore SP
	ld.w	%r15,%r4	; restore return address
	jp	%r15		; back to mini monitor
