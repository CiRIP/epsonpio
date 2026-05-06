from pathlib import Path
import serial
import struct
import argparse
import sys
from tqdm import tqdm
from loadelf import load_elf
from utils import write_byte, exec

def command(ser, address):
    ser.write(struct.pack("<BBIII", 3, 0, address, 0, 0))
    return ser.read(8)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("flasher", type=Path)
    parser.add_argument("start", type=lambda x: int(x, 0))
    parser.add_argument("end", type=lambda x: int(x, 0))
    parser.add_argument("output")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=1)

    print("Configuring misc registers...")
    write_byte(ser, 0x300020, 0x96)
    write_byte(ser, 0x300016, 0x00)
    write_byte(ser, 0x300018, 0x01)

    print("Uploading flasher...")
    exports = load_elf(ser, args.flasher)
    if "SET_CLK_SPEED" not in exports:
        print("Flasher is missing required exports!")
        return

    exec(ser, exports["SET_CLK_SPEED"], r12=0x02)

    total = args.end - args.start
    buf = bytearray()
    with tqdm(total=total, unit="B", unit_scale=True) as bar:
        for address in range(args.start, args.end, 4):
            response = command(ser, address)
            expected = buf[-4:] if len(buf) >= 4 else None
            if expected and expected != response[:4]:
                print(f"\nIntegrity check failed at 0x{address:08x}", file=sys.stderr)
                break
            new_bytes = response if len(buf) == 0 else response[4:]
            buf.extend(new_bytes)
            bar.update(len(new_bytes))

    with open(args.output, "wb") as f:
        f.write(buf)

if __name__ == "__main__":
    main()