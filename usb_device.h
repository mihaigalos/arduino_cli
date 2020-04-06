/**
 * Code adapted from AVR ATtiny USB Tutorial at http://codeandlife.com/, Joonas
 * Pihlajamaa, joonas.pihlajamaa@iki.fi
 */

#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

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

class USBDevice
{
public:
	static usb_dev_handle *open(int vendor, const std::string &vendorName, int product,
								const std::string &productName)
	{

		usb_dev_handle *handle = NULL;

		init();

		for (auto bus = usb_get_busses(); bus; bus = bus->next)
		{
			for (auto dev = bus->devices; dev; dev = dev->next)
			{

				auto vendor_product = getInfo(bus, dev, vendor, product, handle);
				auto devVendor = std::get<0>(vendor_product);
				auto devProduct = std::get<1>(vendor_product);

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
	static void init()
	{
		usb_init();
		usb_find_busses();
		usb_find_devices();
	}

	static bool canOpenDevice(struct usb_device *dev, usb_dev_handle *handle)
	{
		if (!(handle = usb_open(dev)))
		{
			std::cerr << "Warning: cannot open USB device: " << usb_strerror() << "\n";
			return false;
		}
		return true;
	}

	static bool hasVendor(struct usb_device *dev, char *devVendor, uint8_t vendor_size, usb_dev_handle *handle)
	{
		if (getDeviceIdDescriptor(handle, dev->descriptor.iManufacturer,
								  0x0409, devVendor, vendor_size) < 0)
		{
			std::cerr << "Warning: cannot query manufacturer for device: " << usb_strerror() << "\n";
			usb_close(handle);
			return false;
		}
		return true;
	}

	static bool hasProduct(struct usb_device *dev, char *devProduct, uint8_t product_size, usb_dev_handle *handle)
	{
		if (getDeviceIdDescriptor(handle, dev->descriptor.iProduct, 0x0409,
								  devProduct, product_size) < 0)
		{
			std::cerr << "Warning: cannot query product for device: " << usb_strerror() << "\n";
			usb_close(handle);
			return false;
		}
		return true;
	}

	static std::tuple<std::string, std::string> getInfo(struct usb_bus *bus, struct usb_device *dev, int vendor, int product, usb_dev_handle *handle)
	{

		char devVendor[255], devProduct[255];
		if (dev->descriptor.idVendor == vendor &&
			dev->descriptor.idProduct == product &&
			canOpenDevice(dev, handle) &&
			hasVendor(dev, devVendor, sizeof(devVendor), handle) &&
			hasProduct(dev, devProduct, sizeof(devProduct), handle))
		{
			return std::make_tuple<std::string, std::string>(devVendor, devProduct);
		}
		return std::make_tuple<std::string, std::string>("", "");
	}
	//TODO: refactor C to C++
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

#endif // __USB_DEVICE_H__