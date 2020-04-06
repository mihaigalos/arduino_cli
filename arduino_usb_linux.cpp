/**
 * Code adapted from AVR ATtiny USB Tutorial at http://codeandlife.com/, Joonas
 * Pihlajamaa, joonas.pihlajamaa@iki.fi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this is libusb, see http://libusb.sourceforge.net/
#include <usb.h>

#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>

#include "usb_device.h"

#define LED_OFF 0
#define LED_ON 1
#define SEND_DATA_TO_PC 2
#define MODIFY_REPLY_BUFFER 3
#define RECEIVE_DATA_FROM_PC 4

void showHelp()
{
	printf("Usage:\n");
	printf("arduino_usb_linux on\n");
	printf("arduino_usb_linux off\n");
	printf("arduino_usb_linux out\n");
	printf("arduino_usb_linux write\n");
	printf("arduino_usb_linux in <string>\n");
	exit(1);
}

int main(int argc, char **argv)
{
	usb_dev_handle *handle = NULL;
	int nBytes = 0;
	char buffer[255];

	if (argc < 2)
	{
		showHelp();
	}
	auto start = std::chrono::high_resolution_clock::now();
	handle = USBDevice::open(0x16C0, "", 0x05DC, "DotPhat");

	if (handle == NULL)
	{
		fprintf(stderr, "Could not find USB device!\n");
		exit(1);
	}

	if (strcmp(argv[1], "on") == 0)
	{
		nBytes = usb_control_msg(handle,
								 USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
								 LED_ON, 0, 0, (char *)buffer, sizeof(buffer), 5000);
	}
	else if (strcmp(argv[1], "off") == 0)
	{
		nBytes = usb_control_msg(handle,
								 USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
								 LED_OFF, 0, 0, (char *)buffer, sizeof(buffer), 5000);
	}
	else if (strcmp(argv[1], "out") == 0)
	{

		nBytes = usb_control_msg(handle,
								 USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
								 SEND_DATA_TO_PC, 0, 0, (char *)buffer, sizeof(buffer), 5000);
		std::cout << "Got " << nBytes << " bytes: " << std::endl;
		for (int i = 0; i < nBytes; ++i)
		{
			if (i > 0 && 0 == i % 8)
				std::cout << std::endl;
			auto value = static_cast<uint16_t>(static_cast<uint8_t>(buffer[i]));
			std::cout << std::hex << " ";
			if (value < 16)
				std::cout << "0";
			std::cout << value;
		}

		std::cout << std::dec << std::endl;
	}
	else if (strcmp(argv[1], "write") == 0)
	{
		nBytes = usb_control_msg(handle,
								 USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
								 MODIFY_REPLY_BUFFER, 'T' + ('E' << 8), 'S' + ('T' << 8),
								 (char *)buffer, sizeof(buffer), 5000);
	}
	else if (strcmp(argv[1], "in") == 0 && argc > 2)
	{
		nBytes = usb_control_msg(handle,
								 USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
								 RECEIVE_DATA_FROM_PC, 0, 0, argv[2], strlen(argv[2]) + 1, 5000);
	}

	if (nBytes < 0)
		fprintf(stderr, "USB error: %s\n", usb_strerror());

	usb_close(handle);

	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	long long microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	std::cout << "CPU time used: " << microseconds << " ms\n";

	return 0;
}
