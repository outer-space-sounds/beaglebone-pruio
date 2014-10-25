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

unsigned int finished = 0;
void signal_handler(int signal){
   finished = 1;
}

int main(int argc, const char *argv[]){
   // Listen to SIGINT signals (program termination)
   signal(SIGINT, signal_handler);

   bbb_pruio_start_adc();

   unsigned int data[100000];
   unsigned int data_counter = 0;

   unsigned int message;
   while(!finished && data_counter<10000){
      while(bbb_pruio_messages_are_available()){
         bbb_pruio_read_message(&message);
         data[data_counter] = message;
         data_counter++;
      }
      usleep(13000); 
   }


   int i, overruns = 0;
   for(i=0; i<10000; i++){
      if(i>0 && (data[i]-1 != data[i-1])){
         printf("\n%u: %u %u", i, data[i], data[i-1]);   
         printf(" <-- !!!!");
         overruns += (data[i] - data[i-1] - 1);
      }
   }
   printf("\nOverruns: %i\n", overruns);

   bbb_pruio_stop_adc();

   return 0;
}
