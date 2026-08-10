# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2026 Ciprian Ionescu <me@ciprian-ionescu.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
from pathlib import Path
import serial
import struct
import argparse
from binascii import crc32
from tqdm import tqdm
from loadelf import load_elf
from utils import DEBUG_RAM_BASE, DEBUG_ROM_BASE, write_byte, write_word


def command(ser, address):
    ser.write(struct.pack("<BBIII", 3, 0, address, 0, 0))
    return ser.read(8)


SYSCLK_CHOICES = ["SYSCLK/4", "SYSCLK/2", "SYSCLK/1", "SYSCLK/8"]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("flasher", type=Path)
    parser.add_argument("start", type=lambda x: int(x, 0))
    parser.add_argument("end", type=lambda x: int(x, 0))
    parser.add_argument("output")
    parser.add_argument("--bus-speed", choices=SYSCLK_CHOICES, default="SYSCLK/2")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=5)

    print("Configuring misc registers...")
    write_byte(ser, 0x300020, 0x96)
    write_byte(ser, 0x300016, 0x00)
    write_byte(ser, 0x300018, 0x01)

    print(f"Setting bus speed to {args.bus_speed}...")
    write_byte(ser, DEBUG_ROM_BASE + 0x18067, SYSCLK_CHOICES.index(args.bus_speed))

    print("Uploading flasher...")
    exports = load_elf(ser, args.flasher)

    total = args.end - args.start
    buf = bytearray()

    with tqdm(total=total, unit="B", unit_scale=True) as bar:
        write_word(ser, DEBUG_RAM_BASE + 100, exports["DUMP"])
        ser.write(struct.pack("<BBIII", 1, 0, args.start, args.end, 0))

        while len(buf) < total:
            header = ser.read(1)
            if not header:
                raise RuntimeError("Corruption detected. Check your connection and try again.")
            h = header[0]
            if h == 128:
                pass  # NOP
            elif h < 128:
                buf.extend(ser.read(h + 1))
                bar.update(h + 1)
            else:
                byte = ser.read(1)[0]
                buf.extend([byte] * (257 - h))
                bar.update(257 - h)
        
    ret = ser.read(8)
    if not ret:
        raise RuntimeError("Dumping did not finish normally. Check your connection and try again.")
    
    crc, = struct.unpack("<I", ret[:4])

    if crc != crc32(buf):
        raise RuntimeError("CRC mismatch. Dump is corrupted.")

    with open(args.output, "wb") as f:
        f.write(buf)


if __name__ == "__main__":
    main()
