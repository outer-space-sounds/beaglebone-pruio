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

#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "beaglebone_pruio.h"
#include "registers.h"
#include "beaglebone_pruio_pins.h"

#ifndef BEAGLEBONE_PRUIO_START_ADDR_0
   #error "BEAGLEBONE_PRUIO_START_ADDR_0 must be defined."
#endif

#ifndef BEAGLEBONE_PRUIO_PREFIX
   #error "BEAGLEBONE_PRUIO_PREFIX must be defined."
#endif

/* #define DEBUG */


/////////////////////////////////////////////////////////////////////
// MEMORY MAP

volatile unsigned int* gpio0_output_enable = NULL;
volatile unsigned int* gpio1_output_enable = NULL;
volatile unsigned int* gpio2_output_enable = NULL;
volatile unsigned int* gpio3_output_enable = NULL;

volatile unsigned int* gpio0_data_out = NULL;
volatile unsigned int* gpio1_data_out = NULL;
volatile unsigned int* gpio2_data_out = NULL;
volatile unsigned int* gpio3_data_out = NULL;

static int map_device_registers(){
   // Get pointers to hardware registers. See memory map in manual for addresses.
   
   int memdev = open("/dev/mem", O_RDWR | O_SYNC);
   
   // Get pointer to gpio0 registers (start at address 0x44e07000, length 0x1000 (4KB)).
   volatile void* gpio0 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO0);
   if(gpio0 == MAP_FAILED){ return 1; }
   gpio0_output_enable = (volatile unsigned int*)(gpio0 + GPIO_OE);
   gpio0_data_out = (volatile unsigned int*)(gpio0 + GPIO_DATAOUT);

   // same for gpio1, 2 and 3.
   volatile void* gpio1 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO1);
   if(gpio1 == MAP_FAILED){
      return 1;
   }
   gpio1_output_enable = (volatile unsigned int*)(gpio1 + GPIO_OE);
   gpio1_data_out = (volatile unsigned int*)(gpio1 + GPIO_DATAOUT);

   volatile void* gpio2 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO2);
   if(gpio2 == MAP_FAILED){
      return 1;
   }
   gpio2_output_enable = (volatile unsigned int*)(gpio2 + GPIO_OE);
   gpio2_data_out = (volatile unsigned int*)(gpio2 + GPIO_DATAOUT);

   volatile void* gpio3 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO3);
   if(gpio3 == MAP_FAILED){
      return 1;
   }
   gpio3_output_enable = (volatile unsigned int*)(gpio3 + GPIO_OE);
   gpio3_data_out = (volatile unsigned int*)(gpio3 + GPIO_DATAOUT);

   return 0;
}

/////////////////////////////////////////////////////////////////////
// GPIO PINS

typedef struct gpio_pin{ 
   beaglebone_pruio_gpio_mode mode;
   int gpio_number;
} gpio_pin;
static gpio_pin used_pins[BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS];
static int used_pins_count = 0;

static int init_gpio(){
   // Only pinmux is set here, enabling GPIO modules, 
   // clocks, debounce, etc. is set on the PRU side.
   
   // Pins used to control analog mux:
   int e1 = beaglebone_pruio_init_gpio_pin(P9_27, BEAGLEBONE_PRUIO_OUTPUT_MODE);
   int e2 = beaglebone_pruio_init_gpio_pin(P9_30, BEAGLEBONE_PRUIO_OUTPUT_MODE);
   int e3 = beaglebone_pruio_init_gpio_pin(P9_42A, BEAGLEBONE_PRUIO_OUTPUT_MODE);

   if(e1 || e2 || e3){
      return 1;
   } 
   else{
      return 0;
   }
}

static int get_gpio_pin_name(int gpio_number, char* pin_name){
   switch(gpio_number){
      case P8_07: 
         strcpy(pin_name, "P8_07");
         break;
      case P8_08: 
         strcpy(pin_name, "P8_08");
         break;
      case P8_09: 
         strcpy(pin_name, "P8_09");
         break;
      case P8_10: 
         strcpy(pin_name, "P8_10");
         break;
      case P8_11: 
         strcpy(pin_name, "P8_11");
         break;
      case P8_12: 
         strcpy(pin_name, "P8_12");
         break;
      case P8_13: 
         strcpy(pin_name, "P8_13");
         break;
      case P8_14: 
         strcpy(pin_name, "P8_14");
         break;
      case P8_15: 
         strcpy(pin_name, "P8_15");
         break;
      case P8_16: 
         strcpy(pin_name, "P8_16");
         break;
      case P8_17: 
         strcpy(pin_name, "P8_17");
         break;
      case P8_18: 
         strcpy(pin_name, "P8_18");
         break;
      case P8_19: 
         strcpy(pin_name, "P8_19");
         break;
      case P8_26: 
         strcpy(pin_name, "P8_26");
         break;
      case P8_27: 
         strcpy(pin_name, "P8_27");
         break;
      case P8_28: 
         strcpy(pin_name, "P8_28");
         break;
      case P8_29: 
         strcpy(pin_name, "P8_29");
         break;
      case P8_30: 
         strcpy(pin_name, "P8_30");
         break;
      case P8_31: 
         strcpy(pin_name, "P8_31");
         break;
      case P8_32: 
         strcpy(pin_name, "P8_32");
         break;
      case P8_33: 
         strcpy(pin_name, "P8_33");
         break;
      case P8_34: 
         strcpy(pin_name, "P8_34");
         break;
      case P8_35: 
         strcpy(pin_name, "P8_35");
         break;
      case P8_36: 
         strcpy(pin_name, "P8_36");
         break;
      case P8_37: 
         strcpy(pin_name, "P8_37");
         break;
      case P8_38: 
         strcpy(pin_name, "P8_38");
         break;
      case P8_39: 
         strcpy(pin_name, "P8_39");
         break;
      case P8_40: 
         strcpy(pin_name, "P8_40");
         break;
      case P8_41: 
         strcpy(pin_name, "P8_41");
         break;
      case P8_42: 
         strcpy(pin_name, "P8_42");
         break;
      case P8_43: 
         strcpy(pin_name, "P8_43");
         break;
      case P8_44: 
         strcpy(pin_name, "P8_44");
         break;
      case P8_45: 
         strcpy(pin_name, "P8_45");
         break;
      case P8_46:
         strcpy(pin_name, "P8_46");
         break;
      case P9_11: 
         strcpy(pin_name, "P9_11");
         break;
      case P9_12: 
         strcpy(pin_name, "P9_12");
         break;
      case P9_13: 
         strcpy(pin_name, "P9_13");
         break;
      case P9_14: 
         strcpy(pin_name, "P9_14");
         break;
      case P9_15: 
         strcpy(pin_name, "P9_15");
         break;
      case P9_16: 
         strcpy(pin_name, "P9_16");
         break;
      case P9_17: 
         strcpy(pin_name, "P9_17");
         break;
      case P9_18: 
         strcpy(pin_name, "P9_18");
         break;
      case P9_21: 
         strcpy(pin_name, "P9_21");
         break;
      case P9_22: 
         strcpy(pin_name, "P9_22");
         break;
      case P9_23: 
         strcpy(pin_name, "P9_23");
         break;
      case P9_24: 
         strcpy(pin_name, "P9_24");
         break;
      case P9_26: 
         strcpy(pin_name, "P9_26");
         break;
      case P9_27: 
         strcpy(pin_name, "P9_27");
         break;
      case P9_30: 
         strcpy(pin_name, "P9_30");
         break;
      case P9_41A: 
         strcpy(pin_name, "P9_41A");
         break;
      case P9_42A: 
         strcpy(pin_name, "P9_42A");
         break;
      default: 
         return 1;
         break;
   }
   return 0;
}

static int get_gpio_config_file(int gpio_number, char* path){
   char pin_name[256] = "";
   if(get_gpio_pin_name(gpio_number, pin_name)){
      return 1; 
   }

   char tmp[256] = "";

   // look for a path that looks like ocp.* in /sys/devices/
   DIR *dir = opendir("/sys/devices/");
   if(dir==NULL){
      return 1;
   }
   struct dirent* dir_info;
   while(dir){
      dir_info = readdir(dir);
      if(dir_info==NULL){
         closedir(dir);
         return 1;
      }
      // Substring "ocp."
      if(strstr(dir_info->d_name, "ocp.")!=NULL){
         strcat(tmp, "/sys/devices/");
         strcat(tmp, dir_info->d_name);
         strcat(tmp, "/");

         DIR *dir2 = opendir(tmp);
         if(dir2==NULL){
            printf("DD");
            return 1;
         }
         while(dir2){
            dir_info = readdir(dir2);
            if(dir_info==NULL){
               closedir(dir2);
               closedir(dir);
               printf("EE");
               return 1;
            }
            // Substring pin name
            if(strstr(dir_info->d_name, pin_name)!=NULL){
               strcat(tmp, dir_info->d_name);
               strcat(tmp, "/state");
               break; // while dir2
            }
         }
         closedir(dir2);
         break; // while dir1
      }
   }
   closedir(dir);
   strcpy(path, tmp);
   return 0;
}

/////////////////////////////////////////////////////////////////////
// ADC CHANNELS

typedef struct adc_channel{ 
   int channel_number;
} adc_channel;

static adc_channel used_adc_channels[BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS];
static int used_adc_channels_count = 0;


/////////////////////////////////////////////////////////////////////
// PRU Initialization
//

static int load_device_tree_overlay(char* dto){
   // Check if the device tree overlay is loaded, load if needed.
   int device_tree_overlay_loaded = 0; 
   FILE* f;
   f = fopen("/sys/devices/bone_capemgr.9/slots","rt");
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
      f = fopen("/sys/devices/bone_capemgr.9/slots","w");
      if(f==NULL){
         return 1;
      }
      fprintf(f, "%s", dto);
      fclose(f);
   }

   return 0;
}

static int load_device_tree_overlays(){
   if(load_device_tree_overlay("PRUIO-DTO")){
      return 1;
   }
   usleep(100000);
   return 0;
}

static int init_pru_system(){
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   if(prussdrv_init()) return 1;
   if(prussdrv_open(PRU_EVTOUT_0)) return 1;
   if(prussdrv_pruintc_init(&pruss_intc_initdata)) return 1;

   // Get pointer to shared ram
   void* p;
   if(prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &p)) return 1;
   beaglebone_pruio_shared_ram = (volatile unsigned int*)p;

   return 0;
}

static int start_pru0_program(){
   char path[512] = "";
   strcat(path, BEAGLEBONE_PRUIO_PREFIX); 
   strcat(path, "/lib/libbeaglebone_pruio_data0.bin");
   if(prussdrv_load_datafile(0, path)) return 1;

   strcpy(path, BEAGLEBONE_PRUIO_PREFIX); 
   strcat(path, "/lib/libbeaglebone_pruio_text0.bin");
   if(prussdrv_exec_program_at(0, path, BEAGLEBONE_PRUIO_START_ADDR_0)) return 1;

   return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Ring buffer (see header file for more)
//

static void buffer_init(){
   // These shared ram positions control which adc and gpio channels
   // we want to receive data from, see other comments in this file.
   beaglebone_pruio_shared_ram[1026] = 0;
   beaglebone_pruio_shared_ram[1027] = 0;
   beaglebone_pruio_shared_ram[1028] = 0;
   beaglebone_pruio_shared_ram[1029] = 0;
   beaglebone_pruio_shared_ram[1030] = 0;

   beaglebone_pruio_buffer_size = 1024;
   beaglebone_pruio_buffer_start = &(beaglebone_pruio_shared_ram[1024]); // value inited to 0 in pru
   beaglebone_pruio_buffer_end = &(beaglebone_pruio_shared_ram[1025]); // value inited to 0 in pru
}

/////////////////////////////////////////////////////////////////////
// "Public" functions.
//

int beaglebone_pruio_start(){
   if(load_device_tree_overlays()){
      fprintf(stderr, "libbeaglebone_pruio: Could not load device tree overlays.\n");
      return 1;
   }

   if(map_device_registers()){
      fprintf(stderr, "libbeaglebone_pruio: Could not map device's registers to memory.\n");
      return 1;
   }

   if(init_pru_system()){
      fprintf(stderr, "libbeaglebone_pruio: Could not init PRU system.\n");
      return 1;
   }

   buffer_init();

   if(start_pru0_program()){
      fprintf(stderr, "libbeaglebone_pruio: Could not load PRU0 program.\n");
      return 1;
   }

   if(init_gpio()){
      fprintf(stderr, "libbeaglebone_pruio: Could not init GPIO.\n");
      return 1;
   }


   return 0;
}

int beaglebone_pruio_init_adc_pin(int channel_number){
   // Check if channel already in use.
   int i;
   for(i=0; i<used_adc_channels_count; ++i){
      adc_channel channel = used_adc_channels[i];
      if(channel.channel_number==channel_number){
         return 0;
      }
   }

   // Save new channel info;
   adc_channel new_channel;
   new_channel.channel_number = channel_number;
   used_adc_channels[used_adc_channels_count] = new_channel;
   used_adc_channels_count++;

   /** 
    * Tell the PRU unit that we are interested in input from this channel.
    * If a bit is set in shared_ram[1030], it means that we need input 
    * for that ADC channel.
    *
    * shared_ram[1030]
    *
    */
   beaglebone_pruio_shared_ram[1030] |= (1<<channel_number);
   return 0;
}

int beaglebone_pruio_init_gpio_pin(int gpio_number, beaglebone_pruio_gpio_mode mode){
   // Check if pin already in use.
   int i;
   for(i=0; i<used_pins_count; ++i){
      gpio_pin pin = used_pins[i];
      if(pin.gpio_number==gpio_number && pin.mode==mode){
         return 0;
      }
      else if(pin.gpio_number==gpio_number && pin.mode!=mode){
         return 1;
      }
   }

   // Save new pin info;
   gpio_pin new_pin;
   new_pin.gpio_number = gpio_number;
   new_pin.mode = mode;
   used_pins[used_pins_count] = new_pin;
   used_pins_count++;
   
   // Set the pinmux of the pin by writing to the appropriate config file
   char path[256] = "";
   if(get_gpio_config_file(gpio_number, path)){
      return 1;
   }
   FILE *f = fopen(path, "w");
   if(f==NULL){
      return 1;
   }
   int gpio_module = gpio_number >> 5;
   int gpio_bit = gpio_number % 32;
   if(mode == BEAGLEBONE_PRUIO_OUTPUT_MODE){
      fprintf(f, "%s", "output"); 
      // Clear the output enable bit in the gpio config register to actually enable output.
      switch(gpio_module){
         case 0: *gpio0_output_enable &= ~(1<<gpio_bit); break;
         case 1: *gpio1_output_enable &= ~(1<<gpio_bit); break;
         case 2: *gpio2_output_enable &= ~(1<<gpio_bit); break;
         case 3: *gpio3_output_enable &= ~(1<<gpio_bit); break;
      }
   }
   else{
      fprintf(f, "%s", "input"); 
      // Set the output enable bit in the gpio config register to disable output.
      switch(gpio_module){
         case 0: *gpio0_output_enable |= (1<<gpio_bit); break;
         case 1: *gpio1_output_enable |= (1<<gpio_bit); break;
         case 2: *gpio2_output_enable |= (1<<gpio_bit); break;
         case 3: *gpio3_output_enable |= (1<<gpio_bit); break;
      }

      /** 
       * Tell the PRU unit that we are interested in input from this pin.
       *
       * Each one of the 32 bits in shared_ram[1026] represent the 32 pins for
       * the GPIO0 module. If a bit is set, it means that we need
       * input for that GPIO channel. Same mechanism is used for GPIO1, 2 and 3
       * using the following shared ram positions:
       *
       * shared_ram[1026] -> GPIO0
       * shared_ram[1027] -> GPIO1
       * shared_ram[1028] -> GPIO2
       * shared_ram[1029] -> GPIO3
       *
       */
      beaglebone_pruio_shared_ram[gpio_module+1026] |= (1<<gpio_bit);
 
   }
   fclose(f);
   return 0;
}

void beaglebone_pruio_set_pin_value(int gpio_number, int value){
   int gpio_module = gpio_number >> 5;
   int gpio_bit = gpio_number % 32;
   volatile unsigned int* reg=NULL;
   switch(gpio_module){
      case 0: reg = gpio0_data_out; break;
      case 1: reg = gpio1_data_out; break;
      case 2: reg = gpio2_data_out; break;
      case 3: reg = gpio3_data_out; break;
   }

   if(value==1){
      *reg |= (1<<gpio_bit);
   }
   else{
      *reg &= ~(1<<gpio_bit);
   }
}

int beaglebone_pruio_stop(){
   // TODO: send terminate message to PRU

   prussdrv_pru_disable(0);
   prussdrv_exit();

   return 0;
}

