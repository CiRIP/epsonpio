/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (C) 2026 Ciprian Ionescu <me@ciprian-ionescu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
OUTPUT_FORMAT("elf32-c33", "elf32-c33", "elf32-c33")
OUTPUT_ARCH(c33)

__dp            = 0x00000000;
__START_stack   = 0x00004000;

SECTIONS
{
    .bss 0x00000030 :
    {
        __START_bss = . ;
        *(.bss)
        *(.comm)
        __END_bss = . ;
    }

    .data :
    {
        __START_data = . ;
        *(.data)
        __END_data = . ;
    }

    .text 0x00000100 :
    {
        __START_text = . ;
        *(.text)
        __END_text = . ;
    }

    .rodata :
    {
        __START_rodata = . ;
        *(.rodata)
        __END_rodata = . ;
    }
}
