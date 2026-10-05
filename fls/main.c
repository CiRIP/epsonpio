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
#include "crc.h"
#include "packbits.h"

#define FLASH 0x2000000UL
#define DQ5   0x20
#define DQ6   0x40

#define BUFFER      ((unsigned char *)0x2000)
#define BUFFER_SIZE 0x3000
#define BUFFER_END  (BUFFER + BUFFER_SIZE)

#define CMU_GATEDCLK0 (*(volatile unsigned long *)0x301B00)
#define CMU_PROTECT   (*(volatile unsigned long *)0x301B24)
#define LCDC_IRAM     (*(volatile unsigned long *)0x301A64)
#define LCDCSAPB_CKE  0x2

#define DBG_TX     (*(volatile unsigned char *)0x78018)
#define DBG_RX     (*(volatile unsigned char *)0x78019)
#define DBG_STATUS (*(volatile unsigned char *)0x7801A)
#define RDBF       0x1
#define TDBE       0x2

#define FLAG   0x7E
#define ESCAPE 0x7D
#define READY  0xA5

#define REG(addr) (*(volatile unsigned short *)(addr))

enum { READ, ERASE, MAP, WRITE };
enum { OK, BAD_FRAME, BAD_FLASH, BAD_BUFFER };

static unsigned long  cursor;
static unsigned short word;
static int            status;

static void tx(unsigned char c) {
    while (!(DBG_STATUS & TDBE))
        ;
    DBG_TX = c;
}

static inline unsigned char rx(void) {
    while (!(DBG_STATUS & RDBF))
        ;
    return DBG_RX;
}

static void flash_command(unsigned short command) {
    REG(FLASH + 0xAAA) = 0xAA;
    REG(FLASH + 0x554) = 0x55;
    REG(FLASH + 0xAAA) = command;
}

static int flash_wait(unsigned long addr, unsigned short expected) {
    unsigned short value;

    while (((value = REG(addr)) ^ REG(addr)) & DQ6) {
        if ((value & DQ5) && (REG(addr) ^ REG(addr)) & DQ6) {
            REG(FLASH) = 0xF0;
            return BAD_FLASH;
        }
    }

    return REG(addr) == expected ? OK : BAD_FLASH;
}

static int erase(void) {
    flash_command(0x80);
    flash_command(0x10);

    return flash_wait(FLASH, 0xFFFF);
}

static void program(unsigned long addr, unsigned short data) {
    if (data == 0xFFFF)
        return;

    flash_command(0xA0);
    REG(addr) = data;

    status |= flash_wait(addr, data);
}

static void emit(unsigned char c) {
    if (cursor & 1)
        program(cursor - 1, word | c << 8);
    else
        word = c;

    cursor++;
}

static int map_buffer(void) {
    CMU_PROTECT = 0x96;
    CMU_GATEDCLK0 |= LCDCSAPB_CKE;
    CMU_PROTECT = 0;

    LCDC_IRAM = 1;

    return LCDC_IRAM & 1 ? OK : BAD_BUFFER;
}

static int write(unsigned long addr, unsigned int expected) {
    unsigned char *end  = BUFFER;
    unsigned int   crc  = ~0;
    unsigned char  flip = 0;
    unsigned char  c;

    tx(READY);

    while ((c = rx()) != FLAG) {
        if (c == ESCAPE) {
            flip = 0x20;
            continue;
        }

        c ^= flip;
        flip = 0;

        crc = crc32_update(crc, c);

        if (end < BUFFER_END)
            *end = c;
        end++;
    }

    if (~crc != expected || end > BUFFER_END)
        return BAD_FRAME;

    cursor = addr;
    status = OK;
    unpackbits(BUFFER, end, emit);

    return status;
}

unsigned long entry(unsigned long op, unsigned long a, unsigned long b) {
    switch (op) {
    case READ:
        return packbits((unsigned char *)a, b, tx);
    case ERASE:
        return a == FLASH ? erase() : ~0UL;
    case MAP:
        return map_buffer();
    case WRITE:
        return write(a, b);
    }

    return ~0UL;
}
