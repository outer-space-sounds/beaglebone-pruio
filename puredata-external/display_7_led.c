/* Beaglebone Pru IO 
 * 
 * Copyright (C) 2015 Rafael Vega <rvega@elsoftwarehamuerto.org> 
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
 */

#include <m_pd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>

/*
 * This object drives a 2 character, 7 led display using a MCP23017 
 * GPIO expander connected with the I2C-1 bus in a beagle bone black.
 * The GPIO expander has 16 i/o pins that are connected to the display
 * leds as follows:
 *
 *     10       15   
 *   -----    -----  
 *   |11 |12  |14 |13
 *   ----- 9  ----- 16
 *   |7  |8   |2  |1 
 *   -----    -----  
 *     6        3    
 *
 * If you isolate the left character (tens) in a single 8bit group, the 
 * led to bit mapping would look like:
 *
 *     4
 *   -----
 *   |5  |6
 *   ----- 3
 *   |1  |2
 *   -----
 *     0
 *
 * We fill the table below to map integers 0 to 9 to the leds as in this
 * diagram. After that, there's a bunch of bit wrangling in
 * display_7_led_number() to get the bits in the table and put them 
 * in the order needed for the actual connections (first diagram).
 *
 * The protocol for writing data to the GPIO expander throught i2C is 
 * dead simple: send the address of the register you want to write
 * followed by the one byte data, the address pointer is incremented
 * automatically and then send another byte to be written to address+1.
 */    

/////////////////////////////////////////////////////////////////////////
// Constants
//

#define SLAVE_ADDRESS 0x20
#define REGISTER_ADDRESS 0x12

 static const uint8_t display_7_led_table[10] = {
  0b01110111,  // 0
  0b01000100,  // 1
  0b01011011,
  0b01011101,
  0b01101100,
  0b00111101,
  0b00111111,
  0b01010100,
  0b01111111,
  0b01111101   // 9
};

/////////////////////////////////////////////////////////////////////////
// Data
//

static int display_7_led_i2c_file_descriptor = 0;

typedef struct display_7_led {
   t_object x_obj;
   uint16_t current_state;
   float is_on;
   float current_number;
} t_display_7_led;

// A pointer to the class object.
t_class *display_7_led_class;


/////////////////////////////////////////////////////////////////////////
// Utility
//
static int display_7_led_load_device_tree_overlay(char* dto){
   // Check if the device tree overlay is loaded, load if needed.
   int device_tree_overlay_loaded = 0; 
   FILE* f;
   f = fopen("/sys/devices/platform/bone_capemgr/slots","rt");
   if(f==NULL){
      return 1;
   }
   char line[256];
   while(fgets(line, 256, f) != NULL){
      if(strstr(line, dto) != NULL){
         device_tree_overlay_loaded = 1; 
      }
   }
   fclose(f);

   if(!device_tree_overlay_loaded){
      f = fopen("/sys/devices/platform/bone_capemgr/slots","w");
      if(f==NULL){
         return 1;
      }
      fprintf(f, "%s", dto);
      fclose(f);
   }

   return 0;
}

/////////////////////////////////////////////////////////////////////////
// On Message received
//

void display_7_led_on(t_display_7_led* x, t_floatarg f){
   if(f == x->is_on) return;

   if(f!=0 && f!=1){
      error("beaglebone/display_7_led: %f is not a correct value, only 0 and 1 allowed.", f);
      return;
   }

   x->is_on = f;

   #ifdef IS_BEAGLEBONE
     if(f==0){
       // Turn all leds off, write 0s to device register
       uint8_t buf[3] = {REGISTER_ADDRESS, 0x0, 0x0};
       write(display_7_led_i2c_file_descriptor, buf, 3);
     }
     else{
       // Display stored value
       uint16_t msg = x->current_state;
       uint8_t msb = msg >> 8;
       uint8_t lsb = msg & 0xFF;
       uint8_t buf[3] = {REGISTER_ADDRESS, msb, lsb};
       write(display_7_led_i2c_file_descriptor, buf, 3);
     }
   #else
      (void)x;
   #endif 
}

/////////////////////////////////////////////////////////////////////////
// Number Message received
//

void display_7_led_number(t_display_7_led* x, t_floatarg f){
   if(x->current_number == f) return;

   if(f<0 || f>99){
      error("beaglebone/display_7_led: %f is not a correct value, only integers between 0 and 99 are allowed.", f);
      return;
   }
   x->current_number = f;
   unsigned int n = f;

   unsigned int tens = n / 10;
   unsigned int units = n % 10;

   uint8_t t = display_7_led_table[tens];
   uint8_t u = display_7_led_table[units];
   uint8_t un = (u & 0b1) << 2;
   un |= ((u & 0b10));
   un |= ((u & 0b100) >> 2);
   un |= ((u & 0b1000) << 3);
   un |= ((u & 0b10000) << 1);
   un |= ((u & 0b100000) >> 1);
   un |= ((u & 0b1000000) >> 3);

   uint16_t msg = (t << 5);
   msg = msg | (un & 0b0111) | ((un & 0b01111000)<<9);
   x->current_state = msg;

   uint8_t msb = msg >> 8;
   uint8_t lsb = msg & 0xFF;

   #ifdef IS_BEAGLEBONE
     uint8_t buf[3] = {REGISTER_ADDRESS, msb, lsb};
     write(display_7_led_i2c_file_descriptor, buf, 3);
   #endif 
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *display_7_led_new(){
  #ifdef IS_BEAGLEBONE
    if(display_7_led_i2c_file_descriptor == 0){
      // Load device tree overlay to turn i2c-1 hardware on and configure
      // P9_17 and p9_18 pins.
      if(display_7_led_load_device_tree_overlay("BB-I2C1")){
        error("beaglebone/display_7_led: Could not load device tree overlay for i2c-1 device.");
        return NULL;
      }
      usleep(100000);

      // Open the file that represents the i2c device
      char *filename = "/dev/i2c-1";
      if((display_7_led_i2c_file_descriptor = open(filename, O_RDWR)) < 0){
        error("beaglebone/display_7_led: Failed to open the i2c bus: %s\n", strerror(errno));
        return NULL;
      }

      // Set slave device address.
      if(ioctl(display_7_led_i2c_file_descriptor, I2C_SLAVE, SLAVE_ADDRESS) < 0){
        error("beaglebone/display_7_led: Failed to acquire bus access and/or talk to slave: %s\n", strerror(errno));
        return NULL;
      }
      
      // Start talking to slave device. Set IO ports to output. 
      // Write two bytes to address 0x00 and 0x01 (pointer auto increments)
      uint8_t msg[3] = {0x00, 0x00, 0x00};
      write(display_7_led_i2c_file_descriptor, msg, 3);
    }
  #endif

  t_display_7_led *x = (t_display_7_led *)pd_new(display_7_led_class);
  x->current_number = 1000;
  x->is_on = 0;
  display_7_led_number(x, 0);
  display_7_led_on(x, 1);

  return x;
}

static void display_7_led_free(t_display_7_led *x) { 
  close(display_7_led_i2c_file_descriptor);
  display_7_led_i2c_file_descriptor = 0;
  (void)x;
}

/////////////////////////////////////////////////////////////////////////
// Class definition
// 

void display_7_led_setup(void) {
   display_7_led_class = class_new(
      gensym("display_7_led"), 
      (t_newmethod)display_7_led_new, 
      (t_method)display_7_led_free, 
      sizeof(t_display_7_led), 
      CLASS_DEFAULT, 
      (t_atomtype)0
   );

   // Trigger display_7_led_on() when a "on" message is received
   class_addmethod(display_7_led_class, (t_method)display_7_led_on, gensym("on"), A_DEFFLOAT, 0);
   
   // Trigger display_7_led_number() when a "number" message is received
   class_addmethod(display_7_led_class, (t_method)display_7_led_number, gensym("number"), A_DEFFLOAT, 0);
}
