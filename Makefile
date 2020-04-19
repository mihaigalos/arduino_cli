# WinAVR cross-compiler toolchain is used here
CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude

# If you are not using atmega328p and the USBtiny programmer, 
# update the lines below to match your configuration
CFLAGS = -std=c11 -Wall -Os -Isrc/usbdrv -mmcu=atmega328p
OBJFLAGS = -j .text -j .data -O ihex
DUDEFLAGS = -p atmega328p -c usbtiny -v

# Object files for the firmware (usbdrv/oddebug.o not strictly needed I think)
OBJECTS = src/usbdrv/usbdrv.o src/usbdrv/oddebug.o src/usbdrv/usbdrvasm.o src/main.o

# Command-line client
CMDLINE = arduino_cli.exe

# By default, build the firmware and command-line client, but do not flash
all: main.hex $(CMDLINE)

# With this, you can flash the firmware by just typing "make flash" on command-line
flash: main.hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

# One-liner to compile the command-line client from arduino_cli.c
$(CMDLINE): src/arduino_cli.cpp
	g++ -std=c++1y -Wl,--no-as-needed -ldl -O -Wall src/arduino_cli.cpp src/libusb/libusb_dyn.c -o arduino_cli

# Housekeeping if you want it
clean:
	$(RM) src/*.o *.hex *.elf src/usbdrv/*.o arduino_cli

# From .elf file to .hex
%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@

# Main.elf requires additional objects to the firmware, not just main.o
main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Without this dependance, .o files will not be recompiled if you change 
# the config! I spent a few hours debugging because of this...
$(OBJECTS): src/usbdrv/usbconfig-prototype.h

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
