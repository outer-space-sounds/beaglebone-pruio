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
void midi_notein_setup(void);

void beaglebone_setup(void){
   // Initialize hardware.
   // TODO: only initialize when an object actually needs the hardware
   #ifdef IS_BEAGLEBONE
     if(beaglebone_pruio_start()){
       error("beaglebone: Could not init pruio system");
     }
     if(beaglebone_midi_start()){
       error("beaglebone: Could not init midi port (UART)");
     }
   #endif 

   // Setup pd objects
   gpio_input_setup();
   gpio_output_setup();
   adc_input_setup();
   adc_input_tilde_setup();
   display_7_led_setup();
   midi_notein_setup();
}

//////////////////////////////////////////////////////////////////////
// Common functions and variables
//

// The main idea here is that instances of pd objects register themselves
// to receive callbacks for their input of interest (analog, digital 
// or midi) using the beaglebone_register_callback function. 
// This adds new entries to the callbacks arrays. The callbacks arrays
// will be checked when a new message from the PRU or Midi port arrives
// and the callback will be triggered.

#ifdef IS_BEAGLEBONE
#define CLOCK_PERIOD 0.6666 // milliseconds
#else
/* #define CLOCK_PERIOD 1000 // milliseconds */
#define CLOCK_PERIOD 0.6666 // milliseconds
#endif

typedef struct callback{
   void(*callback_function)(void*, t_float);
   void(*midi_callback_function)(void*, beaglebone_midi_message*);
   void* instance;
} callback;

static callback digital_callbacks[BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS];
static callback analog_callbacks[BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS];
static callback midi_notein_callbacks[1];

static beaglebone_midi_message midi_messages[16];
static int n_midi_messages = 0;

static int beaglebone_number_of_pruio_instances = 0;
static int beaglebone_number_of_midi_instances = 0;
t_clock* beaglebone_clock = NULL;

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
           if(cbk->callback_function) cbk->callback_function(cbk->instance, message.value);
         }
         else{ // adc
           cbk = &analog_callbacks[message.adc_channel];
           if(cbk->callback_function) cbk->callback_function(cbk->instance, (t_float)message.value);
         }
      }

      beaglebone_midi_receive_messages(midi_messages, &n_midi_messages);
      int i;
      for(i=0; i<n_midi_messages; ++i){
        if(midi_messages[i].type == BEAGLEBONE_MIDI_NOTE_ON){
          cbk = &(midi_notein_callbacks[0]);
          if(cbk->midi_callback_function){
            printf("A %i %i %p %p %p\n", n_midi_messages, i, cbk, cbk->midi_callback_function, cbk->instance);
            cbk->midi_callback_function(cbk->instance, &(midi_messages[i]));
            printf("WW\n");
          }
        }
        /* else if(midi_messages[i].type == BEAGLEBONE_MIDI_NOTE_OFF){ */
        /*   midi_messages[i].data[2] = 0; */
        /*   cbk = &(midi_notein_callbacks[0]); */
        /*   if(cbk->midi_callback_function){ */
        /*     printf("B %i %i %p\n", n_midi_messages, i, cbk->midi_callback_function); */
        /*     cbk->midi_callback_function(cbk->instance, &(midi_messages[i])); */
        /*     printf("WW\n"); */
        /*   } */
        /* } */
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

static void check_if_first_time(){
  // First time here. Init arrays.
  if(beaglebone_number_of_pruio_instances==0 && beaglebone_number_of_midi_instances==0){
    int i;
    for(i=0; i<BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS; ++i){
      callback new_callback;
      new_callback.callback_function = NULL;
      new_callback.midi_callback_function = NULL;
      new_callback.instance = NULL;
      digital_callbacks[i] = new_callback;
    }

    for(i=0; i<BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS; ++i){
      callback new_callback;
      new_callback.callback_function = NULL;
      new_callback.midi_callback_function = NULL;
      new_callback.instance = NULL;
      analog_callbacks[i] = new_callback;
    }

    for(i=0; i<1; ++i){
      callback new_callback;
      new_callback.callback_function = NULL;
      new_callback.midi_callback_function = NULL;
      new_callback.instance = NULL;
      midi_notein_callbacks[i] = new_callback;
    }
  }
}

static void start_the_clock(){
  // First time here. Start the clock.
  if(beaglebone_number_of_midi_instances==0 && beaglebone_number_of_pruio_instances==0){
    beaglebone_clock = clock_new(NULL,  (t_method)beaglebone_clock_tick); 
    clock_delay(beaglebone_clock, CLOCK_PERIOD);
  }
}


int beaglebone_register_callback(
      beaglebone_callback_type type, 
      int channel, 
      void* instance, 
      void (*callback_function)(void*, t_float) 
){
   check_if_first_time(); 
   
   // If there's already a callback for this channel, bail out.
   if(type == BB_GPIO_DIGITAL){
      if(digital_callbacks[channel].instance!=NULL){
         return 1;    
      }
   }
   else if(type == BB_GPIO_ANALOG){
      if(analog_callbacks[channel].instance!=NULL){
         return 1;    
      }
   }

   // All good, register the callback
   callback* new_callback = NULL;
   if(type == BB_GPIO_DIGITAL){
      new_callback = &(digital_callbacks[channel]);
   }
   else if(type == BB_GPIO_ANALOG){
      new_callback = &(analog_callbacks[channel]);
   }
   new_callback->callback_function = callback_function;
   new_callback->instance = instance;

   beaglebone_number_of_pruio_instances++;

   return 0;
}

int beaglebone_register_midi_callback(
    beaglebone_callback_type type, 
    int channel, 
    void* instance, 
    void (*callback_function)(void*, beaglebone_midi_message*)
){
  check_if_first_time();

  if(type == BB_MIDI_NOTE){
    if(midi_notein_callbacks[0].instance!=NULL){
      return 1;    
    }

    callback* new_callback = &(midi_notein_callbacks[0]);
    printf("yep\n");

    printf("cbk %p %p\n", new_callback, callback_function);
    new_callback->midi_callback_function = callback_function;
    printf("cbk %p\n", midi_notein_callbacks[0].midi_callback_function);
    new_callback->instance = instance;

    start_the_clock();

    beaglebone_number_of_midi_instances++;
  }

  return 0;

  (void)channel;
}

void beaglebone_unregister_callback(beaglebone_callback_type type, int channel){
   // TODO uninit gpio or analog pin?
   
   callback *cbk = NULL;
   if(type == BB_GPIO_DIGITAL){
      cbk = &(digital_callbacks[channel]);
      beaglebone_number_of_pruio_instances--;
   }
   else if(type == BB_GPIO_ANALOG){
      cbk = &(analog_callbacks[channel]);
      beaglebone_number_of_pruio_instances--;
   }
   else if(type == BB_MIDI_NOTE){
      cbk = &(midi_notein_callbacks[0]);
      beaglebone_number_of_midi_instances--;
   }
   else{ // BB_MIDI_CONTROL
      //TODO
   }

   cbk->callback_function = NULL;
   cbk->instance = NULL;

   if(beaglebone_number_of_pruio_instances==0 && beaglebone_number_of_midi_instances==0){
      clock_free(beaglebone_clock);
      beaglebone_clock = NULL;
      #ifdef IS_BEAGLEBONE
         beaglebone_pruio_stop();
         beaglebone_midi_stop();
      #endif
   }
}
