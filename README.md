# morse-code-receiver

# Description

This project involves using the FRDM-KL05Z microcontroller to read, process, and decode Morse code signals. A light sensor, connected to the microcontroller pin, serves as the receiver, while a flashlight is used as the transmitter of light pulses. The program cyclically reads signals from the light sensor, averaging the results using the PIT counter, and then encodes the sequences of Morse code "dots" and "dashes". Based on these sequences, the program decodes alphabetic characters displaying them in the user interface.

# Hardware
- microconteoller FRDM-KL05Z
- Light sensor
- Flashlight
