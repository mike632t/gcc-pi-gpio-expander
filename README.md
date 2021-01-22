## gcc-pi-gpio-expander

Written in C. 

Demonstrates how to write data to the GPIO pins of a MCP23017 I2C I/O port
expander.

Periodically reads the input from port A and when the input changes outputs
the new value to port B.  Because the hardware I used enabled an input by
pulling the line low the input polarity must be inverted for the correct
value to be returned. The advantage of doing it this way is that you can use
the internal pull up resistors to pull the inactive inputs high.

Note: Do NOT use this code to drive LEDs directly from the MCP23017 as the 
total output current could easily exceed the maximum current rating for the
device.  If you want to drive multiple LEDs at the same time you need to use
a display driver or a transistor to switch the current.

Compile with 'gcc -o gcc-pi-led-readwrite gcc-pi-led-readwrite.c'

