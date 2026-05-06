import argparse
from pathlib import Path
import sys
import serial
from elftools.elf.elffile import ELFFile

from utils import write_byte


def load_elf(ser: serial.Serial, path: Path) -> dict[str, int]:
    exports = {}
    with path.open("rb") as f:
        elf = ELFFile(f)
        for section in [".text", ".data"]:
            print(f"Loading section {section}...", file=sys.stderr)
            elf_section = elf.get_section_by_name(section)
            if elf_section is None:
                continue

            addr = elf_section["sh_addr"]
            data = elf_section.data()

            for i, byte in enumerate(data):
                write_byte(ser, addr + i, byte)

        symtab = elf.get_section_by_name(".symtab")
        if symtab is not None:
            for sym in symtab.iter_symbols():
                if sym["st_info"]["bind"] == "STB_GLOBAL" and sym["st_value"] != 0:
                    exports[sym.name] = sym["st_value"]

    return exports


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("file", type=Path)
    args = parser.parse_args()

    ser = serial.Serial(args.port, 921600, timeout=1)

    load_elf(ser, args.file)


if __name__ == "__main__":
    main()
