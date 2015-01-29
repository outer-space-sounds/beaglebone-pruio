#include <stdlib.h>
#include <stdio.h>
#include <bbb_pruio_pins.h>
#include "m_pd.h"

#ifdef IS_BEAGLEBONE
#include <bbb_pruio.h>
#endif 

/////////////////////////////////////////////////////////////////////////
// PD library bootstrapping.
//
void gpio_input_setup(void);
void gpio_output_setup(void);
void adc_input_setup(void);
void adc_input_tilde_setup(void);

void beaglebone_setup(void){
   #ifdef IS_BEAGLEBONE
      bbb_pruio_start();
   #endif 
   gpio_input_setup();
   gpio_output_setup();
   adc_input_setup();
   adc_input_tilde_setup();
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
/* #define CLOCK_PERIOD 1000 // milliseconds */
#define CLOCK_PERIOD 0.6666 // milliseconds
#endif

typedef struct callback{
   void(*callback_function)(void*, t_float);
   void* instance;
} callback;

callback digital_callbacks[BBB_PRUIO_MAX_GPIO_CHANNELS];
callback analog_callbacks[BBB_PRUIO_MAX_ADC_CHANNELS];

static int beaglebone_number_of_instances = 0;
static t_clock* beaglebone_clock = NULL;

void beaglebone_clock_tick(void* x){
   (void)x; // Do not use x, means nothing here, 
            // we're passing null to the "owner" in clock_new
   
   callback *cbk;
   #ifdef IS_BEAGLEBONE
      bbb_pruio_message message;
      while(bbb_pruio_messages_are_available()){
         bbb_pruio_read_message(&message);

         // Message from gpio
         if(message.is_gpio){
            cbk = &digital_callbacks[message.gpio_number];

            // Debug
            /* if(message.gpio_number >= BBB_PRUIO_MAX_GPIO_CHANNELS || cbk->instance == NULL || cbk->callback_function==NULL){ */
            /*    printf("A! i:%p cbk:%p val:%i gpio_num:%i \n", cbk->instance, cbk->callback_function, message.value, message.gpio_number); */
            /*    continue; //while */
            /* } */

            /* if(message.gpio_number<BBB_PRUIO_MAX_GPIO_CHANNELS && cbk->instance!=NULL && cbk->callback_function!=NULL){ */
               cbk->callback_function(cbk->instance, message.value);
            /* } */
         }
         else{ // adc
            cbk = &analog_callbacks[message.adc_channel];

            // Debug
            /* if(message.adc_channel >= BBB_PRUIO_MAX_ADC_CHANNELS || cbk->instance == NULL || cbk->callback_function==NULL){ */
            /*    printf("A! i:%p cbk:%p val:%i chan:%i \n", cbk->instance, cbk->callback_function, message.value, message.adc_channel); */
            /*    continue; //while */
            /* } */

            /* if(message.adc_channel<BBB_PRUIO_MAX_ADC_CHANNELS && cbk->instance!=NULL && cbk->callback_function!=NULL){ */
               cbk->callback_function(cbk->instance, (t_float)message.value/127.0);
            /* } */
         }
      }
   #else
      int i;
      for(i=0; i<BBB_PRUIO_MAX_GPIO_CHANNELS; ++i){
         cbk = &digital_callbacks[i];
         if(cbk->instance != NULL){
            cbk->callback_function(cbk->instance, rand()%2);
         }
      }

      for(i=0; i<BBB_PRUIO_MAX_ADC_CHANNELS; ++i){
         cbk = &analog_callbacks[i];
         if(cbk->instance != NULL){
            cbk->callback_function(cbk->instance, (t_float)rand()/(t_float)RAND_MAX);
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
                         void (*callback_function)(void*, t_float), 
                         char* err){
   
   if(beaglebone_number_of_instances==0){
      int i;
      for(i=0; i<BBB_PRUIO_MAX_GPIO_CHANNELS; ++i){
         callback new_callback;
         new_callback.callback_function = NULL;
         new_callback.instance = NULL;
         digital_callbacks[i] = new_callback;
      }

      for(i=0; i<BBB_PRUIO_MAX_ADC_CHANNELS; ++i){
         callback new_callback;
         new_callback.callback_function = NULL;
         new_callback.instance = NULL;
         analog_callbacks[i] = new_callback;
      }

   }
   int gpio_number = -1;
   int adc_number = -1;
   if(is_digital==1){
      gpio_number = bbb_pruio_get_gpio_number(channel);
      if(gpio_number==-1){
         sprintf(err, "%s is not a valid GPIO pin.", channel);
         return 1;
      }
   }
   else{
      adc_number = strtol(channel, NULL, 10);
      if(adc_number<0 || adc_number>BBB_PRUIO_MAX_ADC_CHANNELS){
         sprintf(err, "%s is not a valid ADC channel.", channel);
         return 1;
      }
   }

   #ifdef IS_BEAGLEBONE
      if(is_digital==1){
         if(bbb_pruio_init_gpio_pin(gpio_number, 1)){  // 1 for input
            sprintf(err, "Could not init pin %s (%i), is it already in use?", channel, gpio_number);
            return 1;
         }
      }
      else{
         if(bbb_pruio_init_adc_pin(adc_number)){
            sprintf(err, "Could not init adc channel %s (%i), is it already in use?", channel, adc_number);
            return 1;
         }
      }
   #else
      if(is_digital==1){
         if(digital_callbacks[gpio_number].instance!=NULL){
            sprintf(err, "Could not init pin %s (%i), is it already in use?", channel, gpio_number);
            return 1;
         }
      }
      else{
         if(analog_callbacks[adc_number].instance!=NULL){
            sprintf(err, "Could not init adc channel %s (%i), is it already in use?", channel, adc_number);
            return 1;
         }
      }
   #endif 
   
   callback new_callback;
   new_callback.callback_function = callback_function;
   new_callback.instance = instance;
   if(is_digital==1){
      digital_callbacks[gpio_number] = new_callback;
   }
   else{
      analog_callbacks[adc_number] = new_callback;
   }


   if(beaglebone_number_of_instances==0){
      beaglebone_clock = clock_new(NULL,  (t_method)beaglebone_clock_tick); 
      clock_delay(beaglebone_clock, CLOCK_PERIOD);
   }
   beaglebone_number_of_instances++;

   return 0;
}

void beaglebone_clock_free(int is_digital, char* channel){
   // TODO uninit pin?
   
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
      // TODO: stop lib pruio?
      clock_free(beaglebone_clock);
      beaglebone_clock = NULL;
   }
}

