#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "bbb_pruio.h"

#ifndef BBB_PRUIO_START_ADDR_0
   #error "BBB_PRUIO_START_ADDR_0 must be defined."
#endif

#ifndef BBB_PRUIO_PREFIX
   #error "BBB_PRUIO_PREFIX must be defined."
#endif

/* #define DEBUG */

/////////////////////////////////////////////////////////////////////
// Forward declarations
//
static int load_device_tree_overlay();
static int init_pru_system();
static void buffer_init();
static int start_pru0_program();

/////////////////////////////////////////////////////////////////////
// "Public" functions.
//

int bbb_pruio_start(){
   int err;
   if(load_device_tree_overlay()){
      fprintf(stderr, "libbbb_pruio: Could not load device tree overlay.\n");
      return 1;
   }

   if((err=init_pru_system())){
      fprintf(stderr, "libbbb_pruio: Could not init PRU system: %i \n", err);
      return 1;
   }

   buffer_init();

   if((err=start_pru0_program())){
      fprintf(stderr, "libbbb_pruio: Could not load PRU0 program: %i \n", err);
      return 1;
   }

   return 0;
}

/* int bbb_pruio_init_adc_pin(unsigned int pin_number){ */
/*    return 0; */
/* } */

/* int bbb_pruio_init_gpio_pin(unsigned int pin_number){ */
/*    return 0; */
/* } */

int bbb_pruio_stop(){
   // TODO: send terminate message to PRU

   prussdrv_pru_disable(0);
   prussdrv_exit();

   return 0;
}

inline void bbb_pruio_set_pin_value(int gpio_number, int value){

}

/////////////////////////////////////////////////////////////////////
// PRU Initialization
//


static int load_device_tree_overlay(){
   // Check if device tree overlay is loaded, load if needed.
   int device_tree_overlay_loaded = 0; 
   FILE* f;
   f = fopen("/sys/devices/bone_capemgr.9/slots","rt");
   if(f==NULL){
      return 1;
   }
   char line[256];
   while(fgets(line, 256, f) != NULL){
      if(strstr(line, "PRUIO-DTO") != NULL){
         device_tree_overlay_loaded = 1; 
      }
   }
   fclose(f);

   if(!device_tree_overlay_loaded){
      f = fopen("/sys/devices/bone_capemgr.9/slots","w");
      if(f==NULL){
         return 1;
      }
      fprintf(f, "PRUIO-DTO");
      fclose(f);
      usleep(100000);
   }

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
   bbb_pruio_shared_ram = (volatile unsigned int*)p;

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
   bbb_pruio_buffer_start = &(bbb_pruio_shared_ram[1024]); // value inited to 0 in pru
   bbb_pruio_buffer_end = &(bbb_pruio_shared_ram[1025]); // value inited to 0 in pru
}
