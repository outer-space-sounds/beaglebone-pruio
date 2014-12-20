#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "bbb_pruio.h"
#include "registers.h"
#include "bbb_pruio_pins.h"

#ifndef BBB_PRUIO_START_ADDR_0
   #error "BBB_PRUIO_START_ADDR_0 must be defined."
#endif

#ifndef BBB_PRUIO_PREFIX
   #error "BBB_PRUIO_PREFIX must be defined."
#endif

/* #define DEBUG */


/////////////////////////////////////////////////////////////////////
// MEMORY MAP

// These functions and macros allow for accessing hardware registers

/* #define MAP(pointer, address, size) pointer = (volatile char *)mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, address); if(pointer==MAP_FAILED){return 1;} */

#define HWREG(pointer, offset) (*((volatile unsigned int *)(pointer+offset)))

/* int emdev; */
static void map_hardware_memory(){
   // TODO: close
   /* memdev = open("/dev/mem", O_RDWR | O_SYNC); */
}

static void close_memory_map(){
   /* close(memdev); */
}

/////////////////////////////////////////////////////////////////////
// GPIO PINS

typedef struct gpio_pin{ 
   bbb_pruio_gpio_mode mode;
   int gpio_number;
} gpio_pin;

static gpio_pin used_pins[MAX_GPIO_CHANNELS];
static int used_pins_count = 0;

static volatile char* gpio0 = NULL;
static volatile char* gpio1 = NULL;
static volatile char* gpio2 = NULL;
static volatile char* gpio3 = NULL;
/* static volatile char* cm_wkup = NULL; */

static int init_gpio(){
   // Get pointers to GPIO registers
   
   int memdev0 = open("/dev/mem", O_RDWR|O_SYNC);
   gpio0 = (volatile char *)mmap(0, 0xFFF, PROT_READ|PROT_WRITE, MAP_SHARED, memdev0, GPIO0); 
   printf("%i %p\n", memdev0, gpio0);
   if(gpio0==MAP_FAILED){ return 1; }

   gpio1 = (volatile char *)mmap(0, 0xFFF, PROT_READ|PROT_WRITE, MAP_SHARED, memdev0, GPIO1); 
   printf("%i %p\n", memdev0, gpio1);
   if(gpio1==MAP_FAILED){return 1;}

   /* int memdev2 = open("/dev/mem", O_RDWR|O_SYNC); */
   /* gpio2 = (volatile char *)mmap(0, 0xFFF, PROT_READ|PROT_WRITE, MAP_SHARED, memdev2, GPIO2);  */
   /* printf("2 %p\n", gpio2); */
   /* if(gpio2==MAP_FAILED){return 1;} */

   /* int memdev3 = open("/dev/mem", O_RDWR|O_SYNC); */
   /* gpio3 = (volatile char *)mmap(0, 0xFFF, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO3);  */
   /* printf("%i %p\n", memdev, gpio3); */
   /* if(gpio3==MAP_FAILED){return 1;} */

   /* printf("%s\n", "DA"); */
   /* HWREG(gpio3, GPIO_CTRL) &= ~(0x03); */

   /* cm_wkup = (volatile char*)mmap(0, 0xFF, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, CM_WKUP); */
   /* if(cm_wkup==MAP_FAILED){return 1;} */
   /* if(map_hardware_address(CM_WKUP, 0xFF, cm_wkup)){return 1;} */
   /* printf("%s\n", "E"); */

   // Enable GPIO0 module
   printf("%s\n", "AA");
   HWREG(gpio0, GPIO_CTRL) &= ~(0x03);
   /* *((volatile unsigned long*)(gpio0+GPIO_CTRL)) &= ~(0x03); */
   // Enable clock for GPIO0 module. 
   /* HWREG(cm_wkup, CM_WKUP_GPIO0_CLKCTRL) = (0x02) | (1<<18); */
   // Set debounce time for GPIO0 module
   // time = (DEBOUNCINGTIME + 1) * 31uSec
   /* printf("%s\n", "AB"); */
   HWREG(gpio0, GPIO_DEBOUNCINGTIME) |= 255;

   // Enable GPIO1 Module.
   printf("%s\n", "BA");
   HWREG(gpio1, GPIO_CTRL) &= ~(0x03);
   /* *((volatile unsigned long*)(gpio1+GPIO_CTRL)) &= ~(0x03); */
   // Enable clock for GPIO1 module. 
   /* HWREG(CM_PER, CM_PER_GPIO1_CLKCTRL) = (0x02) | (1<<18); */
   // Set debounce time for GPIO1 module
   // time = (DEBOUNCINGTIME + 1) * 31uSec
   /* printf("%s\n", "BB"); */
   /* HWREG(gpio1, GPIO_DEBOUNCINGTIME) |= 255; */

   // Enable GPIO2 Module.
   /* printf("%s\n", "CA"); */
   /* HWREG(gpio2, GPIO_CTRL) &= ~(0x03); */
   /* // Enable clock for GPIO2 module.  */
   /* #<{(| HWREG(CM_PER, CM_PER_GPIO2_CLKCTRL) = (0x02) | (1<<18); |)}># */
   /* // Set debounce time for GPIO2 module */
   /* // time = (DEBOUNCINGTIME + 1) * 31uSec */
   /* HWREG(gpio2, GPIO_DEBOUNCINGTIME) = 255; */

   // Enable GPIO3 Module.
   /* printf("%s\n", "DA"); */
   /* HWREG(gpio3, GPIO_CTRL) &= ~(0x03); */
   /* HWREG(gpio3, GPIO_CTRL) = 0x00; */
   /* // Enable clock for GPIO3 module.  */
   /* #<{(| HWREG(CM_PER, CM_PER_GPIO3_CLKCTRL) = (0x02) | (1<<18); |)}># */
   /* // Set debounce time for GPIO3 module */
   /* // time = (DEBOUNCINGTIME + 1) * 31uSec */
   /* HWREG(gpio3, GPIO_DEBOUNCINGTIME) = 255; */

   sleep(3);
   close(memdev0);

   return 0;
}

static int get_gpio_pin_name(int gpio_number, char* pin_name){
   switch(gpio_number){
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
      case P9_41B: 
         strcpy(pin_name, "P9_41B");
         break;
      case P9_42A: 
         strcpy(pin_name, "P9_42A");
         break;
      case P9_42B: 
         strcpy(pin_name, "P9_42B");
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
            return 1;
         }
         while(dir2){
            dir_info = readdir(dir2);
            if(dir_info==NULL){
               closedir(dir2);
               closedir(dir);
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
// ANALOG DIGITAL CONVERTER

/* static int init_adc(){ */
/*    // Get pointers to ADC config registers */
/*    volatile char* adc_tsc = NULL; */
/*    if(map_hardware_address(ADC_TSC, 0x2000, adc_tsc)){return 1;} */
/*  */
/*    #<{(| volatile char* cm_wkup = NULL; |)}># */
/*    #<{(| if(map_hardware_address(CM_WKUP, 0x100, cm_wkup)){return 1;} |)}># */
/*  */
/*    // Enable clock for adc module. */
/*    #<{(| HWREG(cm_wkup, CM_WKUP_ADC_TSK_CLKCTL) = 0x02; |)}># */
/*  */
/*    // Disable ADC module temporarily. */
/*    HWREG(adc_tsc, ADC_TSC_CTRL) &= ~(0x01); */
/*  */
/*    // To calculate sample rate: */
/*    // fs = 24MHz / (CLK_DIV*2*Channels*(OpenDly+Average*(14+SampleDly))) */
/*    // We want 48KHz. (Compromising to 50KHz) */
/*    unsigned int clock_divider = 1; */
/*    unsigned int open_delay = 0; */
/*    unsigned int average = 0;       // can be 0 (no average), 1 (2 samples),  */
/*    // 2 (4 samples),  3 (8 samples)  */
/*    // or 4 (16 samples) */
/*    unsigned int sample_delay = 0; */
/*  */
/*    // Set clock divider (set register to desired value minus one).  */
/*    HWREG(adc_tsc, ADC_TSC_CLKDIV) = clock_divider - 1; */
/*  */
/*    // Set values range from 0 to FFF. */
/*    HWREG(adc_tsc, ADC_TSC_ADCRANGE) = (0xfff << 16) & (0x000); */
/*  */
/*    // Disable all steps. STEPENABLE register */
/*    HWREG(adc_tsc, ADC_TSC_STEPENABLE) &= ~(0xff); */
/*  */
/*    // Unlock step config register. */
/*    HWREG(adc_tsc, ADC_TSC_CTRL) |= (1 << 2); */
/*  */
/*    // Set config and delays for step 1:  */
/*    // Sw mode, one shot mode, fifo0, channel 0. */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG1) = 0 | (0<<26) | (0<<19) | (0<<15) | (average<<2) | (0); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY1)  = 0 | (sample_delay - 1)<<24 | open_delay; */
/*  */
/*    // Set config and delays for step 2:  */
/*    // Sw mode, one shot mode, fifo0, channel 1. */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG2) = 0 | (0x0<<26) | (0x01<<19) | (0x01<<15) | (average<<2) | (0x00); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY2)  = 0 | (sample_delay - 1)<<24 | open_delay; */
/*  */
/*    // Set config and delays for step 3:  */
/*    // Sw mode, one shot mode, fifo0, channel 2. */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG3) = 0 | (0x0<<26) | (0x02<<19) | (0x02<<15) | (average<<2) | (0x00); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY3)  = 0 | ((sample_delay - 1)<<24) | open_delay; */
/*  */
/*    // Set config and delays for step 4:  */
/*    // Sw mode, one shot mode, fifo0, channel 3. */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG4) = 0 | (0x0<<26) | (0x03<<19) | (0x03<<15) | (average<<2) | (0x00); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY4)  = 0 | ((sample_delay - 1)<<24) | open_delay; */
/*  */
/*    // Set config and delays for step 5:  */
/*    // Sw mode, one shot mode, fifo0, channel 4. */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG5) = 0 | (0x0<<26) | (0x04<<19) | (0x04<<15) | (average<<2) | (0x00); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY5)  = 0 | ((sample_delay - 1)<<24) | open_delay; */
/*  */
/*    // Set config and delays for step 6:  */
/*    // Sw mode, one shot mode, fifo0, CHANNEL 6! */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG6) = 0 | (0x0<<26) | (0x06<<19) | (0x06<<15) | (average<<2) | (0x00); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY6)  = 0 | ((sample_delay - 1)<<24) | open_delay; */
/*  */
/*    // Set config and delays for step 7:  */
/*    // Sw mode, one shot mode, fifo0, CHANNEL 5! */
/*    HWREG(adc_tsc, ADC_TSC_STEPCONFIG7) = 0 | (0x0<<26) | (0x05<<19) | (0x05<<15) | (average<<2) | (0x00); */
/*    HWREG(adc_tsc, ADC_TSC_STEPDELAY7)  = 0 | ((sample_delay - 1)<<24) | open_delay; */
/*  */
/*    // Enable tag channel id. Samples in fifo will have channel id bits ADC_CTRL register */
/*    HWREG(adc_tsc, ADC_TSC_CTRL) |= (1 << 1); */
/*  */
/*    // Clear End_of_sequence interrupt */
/*    HWREG(adc_tsc, ADC_TSC_IRQSTATUS) |= (1<<1); */
/*  */
/*    // Enable End_of_sequence interrupt */
/*    HWREG(adc_tsc, ADC_TSC_IRQENABLE_SET) |= (1 << 1); */
/*  */
/*    // Lock step config register. ACD_CTRL register */
/*    HWREG(adc_tsc, ADC_TSC_CTRL) &= ~(1 << 2); */
/*  */
/*    // Clear FIFO0 by reading from it. */
/*    unsigned int count = HWREG(adc_tsc, ADC_TSC_FIFO0COUNT); */
/*    unsigned int i; */
/*    for(i=0; i<count; i++){ */
/*       HWREG(adc_tsc, ADC_TSC_FIFO0DATA); */
/*    } */
/*  */
/*    // Clear FIFO1 by reading from it. */
/*    count = HWREG(adc_tsc, ADC_TSC_FIFO1COUNT); */
/*    for(i=0; i<count; i++){ */
/*       HWREG(adc_tsc, ADC_TSC_FIFO1DATA); */
/*    } */
/*  */
/*    // Enable ADC Module. ADC_CTRL register */
/*    HWREG(adc_tsc, ADC_TSC_CTRL) |= 1; */
/*  */
/*    return 0; */
/* } */

/////////////////////////////////////////////////////////////////////
// DEVICE TREE OVERLAY
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

/////////////////////////////////////////////////////////////////////
// PRU Initialization
//

static int init_pru_system(){
   /* tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA; */
   /* if(prussdrv_init()) return 1; */
   /* if(prussdrv_open(PRU_EVTOUT_0)) return 1; */
   /* if(prussdrv_pruintc_init(&pruss_intc_initdata)) return 1; */
   /*  */
   /* // Get pointer to shared ram */
   /* void* p; */
   /* if(prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &p)) return 1; */
   /* bbb_pruio_shared_ram = (volatile unsigned int*)p; */

   return 0;
}

static int start_pru0_program(){
   char path[512] = "";
   strcat(path, BBB_PRUIO_PREFIX); 
   strcat(path, "/lib/libbbb_pruio_data0.bin");
   if(prussdrv_load_datafile(0, path)) return 1;

   strcpy(path, BBB_PRUIO_PREFIX); 
   strcat(path, "/lib/libbbb_pruio_text0.bin");
   if(prussdrv_exec_program_at(0, path, BBB_PRUIO_START_ADDR_0)) return 1;

   return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Ring buffer (see header file for more)
//

static void buffer_init(){
   bbb_pruio_buffer_size = 1024;
   bbb_pruio_buffer_start = &(bbb_pruio_shared_ram[1024]); 
   bbb_pruio_buffer_end = &(bbb_pruio_shared_ram[1025]); 
   *bbb_pruio_buffer_start = 0;
   *bbb_pruio_buffer_end = 0;
}

/////////////////////////////////////////////////////////////////////
// "Public" functions.
//

int bbb_pruio_start(){
   // DTOs
   /* if(load_device_tree_overlays()){ */
   /*    fprintf(stderr, "libbbb_pruio: Could not load device tree overlays.\n"); */
   /*    return 1; */
   /* } */
    
   // Allow access to hardware registers
   map_hardware_memory();

   // Init GPIO hardware
   if(init_gpio()){
      fprintf(stderr, "libbbb_pruio: Could not init GPIO\n");
      return 1;
   }
   
   // PRU
   /* if(init_pru_system()){ */
   /*    fprintf(stderr, "libbbb_pruio: Could not init PRU system.\n"); */
   /*    return 1; */
   /* } */

   // Init GPIO pins used to control analog mux:
   /* int e1 = bbb_pruio_init_gpio_pin(P9_27, BBB_PRUIO_OUTPUT_MODE); */
   /* int e2 = bbb_pruio_init_gpio_pin(P9_30, BBB_PRUIO_OUTPUT_MODE); */
   /* int e3 = bbb_pruio_init_gpio_pin(P9_42A, BBB_PRUIO_OUTPUT_MODE); */
   /* if(e1 || e2 || e3){ */
   /*    fprintf(stderr, "libbbb_pruio: Could not init mux control pins.\n"); */
   /*    return 1; */
   /* }  */


   /* // Ring Buffer */
   /* buffer_init(); */
   /*  */
   /* // Analog digital converter */
   /* if(init_adc()){ */
   /*    fprintf(stderr, "libbbb_pruio: Could not init ADC system.\n"); */
   /*    return 1; */
   /* } */
   /*  */
   /* // Start the PRU software */
   /* if(start_pru0_program()){ */
   /*    fprintf(stderr, "libbbb_pruio: Could not load PRU0 program.\n"); */
   /*    return 1; */
   /* } */
   /*  */
   return 0;
}

int bbb_pruio_stop(){
   // TODO: send terminate message to PRU
   /* prussdrv_pru_disable(0); */
   /* prussdrv_exit(); */

   close_memory_map();
   return 0;
}

int bbb_pruio_get_gpio_number(char* pin_name){
   if(strcmp(pin_name, "P9_11") == 0){
      return P9_11;
   }
   else if(strcmp(pin_name, "P9_12") == 0){
      return P9_12;
   }
   else if(strcmp(pin_name, "P9_13") == 0){
      return P9_13;
   }
   else if(strcmp(pin_name, "P9_14") == 0){
      return P9_14;
   }
   else if(strcmp(pin_name, "P9_15") == 0){
      return P9_15;
   }
   else if(strcmp(pin_name, "P9_16") == 0){
      return P9_16;
   }
   else if(strcmp(pin_name, "P9_17") == 0){
      return P9_17;
   }
   else if(strcmp(pin_name, "P9_18") == 0){
      return P9_18;
   }
   else if(strcmp(pin_name, "P9_21") == 0){
      return P9_21;
   }
   else if(strcmp(pin_name, "P9_22") == 0){
      return P9_22;
   }
   else if(strcmp(pin_name, "P9_23") == 0){
      return P9_23;
   }
   else if(strcmp(pin_name, "P9_24") == 0){
      return P9_24;
   }
   else if(strcmp(pin_name, "P9_26") == 0){
      return P9_26;
   }
   else if(strcmp(pin_name, "P9_27") == 0){
      return P9_27;
   }
   else if(strcmp(pin_name, "P9_30") == 0){
      return P9_30;
   }
   else if(strcmp(pin_name, "P9_41A") == 0){
      return P9_41A;
   }
   else if(strcmp(pin_name, "P9_41B") == 0){
      return P9_41B;
   }
   else if(strcmp(pin_name, "P9_42A") == 0){
      return P9_42A;
   }
   else if(strcmp(pin_name, "P9_42B") == 0){
      return P9_42B;
   }
   return -1;
}

int bbb_pruio_init_adc_pin(int channel_number){
   /** 
    * Tell the PRU unit that we are interested in input from this channel.
    *
    * Each one of the 14 least significant bits in shared_ram[1030]
    * represent the 14 ADC channels. If a bit is set, it means that we need
    * input for that ADC channel.
    *
    * shared_ram[1030] -> ADC Channels in use
    *
    */
   bbb_pruio_shared_ram[1030] |= (1<<channel_number);
   return 0;
}

int bbb_pruio_init_gpio_pin(int gpio_number, bbb_pruio_gpio_mode mode){
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
   FILE *f = fopen(path, "rt");
   if(f==NULL){
      return 1;
   }
   int gpio_module = gpio_number >> 5;
   int gpio_bit = gpio_number % 32;
   if(mode == BBB_PRUIO_OUTPUT_MODE){
      fprintf(f, "%s\n", "output"); 
      // Reset the output enable bit in the gpio config register to actually enable output.
      switch(gpio_module){
         case 0: HWREG(gpio0, GPIO_OE) &= ~(1<<gpio_bit); break;
         case 1: HWREG(gpio1, GPIO_OE) &= ~(1<<gpio_bit); break;
         case 2: HWREG(gpio2, GPIO_OE) &= ~(1<<gpio_bit); break;
         case 3: HWREG(gpio3, GPIO_OE) &= ~(1<<gpio_bit); break;
      }
   }
   else{
      fprintf(f, "%s\n", "input"); 
      // Set the output enable bit in the gpio config register to disable output.
      switch(gpio_module){
         case 0: HWREG(gpio0, GPIO_OE) |= (1<<gpio_bit); break;
         case 1: HWREG(gpio1, GPIO_OE) |= (1<<gpio_bit); break;
         case 2: HWREG(gpio2, GPIO_OE) |= (1<<gpio_bit); break;
         case 3: HWREG(gpio3, GPIO_OE) |= (1<<gpio_bit); break;
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
      bbb_pruio_shared_ram[gpio_module+1026] |= (1<<gpio_bit);
 
   }
   fclose(f);
   return 0;
}

/* void bbb_pruio_set_pin_value(int gpio_number, int value){ */
/*    int gpio_module = gpio_number >> 5; */
/*    int gpio_bit = gpio_number % 32; */
/*    volatile char* reg=NULL; */
/*    switch(gpio_module){ */
/*       case 0: reg = gpio0; break; */
/*       case 1: reg = gpio1; break; */
/*       case 2: reg = gpio2; break; */
/*       case 3: reg = gpio3; break; */
/*    } */
/*    if(value==1){ */
/*       HWREG(reg, GPIO_DATAOUT) |= (1<<gpio_bit); */
/*    } */
/*    else{ */
/*       HWREG(reg, GPIO_DATAOUT) &= ~(1<<gpio_bit); */
/*    } */
/* } */
