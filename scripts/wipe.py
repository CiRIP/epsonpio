import argparse
import serial

from utils import exec


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=30)

    print("Wiping flash... This may take a while.")
    print(exec(ser, 0x100, r12=0x2000000, r13=0x00, r14=0x01).hex())
    print("Done.")


if __name__ == "__main__":
    main()
