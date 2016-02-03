   /* 
    * Beaglebone Pru IO 
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <beaglebone_pruio.h>
#include <beaglebone_pruio_pins.h>


unsigned int finished = 0;
void signal_handler(int signal){
   finished = 1;
}

/////////////////////////////////////////////////////////////////////
static pthread_t monitor_thread;

static void* monitor_inputs(void* param){
   beaglebone_pruio_message message;
   beaglebone_midi_message midi_messages[16];
   int n_midi_messages = 0;

   while(!finished){
      while(beaglebone_pruio_messages_are_available() && !finished){
         beaglebone_pruio_read_message(&message);

         // Message from gpio
         if(message.is_gpio){
            printf("GPIO %i: %i\n", message.gpio_number, message.value);
         }

         // Messages from adc
         if(!message.is_gpio){
            printf("ADC %i: %i\n", message.adc_channel, message.value);
         }
      }
      
      // Midi messages
      beaglebone_midi_receive_messages(midi_messages, &n_midi_messages);
      int i;
      for(i=0; i<n_midi_messages; ++i){
        switch(midi_messages[i].type){
          case BEAGLEBONE_MIDI_NOTE_ON: printf("Note on\n"); break;
          case BEAGLEBONE_MIDI_NOTE_OFF: printf("Note off\n"); break;
          /* case BEAGLEBONE_MIDI_PITCH_BEND: printf("Pitch bend\n"); break; */
          default: continue; break;
          // etc.
        }
        printf("Channel: %i \n", midi_messages[i].channel);
        printf("Data 1: %i \n", midi_messages[i].data[1]);
        printf("Data 2: %i \n\n", midi_messages[i].data[2]);
      } 

      usleep(1000);
   }

   return NULL;
}

static int start_monitor_thread(){
   // TODO: set real time priority to this thread
   pthread_attr_t attr;
   if(pthread_attr_init(&attr)){
      return 1;
   }
   if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)){
      return 1;
   }
   if(pthread_create(&monitor_thread, &attr, &monitor_inputs, NULL)){
      return 1;
   }

   return 0;
}

static void stop_monitor_thread(){
   pthread_cancel(monitor_thread);
}

int main(int argc, const char *argv[]){
   // Listen to SIGINT signals (program termination)
   signal(SIGINT, signal_handler);

   if(beaglebone_midi_start()){
     return 1;
   }
   if(beaglebone_pruio_start()){
     beaglebone_midi_stop();
     return 1;
   }
   if(start_monitor_thread()){
     beaglebone_midi_stop();
     beaglebone_pruio_stop();
     return 1;
   }

   // Initialize pins as outputs
   beaglebone_pruio_init_gpio_pin(P9_12, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);
   beaglebone_pruio_init_gpio_pin(P9_14, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);
   beaglebone_pruio_init_gpio_pin(P9_15, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);
   beaglebone_pruio_init_gpio_pin(P9_21, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);
   beaglebone_pruio_init_gpio_pin(P9_24, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);
   beaglebone_pruio_init_gpio_pin(P9_22, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);
   beaglebone_pruio_init_gpio_pin(P9_23, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT);

   // Init pins as inputs
   beaglebone_pruio_init_gpio_pin(P8_07, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_07, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_08, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_09, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P9_16, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P9_27, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_45, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_30, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_32, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P9_26, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P9_41A, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_44, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_41, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_42, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_39, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_40, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_37, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_38, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_35, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_36, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_33, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_34, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_31, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_18, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_43, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);
   beaglebone_pruio_init_gpio_pin(P8_46, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT);

   // Init analog pins
   beaglebone_pruio_init_adc_pin(1, 8);
   beaglebone_pruio_init_adc_pin(2, 8);
   beaglebone_pruio_init_adc_pin(3, 8);
   beaglebone_pruio_init_adc_pin(4, 8);
   beaglebone_pruio_init_adc_pin(0, 8);
   beaglebone_pruio_init_adc_pin(10, 8);
   beaglebone_pruio_init_adc_pin(8, 8);
   beaglebone_pruio_init_adc_pin(11, 8);
   beaglebone_pruio_init_adc_pin(9, 8);
   beaglebone_pruio_init_adc_pin(7, 8);
   beaglebone_pruio_init_adc_pin(6, 8);


   // Blink 2 outputs
   /* uint8_t messages[3] = {0x90, 0x06, 0x08}; */
   while(!finished){
     beaglebone_pruio_set_pin_value(P9_12, 0);
     beaglebone_pruio_set_pin_value(P9_14, 0);
     beaglebone_pruio_set_pin_value(P9_15, 0);
     beaglebone_pruio_set_pin_value(P9_21, 0);
     beaglebone_pruio_set_pin_value(P9_24, 0);
     beaglebone_pruio_set_pin_value(P9_22, 0);
     beaglebone_pruio_set_pin_value(P9_23, 0);
     sleep(1);
     beaglebone_pruio_set_pin_value(P9_12, 1);
     beaglebone_pruio_set_pin_value(P9_14, 1);
     beaglebone_pruio_set_pin_value(P9_15, 1);
     beaglebone_pruio_set_pin_value(P9_21, 1);
     beaglebone_pruio_set_pin_value(P9_24, 1);
     beaglebone_pruio_set_pin_value(P9_22, 1);
     beaglebone_pruio_set_pin_value(P9_23, 1);
     sleep(1);
   }

   beaglebone_pruio_stop();
   stop_monitor_thread();
   beaglebone_midi_stop();

   return 0;
}
