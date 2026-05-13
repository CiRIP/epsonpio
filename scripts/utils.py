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
):
    ser.write(struct.pack("<BBIII", id, arg, address, data1, data2))
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
):
    global _last_exec_address
    if address != _last_exec_address:
        write_word(ser, DEBUG_RAM_BASE + 100, address)
        _last_exec_address = address

    return command(ser, 1, r11, r12, r13, r14)
