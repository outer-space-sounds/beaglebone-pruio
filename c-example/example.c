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

/////////////////////////////////////////////////////////////////////
// Library code (TODO: move to library)
//

#include <stdint.h> 
#include <unistd.h> 
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stropts.h>
#include <sys/signal.h>
#include <asm/termios.h>

// Privates
#define BEAGLEBONE_MIDI_BUFFER_SIZE 128
#define BEAGLEBONE_MIDI_UART_NUMBER 4
int uart;
uint8_t uart_buffer[BEAGLEBONE_MIDI_BUFFER_SIZE];

// TODO: theres already a library function that does this.
static int load_device_tree_overlay_midi(){
  char dto[9];
  sprintf(dto, "BB-UART%i", BEAGLEBONE_MIDI_UART_NUMBER);

  // Check if the device tree overlay is loaded.
  int device_tree_overlay_loaded = 0; 
  FILE* f;
  f = fopen("/sys/devices/bone_capemgr.9/slots","rt");
  if(f==NULL){
    return 1;
  }
  char line[256];
  while(fgets(line, 256, f) != NULL){
    if(strstr(line, dto) != NULL){
      device_tree_overlay_loaded = 1; 
    }
  }
  fclose(f);

  // Load if needed
  if(!device_tree_overlay_loaded){
    f = fopen("/sys/devices/bone_capemgr.9/slots","w");
    if(f==NULL){
      return 1;
    }
    fprintf(f, "%s", dto);
    fclose(f);
  }

  usleep(100000);

  return 0;
}

static int init_uart(){
  char path[11];
  sprintf(path, "/dev/ttyO%i", BEAGLEBONE_MIDI_UART_NUMBER);

  uart = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK); 
  if(uart < 0){
    perror(path);
    return errno;
  }

  struct termios2 uart_parameters;
  if(ioctl(uart, TCGETS2, &uart_parameters) < 0){
    perror(path);
    return errno;
  }

  // Enable receiver
  uart_parameters.c_iflag |= CREAD;

  // Enable interrupt signals
  /* fcntl(uart, F_SETOWN, getpid()); */
  /* fcntl(uart, F_SETFL, FASYNC); */

  // Custom baud rate 31250.
  // http://stackoverflow.com/questions/12646324/how-to-set-a-custom-baud-rate-on-linux
  uart_parameters.c_cflag &= ~CBAUD;
  uart_parameters.c_cflag |= BOTHER;
  uart_parameters.c_ispeed = 31250;
  uart_parameters.c_ospeed = 31250;

  // Raw mode, no parity, 8 bits
  // http://linux.die.net/man/3/termios
  uart_parameters.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  uart_parameters.c_oflag &= ~OPOST;
  uart_parameters.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  uart_parameters.c_cflag &= ~(CSIZE | PARENB);
  uart_parameters.c_cflag |= CS8;

  // Read 1 byte at a time, no wait.
  uart_parameters.c_cc[VTIME] = 0;
  uart_parameters.c_cc[VMIN] = 1;

  if(ioctl(uart, TCSETS2, &uart_parameters)){
    perror(path);
    return errno;
  }
  return 0;
}


// Publics

typedef enum{  
   BEAGLEBONE_MIDI_NOTE_OFF = 0x8,
   BEAGLEBONE_MIDI_NOTE_ON = 0x9,
   BEAGLEBONE_MIDI_CONTROL_CHANGE = 0xB,
   BEAGLEBONE_MIDI_PROGRAM_CHANGE = 0xC,
   BEAGLEBONE_MIDI_PITCH_BEND = 0xE,
} beaglebone_midi_message_type;

typedef struct { 
   beaglebone_midi_message_type type;
   uint8_t channel;
   uint8_t data1;
   uint8_t data2;
} beaglebone_midi_message;

void beaglebone_midi_stop(){
  close(uart);
}

int beaglebone_midi_start(){
  if(load_device_tree_overlay_midi()){
    fprintf(stderr, "libbeaglebone_pruio: Could not load device tree overlay for MIDI port (UART).\n");
    return 1;
  }

  int e = init_uart();
  if(e){
    fprintf(stderr, "libbeaglebone_pruio: Could not initialize MIDI port (UART). %i\n", e);
    return 1;
  }

  return 0;
}

uint8_t midi_read_state = 0; //0 is idle, 1 is first byte, 2 is second byte, 3 is third byte
beaglebone_midi_message current_message;


inline void beaglebone_midi_read(beaglebone_midi_message* messages, int* num_messages){
  int n = read(uart, uart_buffer, BEAGLEBONE_MIDI_BUFFER_SIZE);

  *num_messages = 0;
  int i = 0;
  uint8_t byte; 
  for(i=0; i<n; ++i){
    byte = uart_buffer[i];

    // If this is a status byte (most significant bit is 1)
    if((byte & 0x80) == 0x80){
      midi_read_state = 1;
      switch((byte & 0xf0)>>4){
        case BEAGLEBONE_MIDI_NOTE_OFF:
          current_message.type = BEAGLEBONE_MIDI_NOTE_OFF;
          break;
        case BEAGLEBONE_MIDI_NOTE_ON:
          current_message.type = BEAGLEBONE_MIDI_NOTE_ON;
          break;
        case BEAGLEBONE_MIDI_CONTROL_CHANGE:
          current_message.type = BEAGLEBONE_MIDI_CONTROL_CHANGE;
          break;
        case BEAGLEBONE_MIDI_PROGRAM_CHANGE:
          current_message.type = BEAGLEBONE_MIDI_PROGRAM_CHANGE;
          break;
        case BEAGLEBONE_MIDI_PITCH_BEND:
          current_message.type = BEAGLEBONE_MIDI_PITCH_BEND;
          break;
      } 
      current_message.channel = byte & 0x0F;
    }
    else if(midi_read_state==1){
      current_message.data1 = byte;

      // Program change messages are 2 bytes long
      if(current_message.type == BEAGLEBONE_MIDI_PROGRAM_CHANGE){
        current_message.data2 = 0;
        messages[*num_messages] = current_message;
        midi_read_state = 0;
        (*num_messages)++;
      }
      // Other messages are 3 bytes long
      else{
        midi_read_state = 2;
      }
    }
    else if(midi_read_state==2){
      current_message.data2 = byte;
      messages[*num_messages] = current_message;
      midi_read_state = 0;
      (*num_messages)++;
    }
  }
}

/* inline void beaglebone_midi_write(beaglebone_midi_message* messages, int num_messages){ */
inline void beaglebone_midi_write(uint8_t* messages, int num_messages){
  write(uart, messages, num_messages);
}

/////////////////////////////////////////////////////////////////////

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
         /* if(message.is_gpio && message.gpio_number==P9_11){ */
            /* printf("P9_11: %i\n", message.value); */
         /* } */
         /* else if(message.is_gpio && message.gpio_number==P9_13){ */
            /* printf("P9_13: %i\n", message.value); */
         /* } */

         // Messages from adc
         if(!message.is_gpio && message.adc_channel==0){
            /* printf("ADC 0: %i\n", message.value); */
         }
         else if(!message.is_gpio && message.adc_channel==6){
            /* printf("ADC 6: %i\n", message.value); */
         }
      }
      
      // Midi messages
      beaglebone_midi_read(&(midi_messages[0]), &n_midi_messages);
      int i;
      for(i=0; i<n_midi_messages; ++i){
        switch(midi_messages[i].type){
          case BEAGLEBONE_MIDI_NOTE_ON: printf("Note on\n"); break;
          case BEAGLEBONE_MIDI_NOTE_OFF: printf("Note off\n"); break;
          case BEAGLEBONE_MIDI_PITCH_BEND: printf("Pitch bend\n"); break;
          default: break;
          // etc.
        }
        printf("Channel: %i \n", midi_messages[i].channel);
        printf("Data 1: %i \n", midi_messages[i].data1);
        printf("Data 2: %i \n\n", midi_messages[i].data2);
      } 

      usleep(10000);
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
   /* pthread_cancel(monitor_thread); */
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

   // Initialize 2 pins as outputs
   if(beaglebone_pruio_init_gpio_pin(P9_16, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT)){
      fprintf(stderr, "%s\n", "Could not initialize pin P9_12");
   }
   if(beaglebone_pruio_init_gpio_pin(P9_18, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT)){
      fprintf(stderr, "%s\n", "Could not initialize pin P9_14");
   }

   // Init 2 pins as inputs
   /* if(beaglebone_pruio_init_gpio_pin(P9_13, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT)){ */
   /*    fprintf(stderr, "%s\n", "Could not initialize pin P9_13"); */
   /* } */
   /* if(beaglebone_pruio_init_gpio_pin(P9_11, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT)){ */
   /*    fprintf(stderr, "%s\n", "Could not initialize pin P9_11"); */
   /* } */

   // Init 2 analog inputs
   if(beaglebone_pruio_init_adc_pin_with_ranges(0, 12)){
      fprintf(stderr, "%s\n", "Could not initialize adc pin 0");
   }
   if(beaglebone_pruio_init_adc_pin(6, 7)){
      fprintf(stderr, "%s\n", "Could not initialize adc pin 6");
   }

   // Check if library is returning adequately when trying to 
   // re-initialize a pin.
   if(!beaglebone_pruio_init_gpio_pin(P9_16, BEAGLEBONE_PRUIO_GPIO_MODE_INPUT)){
      fprintf(stderr, "%s\n", "P9_16 was already initialized, should have returned error");
      exit(1);
   }
   if(beaglebone_pruio_init_gpio_pin(P9_18, BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT)){
      fprintf(stderr, "%s\n", "P9_18 was already initialized as output, should have not returned error");
      exit(1);
   }

   // Blink 2 outputs
   uint8_t messages[3] = {0x90, 0x06, 0x08};
   while(!finished){
      beaglebone_pruio_set_pin_value(P9_16, 0);
      beaglebone_pruio_set_pin_value(P9_18, 1);
      beaglebone_midi_write(messages, 3);
      sleep(3);
      beaglebone_pruio_set_pin_value(P9_16, 1);
      beaglebone_pruio_set_pin_value(P9_18, 0);
      beaglebone_midi_write(messages, 3);
      sleep(3);

   }

   sleep(3);
   beaglebone_pruio_stop();
   stop_monitor_thread();
   beaglebone_midi_stop();

   return 0;
}
