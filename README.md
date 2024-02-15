# Eater_6502 whisperer

This is a eeprom and ram emulator for the pi pico.

It is designed to be used with an fpga emulating the 6502 processor.

Inspired by the [eater 6502](https://eater.net/6502) project.

## Usage

Just connect the address and data lines to the pi pico and the fpga.

The pico uses an interrupt on the clock line to know when to read or write the data.