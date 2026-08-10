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
export CYGWIN=nodosfilewarning

TOOL_DIR = C:/EPSON/GNU33
TARGET   = fls

GCC = $(TOOL_DIR)/xgcc
LD  = $(TOOL_DIR)/ld

CFLAGS  = -B$(TOOL_DIR)/ -mc33pe -gstabs -medda32 -O1 -I$(TOOL_DIR)/include -fno-builtin -mlong-calls -Wall -Werror-implicit-function-declaration -std=c99
ASFLAGS = -B$(TOOL_DIR)/ -mc33pe -c -xassembler-with-cpp -Wa,--gstabs -Wa,-medda32
LDFLAGS = -Map $(TARGET).map -N -T $(TARGET).lds

OBJS = entrypoint.o main.o packbits.o crc.o

.PHONY: all clean

all: $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

entrypoint.o: entrypoint.s
	$(GCC) $(ASFLAGS) -o $@ $<

main.o: main.c
	$(GCC) $(CFLAGS) -c -o $@ $<
	
packbits.o: packbits.c
	$(GCC) $(CFLAGS) -c -o $@ $<

crc.o: crc.c
	$(GCC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).map
