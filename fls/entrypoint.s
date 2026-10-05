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

#define SP_INI 0x2000

	.text
	.global ENTRY
ENTRY:
	ld.w	%r5,%sp		; save SP
	xld.w	%r4,SP_INI
	ld.w	%sp,%r4		; set SP
	ld.w	%r4,%r15	; save return address
	pushn	%r8		; save registers
	ld.w	%r6,%r11
	ld.w	%r7,%r12
	ld.w	%r8,%r13
	xld.w	%r15,0		; set global pointer for safety
	xcall	entry		; enter C program
	ld.w	%r10,%r4
	popn	%r8		; restore registers
	ld.w	%sp,%r5		; restore SP
	ld.w	%r15,%r4	; restore return address
	jp	%r15		; back to mini monitor
