; SPDX-License-Identifier: AGPL-3.0-or-later
; Copyright (C) 2026 Ciprian Ionescu <me@ciprian-ionescu.com>
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU Affero General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU Affero General Public License for more details.
;
; You should have received a copy of the GNU Affero General Public License
; along with this program. If not, see <https://www.gnu.org/licenses/>.
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


	.global DUMP
DUMP:
	ld.w	%r5,%sp		; save SP
	xld.w	%r4,SP_INI
	ld.w	%sp,%r4		; set SP
	ld.w	%r4,%r15	; save return address
	pushn	%r8		; save registers
	ld.w	%r6,%dbbr
	ld.w	%r6,%r12
	ld.w	%r7,%r13
	ld.w	%r8,%r14
	xld.w	%r15,GP_INI	; set global pointer for safty
	xcall	dump	; enter C program
	ld.w	%r10,%r4
	popn	%r8		; restore registers
	ld.w	%sp,%r5		; restore SP
	ld.w	%r15,%r4	; restore return address
	jp	%r15		; back to mini monitor

	.global BULK_LOAD
BULK_LOAD:
	ld.w	%r5,%sp		; save SP
	xld.w	%r4,SP_INI
	ld.w	%sp,%r4		; set SP
	ld.w	%r4,%r15	; save return address
	pushn	%r8		; save registers
	ld.w	%r6,%dbbr
	ld.w	%r6,%r12
	ld.w	%r7,%r13
	ld.w	%r8,%r14
	xld.w	%r15,GP_INI	; set global pointer for safty
	xcall	bulk_load	; enter C program
	ld.w	%r10,%r4
	popn	%r8		; restore registers
	ld.w	%sp,%r5		; restore SP
	ld.w	%r15,%r4	; restore return address
	jp	%r15		; back to mini monitor


BULK_LOAD_BAK:
    btst       [%r13],0x0
    jreq       BULK_LOAD
    ld.ub      %r8,[%r14]
BULK_LOAD_2:
    btst       [%r13],0x0
    jreq       BULK_LOAD_2
    ld.ub      %r9,[%r14]
    sll        %r9,0x8
    or         %r9,%r8
    ld.h       [%r12]+,%r9
    sub        %r11,0x1
    jrne       BULK_LOAD
    jp         %r15
