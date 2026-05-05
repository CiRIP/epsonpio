import argparse
from pathlib import Path
import serial

from utils import DEBUG_RAM_BASE, command, write_byte, write_word


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("file", type=Path)
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=1)

    data = args.file.read_bytes()
    for i, byte in enumerate(data):
        write_byte(ser, 0x100 + i, byte)
    write_word(ser, DEBUG_RAM_BASE + 100, 0x100)

    print(command(ser, 1, address=0x02).hex())


if __name__ == "__main__":
    main()
