/*****************************************************************************
packbits.c  -  run length encoding and decoding using MacPaint / TIFF format.

MIT License

Copyright (c) 2021 Simon Large, Skirrid Systems

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This is a very simple form of run length encoding which packs repeated bytes.
It is not the most efficient packing method, but it takes very little resource
to unpack the data and all the data is inline with no header structure.
This makes it usable on both large and small objects.

It is particularly useful for packing low colour graphics objects up to
8-bits per pixel, where it can efficiently compress runs of identical pixels.

The output stream consists of a header byte which indicates the uncompressed
length and whether the data is repeated or differing, followed by either
a single byte to be repeated or the set of differeing data.

The maximum number of bytes with a single header byte is 128.

Header = 0..127     (1 + n) literal bytes
Header = 129..255   (257 - n) repeated bytes
Header = 128        No operation

https://en.wikipedia.org/wiki/PackBits

Source on GitHub:
https://github.com/skirridsystems/packbits
******************************************************************************/

#include "packbits.h"
#include <stdbool.h>

#define MIN_REPT 3   // Minimum run to compress between differ blocks
#define MAX_REPT 128 // Maximum run of repeated byte
#define MAX_DIFF 128 // Maximum run of differing bytes

// Encoding for header byte based on number of bytes represented.
#define ENCODE_DIFF(n) (unsigned char)((n) - 1)
#define ENCODE_REPT(n) (unsigned char)(1 - (n))

// Decoding for header byte to give output run length
#define IS_DIFF(h)     ((h) < 128)
#define IS_REPT(h)     ((h) > 128)
#define DECODE_DIFF(h) (unsigned char)((h) + 1)
#define DECODE_REPT(h) (unsigned char)(1 - (h))

static unsigned int lut[16] = {0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4,
                               0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
                               0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C};

/*----------------------------------------------------------------------------
packbits compresses srcCount bytes from srcPtr, emitting output one byte
at a time via the put callback.

In the pathological case where there are no runs (e.g. an incrementing
byte counter) there is an overhead of 1 byte for each 128 bytes of source.

Returns the CRC32 of the input data, which can be used to verify the integrity
of the data after unpacking.
----------------------------------------------------------------------------*/
unsigned int packbits(const unsigned char *srcPtr, unsigned int srcCount, void (*put)(unsigned char)) {
    bool                 inRun        = false;
    unsigned int         bytesPending = 0;
    const unsigned char *pendingPtr;
    unsigned int         runStart = 0;
    unsigned char        currByte;
    unsigned char        lastByte;
    unsigned int         crc = ~0;

    if (srcCount == 0)
        return 0;

    pendingPtr = srcPtr;
    lastByte   = *srcPtr++;

    crc = lut[(crc ^ lastByte) & 0x0F] ^ (crc >> 4);
    crc = lut[(crc ^ (lastByte >> 4)) & 0x0F] ^ (crc >> 4);

    ++bytesPending;

    while (--srcCount != 0) {
        currByte = *srcPtr++;

        crc = lut[(crc ^ currByte) & 0x0F] ^ (crc >> 4);
        crc = lut[(crc ^ (currByte >> 4)) & 0x0F] ^ (crc >> 4);

        ++bytesPending;

        if (inRun) {
            if ((currByte != lastByte) || (bytesPending > MAX_REPT)) {
                put(ENCODE_REPT(bytesPending - 1));
                put(lastByte);
                bytesPending = 1;
                pendingPtr   = srcPtr - 1;
                runStart     = 0;
                inRun        = false;
            }
        } else {
            if (bytesPending > MAX_DIFF) {
                put(ENCODE_DIFF(MAX_DIFF));
                for (unsigned int i = 0; i < MAX_DIFF; i++)
                    put(pendingPtr[i]);
                pendingPtr += MAX_DIFF;
                bytesPending -= MAX_DIFF;
                runStart = bytesPending - 1;
            } else if (currByte == lastByte) {
                if ((bytesPending - runStart >= MIN_REPT) || (runStart == 0)) {
                    if (runStart != 0) {
                        put(ENCODE_DIFF(runStart));
                        for (unsigned int i = 0; i < runStart; i++)
                            put(pendingPtr[i]);
                    }
                    bytesPending -= runStart;
                    inRun = true;
                }
            } else {
                runStart = bytesPending - 1;
            }
        }
        lastByte = currByte;
    }

    if (inRun) {
        put(ENCODE_REPT(bytesPending));
        put(lastByte);
    } else {
        put(ENCODE_DIFF(bytesPending));
        for (unsigned int i = 0; i < bytesPending; i++)
            put(pendingPtr[i]);
    }

    return ~crc;
}

/*----------------------------------------------------------------------------
unpackbits decompresses a stream, reading one byte at a time via the get
callback and emitting output one byte at a time via the put callback.

get should return the next byte from the compressed stream.
put receives each decompressed byte.

destCount specifies the maximum number of bytes to decompress. Unpacking
stops when destCount bytes have been emitted.

Returns the number of bytes emitted.
----------------------------------------------------------------------------*/
unsigned int unpackbits(unsigned char (*get)(void), void (*put)(unsigned char), unsigned int destCount) {
    unsigned char hdr;
    unsigned char count;
    unsigned int  destRemaining = destCount;

    while (destRemaining != 0) {
        hdr = get();

        if (IS_DIFF(hdr)) {
            count = DECODE_DIFF(hdr);
            if (count > destRemaining)
                count = destRemaining;
            for (unsigned int i = 0; i < count; i++)
                put(get());
            destRemaining -= count;
        } else if (IS_REPT(hdr)) {
            count = DECODE_REPT(hdr);
            if (count > destRemaining)
                count = destRemaining;
            unsigned char byte = get();
            for (unsigned int i = 0; i < count; i++)
                put(byte);
            destRemaining -= count;
        }
        // header == 128 is a no-op, loop continues
    }

    return destCount - destRemaining;
}
