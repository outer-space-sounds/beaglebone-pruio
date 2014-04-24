/*
 * Lib BBB Pruio
 * Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
 * Copyright (C) 2014 Miguel Vargas <miguelito.vargasf@gmail.com>
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
#include <signal.h>
#include <unistd.h>

#include <bbb_pruio.h>

// Attention: callback functions are invoked
// from another thread, use appropriate locks.
void callback_pin1(unsigned int value, unsigned int raw_value){
   printf("Pin 1 value: %u \n", value);
}

void callback_pin2(unsigned int value, unsigned int raw_value){
   printf("Pin 2 value: %u \n", value);
}

unsigned int finished = 0;
void signal_handler(int signal){
   finished = 1;
}

int main(int argc, const char *argv[]){
   // Listen to SIGINT signals (program termination)
   signal(SIGINT, signal_handler);

   bbb_pruio_init_adc();

   bbb_pruio_init_adc_pin(1, 0, callback_pin1);
   bbb_pruio_init_adc_pin(2, 8, callback_pin2);

   while(!finished){
      sleep(1); 
   }

   bbb_pruio_close_adc();
   return 0;
}
