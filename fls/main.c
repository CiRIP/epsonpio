#define ADDR_COM1 (0x555 << 1)
#define ADDR_COM2 (0x2AA << 1)

#define ERASE1  0xAA
#define ERASE2  0x55
#define ERASE3  0x80
#define ERASE4  0xAA
#define ERASE5  0x55
#define ERASE6C 0x10
#define ERASE6B 0x30
#define ERASE6S 0x50

#define PROG1 0xAA
#define PROG2 0x55
#define PROG3 0xA0

#define SUCCESS  0
#define E_VERIFY 1
#define E_PARAM1 3
#define E_PARAM2 4
#define E_CHANGE 5
#define E_REGERR 6

#define BLKSIZE 65536
#define BLKMAX  128
#define INIDAT  0xFFFF
#define MSKLOW  0xFFFF
#define DQ6     0x0040

#define REG(addr) (*((volatile unsigned short *)(addr)))

#include "dsio.h"
#include "packbits.h"

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef unsigned long  size_t;
#define NULL ((unsigned long)0)

static unsigned long ctrl_reg = 0x2000000;

static unsigned short dq_poll(unsigned long addr) { return REG(addr); }

static int verify(unsigned long addr, unsigned long expected) {
    unsigned long old = INIDAT, cur;
    while (1) {
        cur = dq_poll(addr) & DQ6;
        if (old == cur)
            break;
        old = cur;
    }
    return ((dq_poll(addr) & MSKLOW) == (expected & MSKLOW)) ? SUCCESS : E_VERIFY;
}

static void erase_cmd(unsigned long addr, int mode) {
    REG(ctrl_reg + ADDR_COM1) = ERASE1;
    REG(ctrl_reg + ADDR_COM2) = ERASE2;
    REG(ctrl_reg + ADDR_COM1) = ERASE3;
    REG(ctrl_reg + ADDR_COM1) = ERASE4;
    REG(ctrl_reg + ADDR_COM2) = ERASE5;
    if (mode == 1)
        REG(ctrl_reg + ADDR_COM1) = ERASE6C;
    else
        REG(addr) = ERASE6S;
}

static void program_word(unsigned long addr, unsigned long data) {
    REG(ctrl_reg + ADDR_COM1) = PROG1;
    REG(ctrl_reg + ADDR_COM2) = PROG2;
    REG(ctrl_reg + ADDR_COM1) = PROG3;
    REG(addr)                 = (unsigned short)data;
}

int flash_erase(unsigned long base, unsigned long start, unsigned long end) {
    ctrl_reg = base;

    if (base % BLKSIZE)
        return E_REGERR;
    if ((long)start == -1)
        return E_CHANGE;
    if ((long)start > BLKMAX || (long)start < 0)
        return E_PARAM1;
    if ((long)end > BLKMAX || (long)end < 0)
        return E_PARAM2;

    if (start == 0) {
        erase_cmd(base, 1);
        return verify(base, INIDAT);
    }

    for (start--, end--; start <= end; start++) {
        unsigned long addr = start * BLKSIZE + base;

        erase_cmd(addr, 2);

        int ret = verify(addr, INIDAT);
        if (ret != SUCCESS)
            return ret;
    }

    return SUCCESS;
}

int flash_load(unsigned long addr, unsigned long data1, unsigned long data2) {
    if (ctrl_reg % BLKSIZE)
        return E_REGERR;

    for (int i = 0; i < 4; i++) {
        unsigned long word = (i < 2 ? data1 : data2) & MSKLOW;

        if (i < 2)
            data1 >>= 16;
        else
            data2 >>= 16;

        if (i == 0 || word != INIDAT) {
            if ((REG(addr) & word) != word)
                return E_VERIFY;

            program_word(addr, word);

            int ret = verify(addr, word);
            if (ret != SUCCESS)
                return ret;
        }
        addr += 2;
    }

    return SUCCESS;
}

/*---------------------------------------------------------------------------
 Function name: DBG_PutC
 Description  : Put char into debug serial interface.
 Parameters   : ucData  (In)  - transmit data
 Return value : none
 *--------------------------------------------------------------------------*/
void DBG_PutC(unsigned char ucData) {
    while (!SSR_TDBE)
        ;
    STDR_TXD = ucData;
}

/*---------------------------------------------------------------------------
 Function name: DBG_GetC
 Description  : Get char from debug serial interface
 Parameters   : none
 Return value : received data
 *--------------------------------------------------------------------------*/
unsigned char DBG_GetC(void) {
    while (!SSR_RDBF)
        ;
    return SRDR_RXD;
}

int dump(unsigned char *start, unsigned char *end) { return packbits(start, end - start, DBG_PutC); }

unsigned char *wpos;
unsigned char wbuf[2];

void emit(unsigned char c) {
    wbuf[(unsigned long) wpos & 1L] = (c);

    if ((unsigned long) wpos & 1L) {
        unsigned long word = wbuf[0] | (unsigned long)wbuf[1] << 8;
        program_word((unsigned long) wpos & ~1L, word);
        verify((unsigned long) wpos & ~1L, word);
    }

    wpos++;
}

unsigned char buf[64];

int bulk_load(unsigned char *start, unsigned char *end) {
    int ret = SUCCESS;
    unsigned int len = (unsigned int) (end - start);

    // DBG_PutC(0x01); // ACK

    for (unsigned long addr = (unsigned long) start; addr < ((unsigned long) end & ~1UL); addr += 2) {
        unsigned short word = DBG_GetC() | (DBG_GetC() << 8);
        program_word(addr, word);
        ret |= verify(addr, word);
    }

    // for (unsigned long addr = (unsigned long) start; addr < ((unsigned long) end & ~1UL); addr += 2) {
    //     unsigned short word = buf[addr - (unsigned long) start] | ((unsigned short)buf[addr - (unsigned long) start + 1] << 8);
    //     program_word(addr, word);
    //     ret |= verify(addr, word);
    // }

    // if ((unsigned long) end & 1L) {
    //     unsigned char c = buf[(unsigned long) end - (unsigned long) start - 1] | 0xFF00UL;
    //     program_word((unsigned long) end - 1, c);
    //     ret |= verify((unsigned long) end - 1, c);
    // }

    return ret;
}
