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
