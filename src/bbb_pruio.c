#include <pthread.h>
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

/////////////////////////////////////////////////////////////////////
// PRU Initialization
//

static volatile unsigned int* shared_ram = NULL;

static int load_device_tree_overlay(){
   // Check if device tree overlay is loaded, load if needed.
   int device_tree_overlay_loaded = 0; 
   FILE* f;
   f = fopen("/sys/devices/bone_capemgr.9/slots","rt");
   if(f==NULL){
      return(1);
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
         return(1);
      }
      fprintf(f, "PRUIO-DTO");
      fclose(f);
      usleep(100000);
   }

   return 0;
}

static int init_pru0_program(){
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   prussdrv_init();
   if(prussdrv_open(PRU_EVTOUT_0)) return 1;
   if(prussdrv_pruintc_init(&pruss_intc_initdata)) return 2;

   // Get pointer to shared ram
   void* p;
   prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &p);
   shared_ram = (volatile unsigned int*)p;

   char path[512] = "";
   strcat(path, BBB_PRUIO_PREFIX); 
   strcat(path, "/lib/libbbb_pruio_data0.bin");
   printf("%s \n",path);
   if(prussdrv_load_datafile(0, path)) return 3;

   strcpy(path, BBB_PRUIO_PREFIX); 
   strcat(path, "/lib/libbbb_pruio_text0.bin");
   if(prussdrv_exec_program_at(0, path, BBB_PRUIO_START_ADDR_0)) return 4;

   return 0;
}


/////////////////////////////////////////////////////////////////////
// Array to store adc callbacks
//

// An array of 14 function pointers called callbacks
static void (*adc_callbacks[14])(unsigned int, unsigned int);

static void reset_adc_callbacks(){
   int i;
   for(i=0; i<14; i++){
      adc_callbacks[i] = 0; 
   }
}

/////////////////////////////////////////////////////////////////////
// A thread to wait for ADC interrupts from PRU0
//

static pthread_t adc_thread;

static void* adc_thread_function(void* param){
   while(1){
      // Wait for interrupt from PRU
      prussdrv_pru_wait_event(PRU_EVTOUT_0);
      prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

      (*adc_callbacks[1])(64, 64);
      (*adc_callbacks[2])(3, 87);
   }
   return 0;
}

static int start_adc_thread(){
   // TODO: set real time priority to this thread
   pthread_attr_t attr;
   if(pthread_attr_init(&attr)){
      return 1;
   }
   if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)){
      return 1;
   }
   if(pthread_create(&adc_thread, &attr, &adc_thread_function, NULL)){
      return 1;
   }

   return 0;
}

static void stop_adc_thread(){
   while(pthread_cancel(adc_thread)){}
}

/////////////////////////////////////////////////////////////////////
// "Public" functions. ADC API.
//

int bbb_pruio_start_adc(){
   int err;
   load_device_tree_overlay();
   reset_adc_callbacks();
   start_adc_thread();
   if((err=init_pru0_program())){
      printf("Could not load PRU0 program: %i \n", err);
   }
   return 0;
}

int bbb_pruio_init_adc_pin(unsigned int pin_number, unsigned int steps, bbb_pruio_adc_callback){

   adc_callbacks[pin_number] = callback;

   return 0;
}

int bbb_pruio_stop_adc(){
   prussdrv_pru_disable(0);
   prussdrv_exit ();

   stop_adc_thread();
   reset_adc_callbacks();

   return 0;
}
