/*
 * Lib BEAGLEBONE Pruio
 * Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <beaglebone_pruio_pins.h>

#ifdef IS_BEAGLEBONE
#include <beaglebone_pruio.h>
#endif 

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// PD library bootstrapping.
//
void gpio_input_setup(void);
void gpio_output_setup(void);
void adc_input_setup(void);
void adc_input_tilde_setup(void);
void display_7_led_setup(void);
void midi_in_setup(void);

void beaglebone_setup(void){
   #ifdef IS_BEAGLEBONE
      beaglebone_pruio_start();
   #endif 
   gpio_input_setup();
   gpio_output_setup();
   adc_input_setup();
   adc_input_tilde_setup();
   display_7_led_setup();
   midi_in_setup();
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
/* #define CLOCK_PERIOD 0.6666 // milliseconds */
#endif

typedef struct callback{
   void(*callback_function)(void*, t_float);
   void* instance;
} callback;

callback digital_callbacks[BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS];
callback analog_callbacks[BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS];

static int beaglebone_number_of_instances = 0;
static t_clock* beaglebone_clock = NULL;

void beaglebone_clock_tick(void* x){
   (void)x; // Do not use x, means nothing here, 
            // we're passing null to the "owner" in clock_new
   
   callback *cbk;
   #ifdef IS_BEAGLEBONE
      beaglebone_pruio_message message;
      while(beaglebone_pruio_messages_are_available()){
         beaglebone_pruio_read_message(&message);

         // Message from gpio
         if(message.is_gpio){
            cbk = &digital_callbacks[message.gpio_number];

            // Debug
            /* if(message.gpio_number >= BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS || cbk->instance == NULL || cbk->callback_function==NULL){ */
            /*    printf("A! i:%p cbk:%p val:%i gpio_num:%i \n", cbk->instance, cbk->callback_function, message.value, message.gpio_number); */
            /*    continue; //while */
            /* } */

            /* if(message.gpio_number<BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS && cbk->instance!=NULL && cbk->callback_function!=NULL){ */
               cbk->callback_function(cbk->instance, message.value);
            /* } */
         }
         else{ // adc
            cbk = &analog_callbacks[message.adc_channel];

            // Debug
            /* if(message.adc_channel >= BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS || cbk->instance == NULL || cbk->callback_function==NULL){ */
            /*    printf("A! i:%p cbk:%p val:%i chan:%i \n", cbk->instance, cbk->callback_function, message.value, message.adc_channel); */
            /*    continue; //while */
            /* } */

            /* if(message.adc_channel<BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS && cbk->instance!=NULL && cbk->callback_function!=NULL){ */
               cbk->callback_function(cbk->instance, (t_float)message.value);
            /* } */
         }
      }
   #else
      int i;
      for(i=0; i<BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS; ++i){
         cbk = &digital_callbacks[i];
         if(cbk->instance != NULL){
            cbk->callback_function(cbk->instance, rand()%2);
         }
      }

      for(i=0; i<BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS; ++i){
         cbk = &analog_callbacks[i];
         if(cbk->instance != NULL){
            cbk->callback_function(cbk->instance, (t_float)rand()/(t_float)RAND_MAX);
         }
      }
   #endif 

   clock_delay(beaglebone_clock, CLOCK_PERIOD);
}

int beaglebone_register_callback(
      int is_digital, 
      int channel, 
      void* instance, 
      void (*callback_function)(void*, t_float) 
){
   
   // First time here. Init arrays.
   if(beaglebone_number_of_instances==0){
      int i;
      for(i=0; i<BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS; ++i){
         callback new_callback;
         new_callback.callback_function = NULL;
         new_callback.instance = NULL;
         digital_callbacks[i] = new_callback;
      }

      for(i=0; i<BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS; ++i){
         callback new_callback;
         new_callback.callback_function = NULL;
         new_callback.instance = NULL;
         analog_callbacks[i] = new_callback;
      }
   }
   
   // If there's already a callback for this channel, bail out.
   if(is_digital){
      if(digital_callbacks[channel].instance!=NULL){
         return 1;    
      }
   }
   else{
      if(analog_callbacks[channel].instance!=NULL){
         return 1;    
      }
   }

   // All good, register the callback
   callback new_callback;
   new_callback.callback_function = callback_function;
   new_callback.instance = instance;
   if(is_digital==1){
      digital_callbacks[channel] = new_callback;
   }
   else{
      analog_callbacks[channel] = new_callback;
   }

   // First time here. Start the clock.
   if(beaglebone_number_of_instances==0){
      beaglebone_clock = clock_new(NULL,  (t_method)beaglebone_clock_tick); 
      clock_delay(beaglebone_clock, CLOCK_PERIOD);
   }

   beaglebone_number_of_instances++;

   return 0;
}

void beaglebone_unregister_callback(int is_digital, int channel){
   // TODO uninit pin?
   
   callback *cbk;
   if(is_digital==1){
      cbk = &(digital_callbacks[channel]);
   }
   else{
      cbk = &(analog_callbacks[channel]);
   }
   cbk->callback_function = NULL;
   cbk->instance = NULL;

   beaglebone_number_of_instances--;
   if(beaglebone_number_of_instances==0){
      clock_free(beaglebone_clock);
      beaglebone_clock = NULL;
      #ifdef IS_BEAGLEBONE
         beaglebone_pruio_stop();
      #endif
   }
}
