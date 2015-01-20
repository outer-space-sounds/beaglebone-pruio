#include <stdlib.h>
#include "m_pd.h"

#ifdef IS_BEAGLEBONE
#include <bbb_pruio.h>
#endif 
#include <bbb_pruio_pins.h>

/////////////////////////////////////////////////////////////////////////
// PD library bootstrapping.
//
void gpio_input_setup(void);
void gpio_output_setup(void);
void adc_input_setup(void);

void beaglebone_setup(void){
   gpio_input_setup();
   gpio_output_setup();
   adc_input_setup();
}

//////////////////////////////////////////////////////////////////////
// Common functions and variables
//

// The main idea here is that instances of pd objects register themselves
// to receive callbacks for their input of interest (analog or digital)
// using the beaglebone_clock_new function. This adds a new entry to the
// callbacks array. The callbacks array will be checked when a new 
// message from the PRU arrives and the callback will be triggered.

#ifdef IS_BEAGLEBONE
#define CLOCK_PERIOD 0.6666 // milliseconds
#else
#define CLOCK_PERIOD 1000 // milliseconds
#endif

typedef struct callback{
   void(*callback_function)(void*, float);
   void* instance;
} callback;

callback digital_callbacks[BBB_PRUIO_MAX_GPIO_CHANNELS+1];
callback analog_callbacks[BBB_PRUIO_MAX_ADC_CHANNELS+1];

static int beaglebone_number_of_instances = 0;
static t_clock* beaglebone_clock = NULL;

void beaglebone_clock_tick(void* x){
   (void)x; // Do not use x, means nothing here, 
            // we're passing null to the "owner" in clock_new
   
   callback cbk;
   #ifdef IS_BEAGLEBONE
      // TODO
   #else
      int i;
      for(i=0; i<=BBB_PRUIO_MAX_GPIO_CHANNELS; ++i){
         cbk = digital_callbacks[i];
         if(cbk.instance != NULL){
            cbk.callback_function(cbk.instance, rand()%2);
         }
      }

      for(i=0; i<=BBB_PRUIO_MAX_ADC_CHANNELS; ++i){
         cbk = analog_callbacks[i];
         if(cbk.instance != NULL){
            cbk.callback_function(cbk.instance, (float)rand()/(float)RAND_MAX);
         }
      }
   #endif 

   clock_delay(beaglebone_clock, CLOCK_PERIOD);
}

/**
 * set is_digital to 1 if digital pin, 0 if analog
 */
int beaglebone_clock_new(int is_digital, 
                         char* channel, 
                         void* instance, 
                         void (*callback_function)(void*, float), 
                         char* err){
   
   int gpio_number;
   if(is_digital==1){
      gpio_number = bbb_pruio_get_gpio_number(channel);
      if(gpio_number==-1){
         sprintf(err, "%s is not a valid GPIO pin.", channel);
         return 1;
      }
   }
   else{
      gpio_number = strtol(channel, NULL, 10);
      if(gpio_number==0 || gpio_number>BBB_PRUIO_MAX_ADC_CHANNELS){
         sprintf(err, "%s is not a valid ADC channel.", channel);
         return 1;
      }
   }

   #ifdef IS_BEAGLEBONE
      // TODO: check with libpruio if pin was actually inited
      /* err = "lalala"; */
      // return 1;
   #endif 
   
   callback new_callback;
   new_callback.callback_function = callback_function;
   new_callback.instance = instance;
   if(is_digital==1){
      digital_callbacks[gpio_number] = new_callback;
   }
   else{
      analog_callbacks[gpio_number] = new_callback;
   }


   if(beaglebone_number_of_instances==0){
      beaglebone_clock = clock_new(NULL, (t_method)beaglebone_clock_tick);
      clock_delay(beaglebone_clock, CLOCK_PERIOD);
   }
   beaglebone_number_of_instances++;

   return 0;
}

void beaglebone_clock_free(int is_digital, char* channel){
   callback *cbk;
   if(is_digital==1){
      int gpio_number = bbb_pruio_get_gpio_number(channel);
      cbk = &(digital_callbacks[gpio_number]);
   }
   else{
      int adc_channel = strtol(channel, NULL, 10);
      cbk = &(analog_callbacks[adc_channel]);
   }
   cbk->callback_function = NULL;
   cbk->instance = NULL;

   beaglebone_number_of_instances--;
   if(beaglebone_number_of_instances==0){
      clock_free(beaglebone_clock);
      beaglebone_clock = NULL;
   }
}

