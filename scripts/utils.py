import struct
from typing import Union

import serial


DEBUG_ROM_BASE = 0x60000
DEBUG_RAM_BASE = 0x84780


def command(
    ser: serial.Serial,
    id: int,
    arg: int = 0,
    address: int = 0,
    data1: int = 0,
    data2: int = 0,
    skip_response: bool = False,
):
    ser.write(struct.pack("<BBIII", id, arg, address, data1, data2))
    if skip_response:
        return b""
    
    res = ser.read(8)
    if len(res) != 8:
        raise RuntimeError("Command timed out. Check your connection and try again.")
    return res


def read(ser: serial.Serial, address: int):
    return command(ser, 3, 0, address)


def read_short(ser: serial.Serial, address: int):
    return command(ser, 5, 0, address)


def write_byte(ser: serial.Serial, address: int, value: int):
    return command(ser, 2, 0, address, value)


def write_word(ser: serial.Serial, address: int, value: int):
    return command(ser, 7, 0, address, value)


_last_exec_address: Union[int, None] = None


def exec(
    ser: serial.Serial,
    address: int,
    r11: int = 0,
    r12: int = 0,
    r13: int = 0,
    r14: int = 0,
    skip_response: bool = False,
):
    global _last_exec_address
    if address != _last_exec_address:
        write_word(ser, DEBUG_RAM_BASE + 100, address)
        _last_exec_address = address

    return command(ser, 1, r11, r12, r13, r14, skip_response=skip_response)


def packbits(src: bytes, put) -> int:
    MIN_REPT = 3
    MAX_REPT = 128
    MAX_DIFF = 128

    def encode_diff(n): return (n - 1) & 0xFF
    def encode_rept(n): return (1 - n) & 0xFF

    if not src:
        return 0

    crc = 0xFFFFFFFF
    lut = [0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4,
           0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
           0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C]

    def update_crc(crc, byte):
        crc = lut[(crc ^ byte) & 0x0F] ^ (crc >> 4)
        crc = lut[(crc ^ (byte >> 4)) & 0x0F] ^ (crc >> 4)
        return crc

    in_run = False
    bytes_pending = 0
    pending_start = 0
    run_start = 0

    last_byte = src[0]
    crc = update_crc(crc, last_byte)
    bytes_pending = 1

    for i in range(1, len(src)):
        curr_byte = src[i]
        crc = update_crc(crc, curr_byte)
        bytes_pending += 1

        if in_run:
            if curr_byte != last_byte or bytes_pending > MAX_REPT:
                put(encode_rept(bytes_pending - 1))
                put(last_byte)
                bytes_pending = 1
                pending_start = i
                run_start = 0
                in_run = False
        else:
            if bytes_pending > MAX_DIFF:
                put(encode_diff(MAX_DIFF))
                for b in src[pending_start:pending_start + MAX_DIFF]:
                    put(b)
                pending_start += MAX_DIFF
                bytes_pending -= MAX_DIFF
                run_start = bytes_pending - 1
            elif curr_byte == last_byte:
                if (bytes_pending - run_start >= MIN_REPT) or (run_start == 0):
                    if run_start != 0:
                        put(encode_diff(run_start))
                        for b in src[pending_start:pending_start + run_start]:
                            put(b)
                    bytes_pending -= run_start
                    in_run = True
            else:
                run_start = bytes_pending - 1

        last_byte = curr_byte

    if in_run:
        put(encode_rept(bytes_pending))
        put(last_byte)
    else:
        put(encode_diff(bytes_pending))
        for b in src[pending_start:pending_start + bytes_pending]:
            put(b)

    return ~crc & 0xFFFFFFFF
