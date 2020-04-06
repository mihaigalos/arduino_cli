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

#define LED_OFF 0
#define LED_ON 1
#define SEND_DATA_TO_PC 2
#define MODIFY_REPLY_BUFFER 3
#define RECEIVE_DATA_FROM_PC 4

class USBDevice
{
public:
	static usb_dev_handle *open(int vendor, const std::string &vendorName, int product,
								const std::string &productName)
	{
		char devVendor[256], devProduct[256];

		usb_dev_handle *handle = NULL;

		usb_init();
		usb_find_busses();
		usb_find_devices();

		for (auto bus = usb_get_busses(); bus; bus = bus->next)
		{
			for (auto dev = bus->devices; dev; dev = dev->next)
			{
				if (dev->descriptor.idVendor != vendor || dev->descriptor.idProduct != product)
					continue;

				// we need to open the device in order to query strings
				if (!(handle = usb_open(dev)))
				{
					std::cerr << "Warning: cannot open USB device: " << usb_strerror() << "\n";
					continue;
				}

				// get vendor name
				if (getDeviceIdDescriptor(handle, dev->descriptor.iManufacturer,
										  0x0409, devVendor, sizeof(devVendor)) < 0)
				{
					std::cerr << "Warning: cannot query manufacturer for device: " << usb_strerror() << "\n";
					usb_close(handle);
					continue;
				}

				// get product name
				if (getDeviceIdDescriptor(handle, dev->descriptor.iProduct, 0x0409,
										  devProduct, sizeof(devVendor)) < 0)
				{
					std::cerr << "Warning: cannot query product for device: " << usb_strerror() << "\n";
					usb_close(handle);
					continue;
				}

				std::cout << "Found vendor: " << devVendor << "\n";
				std::cout << "Found product: " << devProduct << "\n\n";

				if (devProduct == productName)
					return handle;
				else
				{
					usb_close(handle);
					std::cerr << "Cannot find usb device from : \nVendor: " << vendorName << "\nProduct: " << productName << "\n\n";
				}
			}
		}

		return NULL;
	}

private:
	// used to get descriptor strings for device identification
	static int getDeviceIdDescriptor(usb_dev_handle *dev, int index, int langid,
									 char *buf, int buflen)
	{
		char buffer[256];
		int rval, i;

		// make standard request GET_DESCRIPTOR, type string and given index
		// (e.g. dev->iProduct)
		rval = usb_control_msg(dev,
							   USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
							   USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid,
							   buffer, sizeof(buffer), 1000);

		if (rval < 0) // error
			return rval;

		// rval should be bytes read, but buffer[0] contains the actual response size
		if ((unsigned char)buffer[0] < rval)
			rval = (unsigned char)buffer[0]; // string is shorter than bytes read

		if (buffer[1] != USB_DT_STRING) // second byte is the data type
			return 0;					// invalid return type

		// we're dealing with UTF-16LE here so actual chars is half of rval,
		// and index 0 doesn't count
		rval /= 2;

		// lossy conversion to ISO Latin1
		for (i = 1; i < rval && i < buflen; i++)
		{
			if (buffer[2 * i + 1] == 0)
				buf[i - 1] = buffer[2 * i];
			else
				buf[i - 1] = '?'; // outside of ISO Latin1 range
		}
		buf[i - 1] = 0;

		return i - 1;
	}
};
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
