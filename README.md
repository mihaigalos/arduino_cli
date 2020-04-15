## Arduino CLI

Commandline USB communication with Atmel AtMega devices with no previous USB support.
USB protocol is fully emulated by Software.

## Usage

The tool consists of two parts: `arduino_cli` which is a Ubuntu executable and a `main.hex`, both of which are artefacts of compilation. You'll have to flash the `main.hex` to your microcontroller in order to be able to communicate with it using the `arduino_cli`.

## Building

* with `bazel` : `bazel build //...`
* with `make`  : `make all`

### Credits:

* Joonas Pihlajamaa, published at Code and Life blog, http://codeandlife.com/2012/01/22/avr-attiny-usb-tutorial-part-1/
* libusb-win32 available at http://sourceforge.net/apps/trac/libusb-win32/wiki
* V-USB library available at http://www.obdev.at/avrusb/

These works and their contents are copyrighted by their respective authors.
