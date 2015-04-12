AVR Minimus servo controller
============================

![Servo control](/minimus-servo.jpeg)

## How to build the servo controller:

1. Install pre-requisites: aptitude install gcc-avr avr-libc dfu-programmer
2. Download and extract the LUFA zip somewhere.
3. Install and configure dfu-programmer. You should be able to run 'dfu-programmer atmega32u2 erase' successfully.
4. Alter 'makefile' and set LUFA_PATH to the LUFA directory inside the directory structure you just unpacked.
5. If necessary, change MCU to at90usb162. (I haven't tested it on this chip.)
6. Run 'make hex'.
7. If this succeeded, you should have a "ServoController.hex" file.
8. Put the minimus into bootloader mode (press 'reset' while holding down HWB1).
9. run 'make install'

## How to use the controller

You should now have a device called /dev/ttyACM0. Run 'screen /dev/ttyACM0 115200' (you may need sudo), or run your favoure terminal emulator.

Typing the numbers 0-9 on the serial console should roughly set the servo position. This is only meant for testing and will only affect pin PC6.

To set the position with 8-bit precision, send character 'X' followed by another byte which will control the position: 00 = full left, FF = full right, 7F = centre. You can set the other channels by sending either 'Y' or 'Z' followed by a byte.

The channel pin assignments are:

* X: PC6
* Y: PC5
* Z: PB7

These should be connected to the control line of your servo, which is usually the white wire. Note that I do not recommend powering servos directly from the Vdd line of the Minimus (even though I did in the picture above.) I haven't measured the current draw of a servo, but I suspect it could be well above the 100mA USB specification.

Sending lower-case letters returns the Caesar-ciphered letter. This is only there to test the serial communication channel.