How to build the servo controller:

1) Install pre-requisites: aptitude install gcc-avr avr-libc dfu-programmer
2) Download and extract the LUFA zip somewhere.
3) Install and configure dfu-programmer. You should be able to run 'dfu-programmer atmega32u2 erase' successfully.
4) Alter 'makefile' and set LUFA_PATH to the LUFA directory inside the directory structure you just unpacked.
5) If necessary, change MCU to at90usb162. (I haven't tested it on this chip.)
6) run 'make hex'.
7) If this succeeded, you should have a "ServoController.hex" file.
8) Put the minimus into bootloader mode (press 'reset' while holding down HWB1).
9) run 'make install'

You should now have a device called /dev/ttyACM0. Run 'screen /dev/ttyACM0 115200' (you may need sudo), or run your favoure terminal emulator.

Typing the numbers 0-9 on the serial console should roughly set the servo position. To set the position with 8-bit precision, send character 'X' followed by another byte which will control the position: 00 = full left, FF = full right, 7F = centre.

The servo control line should appear on pin PC6.