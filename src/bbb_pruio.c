#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <bbb_pruio.h>

/////////////////////////////////////////////////////////////////////
// Array to store adc callbacks
//

// An array of 14 function pointers called callbacks
void (*adc_callbacks[14])(unsigned int, unsigned int);

static void null_adc_callbacks(){
   int i;
   for(i=0; i<14; i++){
      adc_callbacks[i] = 0; 
   }
}

/////////////////////////////////////////////////////////////////////
// A thread to wait for ADC interrupts from PRU1
//

static pthread_t adc_thread;

static void* adc_thread_function(void* param){
   while(1){
      sleep(5);
      (*adc_callbacks[1])(64, 64);
      sleep(1);
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

int bbb_pruio_init_adc(){
   null_adc_callbacks();
   start_adc_thread();
   return 0;
}

int bbb_pruio_init_adc_pin(unsigned int pin_number, unsigned int steps, bbb_pruio_adc_callback){

   adc_callbacks[pin_number] = callback;

   return 0;
}
int bbb_pruio_close_adc(){
   stop_adc_thread();
   null_adc_callbacks();

   return 0;
}
