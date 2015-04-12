/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/* Servo Controller application by Jim MacArthur 
   Based on the VirtualSerial demo application from LUFA. */

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "ServoController.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
	int mode = 0;

	for (;;)
	{
		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
		int c = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		if(c >= 0) {
		  if(mode > 0) {
		    if(mode==1) { OCR1A = 2000 + (c * 8); }
		    else if(mode==2) { OCR1B = 2000 + (c * 8); }
		    else if(mode==3) { OCR1C = 2000 + (c * 8); }
		    mode = 0;
		  }
		  else if(c >= 'a' && c <= 'z')
		    CDC_Device_SendByte(&VirtualSerial_CDC_Interface, (uint8_t) 97 + ( (c-97+3) % 26 ) );
		  else if(c >= '0' && c <= '9') {
		    int pos = c - '0';
		    OCR1A = 2000 + pos * 200;
		  }
		  else if(c == 'X') {
		    mode = 1;
		  }
		  else if(c == 'Y') {
		    mode = 2;
		  }
		  else if(c == 'Z') {
		    mode = 3;
		  }
		}
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();

	// configure TIMER1 interrupt
	DDRC |= (1 << DDC6) | (1 << DDC5); // PC6,5 are outputs
	DDRB |= (1 << DDB7); // PB7 is output

	TCCR1A = 0;        // set entire TCCR1A register to 0
	TCCR1B = 0;

	TCCR1A |= (1 << COM1A1) | (1<<COM1B1) | (1<<COM1C1); // Clear OC1* on match, set at TOP
	TCCR1B |= (1 << CS11); // Timer1 clock = clk/8.

	// Now timer1 runs at 2MHz. We want to reset every 20ms, so we
	// set ICR1 to 40000.
	ICR1 = 40000;

	// Fast PWM on timer1, TOP=ICR1, PWM mode
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);

	OCR1A = 3000;  // Roughly 1.5ms. 2000=1ms. 4000=2ms.
	OCR1B = 3000;  // Roughly 1.5ms. 2000=1ms. 4000=2ms.
	OCR1C = 3000;  // Roughly 1.5ms. 2000=1ms. 4000=2ms.
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

