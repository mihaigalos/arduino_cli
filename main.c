/**
 * Code adapted from AVR ATtiny USB Tutorial at http://codeandlife.com/, Joonas
 * Pihlajamaa, joonas.pihlajamaa@iki.fi
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "usbdrv.h"

#define F_CPU 16000000L
#include <util/delay.h>

#include <stdint.h>

#define USB_LED_OFF 0
#define USB_LED_ON 1
#define SEND_DATA_TO_PC 2
#define USB_DATA_WRITE 3
#define RECEIVE_DATA_FROM_PC 4

#define columnCount 31
#define bytesPercolumn 8

static uchar replyBuf[columnCount * bytesPercolumn] = "Hello, USB!";
static uchar dataReceived = 0, dataLength = 0; // for RECEIVE_DATA_FROM_PC

static uint16_t offset = 0;

void fillBufferFromFlash()
{

	for (uint16_t i = 0; i < sizeof(replyBuf); ++i)
	{
		replyBuf[i] = pgm_read_byte_near(i + offset);
	}
	offset += sizeof(replyBuf);
}

// this gets called when custom control message is received
USB_PUBLIC uchar onReceiveCustomControlMessage(uchar data[8])
{
	usbRequest_t *rq = (void *)data;

	switch (rq->bRequest)
	{
	case USB_LED_ON:
		PORTD &= ~(1 << 0) & ~(1 << 1);
		return 0;
	case USB_LED_OFF:
		PORTD |= (1 << 0) | (1 << 1);
		return 0;
	case SEND_DATA_TO_PC: // send data to PC
		fillBufferFromFlash();
		usbMsgPtr = replyBuf;
		return sizeof(replyBuf);
	case USB_DATA_WRITE: // modify reply buffer
		replyBuf[7] = rq->wValue.bytes[0];
		replyBuf[8] = rq->wValue.bytes[1];
		replyBuf[9] = rq->wIndex.bytes[0];
		replyBuf[10] = rq->wIndex.bytes[1];
		return 0;
	case RECEIVE_DATA_FROM_PC: // receive data from PC
		dataLength = (uchar)rq->wLength.word;
		dataReceived = 0;

		if (dataLength > sizeof(replyBuf)) // limit to buffer size
			dataLength = sizeof(replyBuf);

		return USB_NO_MSG; // onDataFromPCtoDevice will be called now
	}

	return 0;
}

USB_PUBLIC uchar onDataFromPCtoDevice(uchar *data, uchar len)
{
	for (uint8_t i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
	{
		replyBuf[dataReceived] = data[i];
	}

	return (dataReceived == dataLength); // 1 if we received it all, 0 if not
}

void enumerateUSB()
{
	usbDeviceDisconnect();
	for (uint8_t i = 0; i < 250; i++)
	{
		wdt_reset();
		_delay_ms(2);
	}
	usbDeviceConnect();
}

void USB_INTR_VECTOR(void);
USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	return onReceiveCustomControlMessage(data);
}

int main()
{

	DDRD = (1 << 1 | 1 << 0);

	usbInit();
	enumerateUSB();
	sei();

	while (1)
	{
		if (USB_INTR_PENDING & (1 << USB_INTR_PENDING_BIT))
		{
			USB_INTR_VECTOR();
			USB_INTR_PENDING =
				1 << USB_INTR_PENDING_BIT; // Clear int pending, in case timeout
										   // occured during SYNC
		}

		wdt_reset(); // keep the watchdog happy
		usbPoll();
	}

	return 0;
}
