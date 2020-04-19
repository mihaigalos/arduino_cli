/**
 * Code adapted from AVR ATtiny USB Tutorial at http://codeandlife.com/, Joonas
 * Pihlajamaa, joonas.pihlajamaa@iki.fi
 */

// this is libusb, see http://libusb.sourceforge.net/
#include <usb.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

#include <string>
#include <vector>

#include "usb_device.h"

#define LED_OFF 0
#define LED_ON 1
#define SEND_DATA_TO_PC 2
#define MODIFY_REPLY_BUFFER 3
#define RECEIVE_DATA_FROM_PC 4

void showHelp()
{
    printf("Usage:\n");
    printf("arduino_cli on\n");
    printf("arduino_cli off\n");
    printf("arduino_cli out\n");
    printf("arduino_cli write\n");
    printf("arduino_cli in <string>\n");
    exit(1);
}

auto argvToVectorOfStrings(int argc, char** argv) -> std::vector<std::string>
{
    std::string current_exec_name = argv[0]; // Name of the current exec program
    std::vector<std::string> all_args;

    if (argc > 1) {
        all_args.assign(argv + 1, argv + argc);
    }

    return all_args;
}

int main(int argc, char** argv)
{
    usb_dev_handle* handle = NULL;
    size_t nBytes = 0;
    char buffer[255];

    if (argc < 2) {
        showHelp();
    }

    std::vector<std::string> args = argvToVectorOfStrings(argc, argv);

    auto start = std::chrono::high_resolution_clock::now();
    handle = USBDevice::open(0x16C0, "", 0x05DC, "DotPhat");

    if (handle == NULL) {
        fprintf(stderr, "Could not find USB device!\n");
        exit(1);
    }

    if (args[1] == "on") {
        nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                                 LED_ON, 0, 0, (char*)buffer, sizeof(buffer), 5000);
    }
    else if (args[1] == "off") {
        nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                                 LED_OFF, 0, 0, (char*)buffer, sizeof(buffer), 5000);
    }
    else if (args[1] == "out") {
        nBytes = usb_control_msg(
            handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
            SEND_DATA_TO_PC, 0, 0, (char*)buffer, sizeof(buffer), 5000);

        std::cout << "Got " << nBytes << " bytes: " << std::endl;

        for (size_t i = 0; i < nBytes; ++i) {
            if (i > 0 && 0 == i % 8) {
                std::cout << std::endl;
            }

            auto value = static_cast<uint16_t>(static_cast<uint8_t>(buffer[i]));

            std::cout << std::hex << " ";

            if (value < 16) {
                std::cout << "0";
            }

            std::cout << value;
        }

        std::cout << std::dec << std::endl;
    }
    else if (args[1] == "write") {
        nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                                 MODIFY_REPLY_BUFFER, 'T' + ('E' << 8),
                                 'S' + ('T' << 8), (char*)buffer, sizeof(buffer), 5000);
    }
    else if (args[1] == "in" && argc > 2) {
        nBytes = usb_control_msg(
            handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
            RECEIVE_DATA_FROM_PC, 0, 0, const_cast<char*>(args[2].c_str()),
            strlen(args[2].c_str()) + 1, 5000);
    }

    if (nBytes < 0)
        fprintf(stderr, "USB error: %s\n", usb_strerror());

    usb_close(handle);

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cout << "CPU time used: " << microseconds << " ms\n";

    return 0;
}
