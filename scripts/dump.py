from pathlib import Path
import serial
import struct
import argparse
import sys
from tqdm import tqdm
from utils import DEBUG_ROM_BASE, write_byte

def command(ser, address):
    ser.write(struct.pack("<BBIII", 3, 0, address, 0, 0))
    return ser.read(8)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("start", type=lambda x: int(x, 0))
    parser.add_argument("end", type=lambda x: int(x, 0))
    parser.add_argument("output")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=1)

    print("Configuring misc registers...")
    write_byte(ser, 0x300020, 0x96)
    write_byte(ser, 0x300016, 0x00)
    write_byte(ser, 0x300018, 0x01)

    print("Setting bus speed to SYSCLK...")
    write_byte(ser, DEBUG_ROM_BASE + 0x18067, 0x02)

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