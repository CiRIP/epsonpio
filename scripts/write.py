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
import argparse
import time
from functools import partial
from pathlib import Path
import struct
from typing import BinaryIO
import serial
import tqdm

from loadelf import load_elf
from utils import DEBUG_ROM_BASE, exec, write_byte, packbits


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("flasher", type=Path)
    parser.add_argument("file", type=Path)
    parser.add_argument("--start", default="0x2000000", type=lambda x: int(x, 0))
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=60)

    print("Configuring misc registers...")
    write_byte(ser, 0x300020, 0x96)
    write_byte(ser, 0x300016, 0x00)
    write_byte(ser, 0x300018, 0x01)
    # write_byte(ser, 0x302229, 0x00)

    print("Setting bus speed to SYSCLK...")
    write_byte(ser, DEBUG_ROM_BASE + 0x18067, 0x03)

    print("Uploading flasher...")
    exports = load_elf(ser, args.flasher)

    if "FLASH_ERASE" not in exports or "FLASH_LOAD" not in exports:
        print("Flasher is missing required exports!")
        return

    print("Wiping flash... (This will take approx. 45 seconds)")
    ret = exec(ser, exports["FLASH_ERASE"], r12=0x2000000, r13=0x00, r14=0x01)

    if ret[0] != 0:
        print(f"Failed to wipe flash: 0x{ret[0]:02x}")
        return

    print("Done.")

    with args.file.open("rb") as f:
        f: BinaryIO
        written = 0
        total = args.file.stat().st_size
        with tqdm.tqdm(total=total, unit="B", unit_scale=True) as bar:
            exec(ser, exports["BULK_LOAD"], r12=args.start, r13=args.start + total, skip_response=True)
            for chunk in iter(partial(f.read, 32), b""):
                ser.write(chunk)
                bar.update(len(chunk))
            ser.read(8)


if __name__ == "__main__":
    main()
