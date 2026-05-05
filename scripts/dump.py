import serial
import struct
import argparse
import sys
from tqdm import tqdm

MAX_RETRIES = 3


def command(ser, address):
    ser.write(struct.pack("<BBIII", 3, 0, address, 0, 0))
    return ser.read(8)


def read_with_retry(ser, address, buf):
    for attempt in range(MAX_RETRIES):
        response = command(ser, address)
        if len(buf) < 4 or buf[-4:] == response[:4]:
            return response
        if attempt == MAX_RETRIES - 1:
            print(
                f"\nIntegrity check failed at 0x{address:08x} after {MAX_RETRIES} retries",
                file=sys.stderr,
            )
            sys.exit(1)
        print(
            f"\nExpected 0x{buf[-4:].hex()} but got 0x{response.hex()}", file=sys.stderr
        )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("start", type=lambda x: int(x, 0))
    parser.add_argument("end", type=lambda x: int(x, 0))
    parser.add_argument("output")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=1)

    total = args.end - args.start
    buf = bytearray()

    with tqdm(total=total, unit="B", unit_scale=True) as bar:
        for address in range(args.start, args.end, 4):
            response = read_with_retry(ser, address, buf)
            new_bytes = response if len(buf) == 0 else response[4:]
            buf.extend(new_bytes)
            bar.update(len(new_bytes))

    with open(args.output, "wb") as f:
        f.write(buf)


if __name__ == "__main__":
    main()
