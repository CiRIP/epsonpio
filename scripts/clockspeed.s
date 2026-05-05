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
