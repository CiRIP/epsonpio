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
