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
