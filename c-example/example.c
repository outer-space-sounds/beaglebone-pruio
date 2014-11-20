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
#include <stdlib.h>
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

   bbb_pruio_start();

   // We're getting data for 14 channels at 1500 Samples/sec.
   // Let's get 10 secs of data.
   unsigned int data[15000][14];
   int i,j;
   for(i=0; i<15000; i++) {
      for(j=0; j<14; j++) {
         data[i][j] = 0;
      }
   }

   unsigned int message = 0;
   int row_counter=0;
   int sample_counter=0;
   int channel_number, gpio_number, value;
   /* while(!finished && row_counter<15000){ */
   while(!finished){
      while(bbb_pruio_messages_are_available()){
         bbb_pruio_read_message(&message);

         // Message from gpio
         if((message & (1<<31)) == 0){
            gpio_number = message & 0xFF;
            value = (message>>8) & 1;
            printf("\nDigital: 0x%X %i %i", message, gpio_number, value);
         }
         // Message from adc
         else{
            channel_number = message & 0xF;
            value = (0xFFF0 & message)>>4;

            data[row_counter][channel_number] = value;

            // Print every 1000th row to stdout
            /* if(row_counter%1000==0 && sample_counter==13){ */
            /*    printf("\nAnalog: "); */
            /*    for(j=0; j<14; j++) { */
            /*       printf("%4X ", data[row_counter][j]); */
            /*    } */
            /* } */

            sample_counter++;
            if(sample_counter > 13){
               sample_counter=0;
               row_counter++;
               if(row_counter>14999){
                  row_counter=0;
               }
            }
         }
      }
      usleep(1000); 
   }
   
   // Print everything to stdout
   /* for(i=0; i<15000; i++) { */
   /*    printf("%i: ", i); */
   /*    for(j=0; j<14; j++) { */
   /*       printf("%4X ", data[i][j]); */
   /*    } */
   /*    printf("\n"); */
   /* } */

   // Save everything to out.txt file
   /* FILE* f = fopen("./out.txt", "w"); */
   /* if(f==NULL){ */
   /*    printf("Could not open output file\n"); */
   /*    return 1; */
   /* } */
   /* for(i=0; i<15000; i++) { */
   /*    fprintf(f, "%i: ", i); */
   /*    for(j=0; j<14; j++) { */
   /*       fprintf(f, "%4X ", data[i][j]); */
   /*    } */
   /*    fprintf(f,"\n"); */
   /* } */
   /* fclose(f); */

   bbb_pruio_stop();

   return 0;
}
