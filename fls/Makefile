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
