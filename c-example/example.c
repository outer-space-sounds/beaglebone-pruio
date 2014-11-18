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

   bbb_pruio_start_adc();

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
   int row_counter = 0;
   int sample_counter = 0;
   int channel_number;
   int value;
   while(!finished && row_counter<15000){
      while(bbb_pruio_messages_are_available()){
         bbb_pruio_read_message(&message);

         channel_number = 0xFFFF & (message >> 16);
         value = message & 0xFFFF;

         data[row_counter][channel_number] = value;

         if(row_counter%100==0 && sample_counter==13){
            printf("%i: ", row_counter);
            for(j=0; j<14; j++) {
               printf("%4X ", data[row_counter][j]);
            }
            printf("\n");
         }

         sample_counter++;
         if(sample_counter > 13){
            sample_counter=0;
            row_counter++;
         }
      }
      usleep(1000); 
   }
   
   for(i=0; i<15000; i++) {
      printf("%i: ", i);
      for(j=0; j<14; j++) {
         printf("%4X ", data[i][j]);
      }
      printf("\n");
   }

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

   bbb_pruio_stop_adc();

   return 0;
}
