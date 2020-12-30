
/*
 * G P I O   U S I N G   A N   I / O   E X P A N D E R
 *
 * Demonstrates how to read and write data to and from the GPIO pins of a 
 * MCP23017 I2C I/O port expander.  
 *
 * Periodically reads the input from port A and when the input changes outputs
 * the new value to port B.  Because the hardware I used enabled an input by
 * pulling the line low the input polarity must be inverted for the correct
 * value to be returned. The advantage of doing it this way is that you can use
 * the internal pull up resistors to pull the inactive inputs high.  
 *
 * Note: Do NOT use this code to drive LEDs directly from the MCP23017 as the 
 * total output current could easily exceed the maximum current rating for the
 * device.  If you want to drive multiple LEDs at the same time you need to use
 * a display driver or a transistor to switch the current.
 *
 * Compile with 'gcc -o gcc-pi-led-readwrite gcc-pi-led-readwrite.c'  
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 30 Jul 12   0.1   - Initial version - MEJT  
 * 31 Jul 12   0.2   - Swapped over the ports and used port A for the input and 
 *                     port B for the output as this made it was easier to 
 *                     layout the components on the breadboard - MEJT
 *                   - Adopted the same register naming convention as used on
 *                     the product datasheet - MEJT
 *                   - Added a counter to terminate the loop after a specified
 *                     number of changes - MEJT
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#define DEVICE    "/dev/i2c-0"
#define ADDRESS   0x20

#define IODIRA    0x00
#define IODIRB    0x01
#define IPOLA     0x02
#define IPOLB     0x03
#define GPINTENA  0x04
#define GPINTENB  0x05
#define DEFVALA   0x06
#define DEFVALB   0x07
#define INTCONA   0x08
#define INTCONB   0x09
#define IOCON     0x0A
#define GPPUA     0x0C
#define GPPUB     0x0D
#define INTFA     0x0E
#define INTFB     0x0F
#define INTCAPA   0x10
#define INTCAPB   0x11
#define GPIOA     0x12
#define GPIOB     0x13
#define OLATA     0x14
#define OLATB     0x15

#define DELAY     200000
#define LIMIT     10

void dumpbin(unsigned char byte) {

/*
 * Shift each bit and use a logical AND to mask the value, outputing the byte 
 * as two seperate nibbles.
 */
   int count;
   for(count = 7; count >= 4; count--) putchar('0' + ((byte >> count) & 1));
   putchar(' ');
   for(count = 3; count >= 0; count--) putchar('0' + ((byte >> count) & 1));
   putchar(' ');
}

int main(int argc, char **argv) {
   int count = 0; // Counter.
   int loop = 0; // Counter.
   int bytes = 0; // Number of bytes in buffer.
   unsigned char data; // Data.

   int fd;  // File descrition.
   unsigned char buf[4];  // Buffer for data being read or written to the device.

/*
 * Open device for reading and writing - return an error on failure.
 */
   if ((fd = open(DEVICE, O_RDWR)) < 0) {
      printf("Failed to open device\n");
      exit(1);
   }
   if (ioctl(fd, I2C_SLAVE, ADDRESS) < 0) {
      printf("Unable to access device\n");
      exit(1);
   }

/*
 * Configure every GPIO pin on port A as inputs and port B as outputs.
 */
   bytes = 4;
   buf[0] = IODIRA;  // Adddress of port A data direction register.
   buf[1] = 0xFF;  // Configure all the GPIO pins on port A as inputs.
   buf[2] = 0x00;  // Configure all the GPIO pins on port B as outputs..
   buf[3] = 0xFF;  // Invert all the inputs on port A.
   if ((write(fd, buf, bytes)) != bytes) {
      printf("Error writing data\n");
      exit(1);
   }

/*
 * Enable the internal pull-up resistors on every GPIO pin on port A.
 */
   bytes = 2;
   buf[0] = GPPUA;  // Adddress of port A pull up register.
   buf[1] = 0xFF;  // Enable all the internal pull-up resistors on port A.
   if ((write(fd, buf, bytes)) != bytes) {
      printf("Error writing data\n");
      exit(1);
   }

/*
 * Loop reading the input from port A and if it has changed output it to port B.
 */
   count =0;
   while (count < LIMIT) {

   /*
    * Read data from port A.
    */
      bytes = 1;
      buf[0] = GPIOA;  // Adddress of port A data register.
      if ((write(fd, buf, bytes)) != bytes) {  // Send the register address. 
         printf("Error writing data\n");
         exit(1);
      }
      if ((read(fd, buf, bytes)) != bytes) {  // Read input data.
         printf("Error reading data\n");
         exit(1);
      }
      
   /*
    * If the input has changed (or this is the first time around the loop) then
    * output the input data and display it on the console.
    */
      if ((data != buf[0]) || (count < 1)) {
         data = buf[0];
         bytes = 2;
         buf[0] = GPIOB;  // Adddress of port B data register.
         buf[1] = data ;  // Put data into buffer.
         if ((write(fd, buf, bytes)) != bytes) {  // Write data to port A.
            printf("Error writing data\n");
            exit(1);
         }
         dumpbin (data);
         printf("%4.2u\n", count);
         usleep(DELAY);  // Slow things down a bit
         count++;
      }
   }

/*
 * Reset all the GPIO pins to be inputs on both port A and B and clise the file.
 */
   bytes = 4;
   buf[0] = IODIRA;  // Adddress of port A data direction register.
   buf[1] = 0xFF;  // Set port A data direction register to all input.
   buf[2] = 0xFF;  // Set port B data direction register to all input.
   buf[3] = 0x00;  // Reset input polarity on port A.
   if ((write(fd, buf, bytes)) != bytes) {
      printf("Error writing data\n");
      exit(1);
   }
   close (fd);

   return 0;
}

