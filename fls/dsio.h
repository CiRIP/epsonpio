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
#ifndef _DEBUG_SERIAL_REG_H
#define _DEBUG_SERIAL_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/* [ Serial Status Register for Debugging ]  SSR            0xFFFFC0 */
/********************************************************************/
union SSR_tag {
    volatile struct {
        unsigned int RDBF : 1;  // Receive data buffer full flag
        unsigned int TDBE : 1;  // Transmit data buffer empty flag
        unsigned int RXDEN : 1; // Receive disable
        unsigned int : 5;       // Reserved
    } bCTL;
    volatile unsigned char ucCTL;
};

/********************************************************************/
/* [ Serial Transmit/Receive Data Register for Debugging ]  SDR 0xFFFFC2 */
/********************************************************************/
union SRDR_tag {
    volatile struct {
        unsigned int RXD : 8; // Receive data
    } bCTL;
    volatile unsigned char ucCTL;
};

union STDR_tag {
    volatile struct {
        unsigned int TXD : 8; // Transmit data
    } bCTL;
    volatile unsigned char ucCTL;
};

/****************************************************************************
 * Macros (#define)
 ****************************************************************************/
#define DBG_SERIAL_BASE (0x60000 + 0x18018)

#define SSR       (*(union SSR_tag *)(DBG_SERIAL_BASE + 0x02)).ucCTL
#define SSR_RDBF  (*(union SSR_tag *)(DBG_SERIAL_BASE + 0x02)).bCTL.RDBF
#define SSR_TDBE  (*(union SSR_tag *)(DBG_SERIAL_BASE + 0x02)).bCTL.TDBE
#define SSR_RXDEN (*(union SSR_tag *)(DBG_SERIAL_BASE + 0x02)).bCTL.RXDEN

#define SRDR     (*(union SRDR_tag *)(DBG_SERIAL_BASE + 0x01)).ucCTL
#define SRDR_RXD (*(union SRDR_tag *)(DBG_SERIAL_BASE + 0x01)).bCTL.RXD

#define STDR     (*(union STDR_tag *)(DBG_SERIAL_BASE + 0x00)).ucCTL
#define STDR_TXD (*(union STDR_tag *)(DBG_SERIAL_BASE + 0x00)).bCTL.TXD

#ifdef __cplusplus
}
#endif
#endif /* _DEBUG_SERIAL_REG_H */
