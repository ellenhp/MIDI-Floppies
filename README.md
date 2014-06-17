MIDI-Floppies
=============

This project allows users to control floppy drives with a MIDI keyboard. It supports an arbitrary number of floppy drives and is easily customizable to meet the needs of different users. MIDI connected to UART 1 instead of UART 0 on Arduino Mega? Not a problem. Wiring your floppies in your own special way? Not a problem. Just change the appropriate lines of the hardware_defs.h file in either case. No need to reverse-engineer the code.

Currently the easiest way to use it with a computer is to send MIDI commands over the Arduino's built-in serial interface. Programs like Hairless MIDI<->Serial Bridge make this easy. To do this, change the baud rate in hardware_defs.h to something standard that the Arduino's FTDI chip can handle. I used 38400 because it's easy to remember and slightly faster than the default MIDI baud rate, then configure Hairless MIDI<->Serial Bridge according to the instructions for your platform.

Dependencies
============

This project carries one dependency, TimerOne. You can find this project here: https://code.google.com/p/arduino-timerone/

Configuration
=============

Make sure to copy the "hardware_defs_template.h" file into the floppies folder and rename it "hardware_defs.h". You should also open it and look at it a little to see if it works for your needs. It will almost certainly need to be changed slightly!

The advantage to having this header file is that you shouldn't have to change anything else. It should all just work. If it doesn't, a test function called testAll() that you can call inside setup() is already written for you. It will move the heads of all the floppies in one direction and then back, one floppy at a time. If this function doesn't work, then you likely have a configuration or wiring problem.
