import argparse
from functools import partial
from pathlib import Path
import struct
from typing import BinaryIO
import serial
import tqdm

from loadelf import load_elf
from utils import exec, write_byte


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("flasher", type=Path)
    parser.add_argument("file", type=Path)
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=300)

    print("Configuring misc registers...")
    write_byte(ser, 0x300020, 0x96)
    write_byte(ser, 0x300016, 0x00)
    write_byte(ser, 0x300018, 0x01)

    print("Uploading flasher...")
    exports = load_elf(ser, args.flasher)

    if "FLASH_ERASE" not in exports or "FLASH_LOAD" not in exports:
        print("Flasher is missing required exports!")
        return

    print("Wiping flash... This may take a while.")
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
            for chunk in iter(partial(f.read, 8), b""):
                if chunk != b"\xff" * len(chunk):
                    r13, r14 = struct.unpack("<II", chunk.ljust(8, b"\x00"))
                    exec(ser, exports["FLASH_LOAD"], r12=0x2000000 + written, r13=r13, r14=r14)
                written += 8
                bar.update(8)


if __name__ == "__main__":
    main()
