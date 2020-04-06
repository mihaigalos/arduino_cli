# WinAVR cross-compiler toolchain is used here
CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude

# If you are not using atmega328p and the USBtiny programmer, 
# update the lines below to match your configuration
CFLAGS = -std=c11 -Wall -Os -Iusbdrv -mmcu=atmega328p
OBJFLAGS = -j .text -j .data -O ihex
DUDEFLAGS = -p atmega328p -c usbtiny -v

# Object files for the firmware (usbdrv/oddebug.o not strictly needed I think)
OBJECTS = usbdrv/usbdrv.o usbdrv/oddebug.o usbdrv/usbdrvasm.o main.o

# Command-line client
CMDLINE = arduino_usb_linux.exe

# By default, build the firmware and command-line client, but do not flash
all: main.hex $(CMDLINE)

# With this, you can flash the firmware by just typing "make flash" on command-line
flash: main.hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

# One-liner to compile the command-line client from arduino_usb_linux.c
$(CMDLINE): arduino_usb_linux.cpp
	g++ -std=c++1y -Wl,--no-as-needed -ldl -O -Wall arduino_usb_linux.cpp ./libusb/libusb_dyn.c -o arduino_usb_linux

# Housekeeping if you want it
clean:
	$(RM) *.o *.hex *.elf usbdrv/*.o arduino_usb_linux

# From .elf file to .hex
%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@

# Main.elf requires additional objects to the firmware, not just main.o
main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Without this dependance, .o files will not be recompiled if you change 
# the config! I spent a few hours debugging because of this...
$(OBJECTS): usbdrv/usbconfig-prototype.h

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
