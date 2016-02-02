#include "beaglebone_pruio.h"

#include <stdint.h> 
#include <unistd.h> 
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stropts.h>
/* #include <sys/signal.h> */
#include <asm/termios.h>

#define BEAGLEBONE_MIDI_BUFFER_SIZE 128
#define BEAGLEBONE_MIDI_UART_NUMBER 4

static int uart;
static uint8_t uart_buffer[BEAGLEBONE_MIDI_BUFFER_SIZE];
static int read_counter = 0;
static beaglebone_midi_message current_message;

extern int beaglebone_midi_start(){
  char dto[9];
  sprintf(dto, "BB-UART%i", BEAGLEBONE_MIDI_UART_NUMBER);
  if(beaglebone_pruio_load_device_tree_overlay(dto)){
    fprintf(stderr, "libbeaglebone_pruio: Could not load device tree overlay for MIDI port (UART).\n");
    return 1;
  }
  
  char path[11];
  sprintf(path, "/dev/ttyO%i", BEAGLEBONE_MIDI_UART_NUMBER);
  
  uart = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK); 
  if(uart < 0){
    fprintf(stderr, "libbeaglebone_pruio: Could not open UART file descriptor.");
    return 1;
  }
  
  struct termios2 uart_parameters;
  if(ioctl(uart, TCGETS2, &uart_parameters) < 0){
    fprintf(stderr, "libbeaglebone_pruio: Could not get default UART parameters.\n");
    return 1;
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
    fprintf(stderr, "libbeaglebone_pruio: Could not set custom UART parameters.\n");
    return 1;
  }
  
  return 0;
}

extern void beaglebone_midi_stop(){
  close(uart);
}

void beaglebone_midi_receive_messages(beaglebone_midi_message* messages, int* num_messages){

  int n = read(uart, uart_buffer, BEAGLEBONE_MIDI_BUFFER_SIZE);

  *num_messages = 0;
  int i = 0;
  uint8_t byte; 
  for(i=0; i<n; ++i){
    byte = uart_buffer[i];

    // If this is a status byte (most significant bit is 1)
    if((byte & 0x80) == 0x80){
      read_counter = 0;
      switch((byte & 0xf0)>>4){
        case BEAGLEBONE_MIDI_NOTE_OFF:
          current_message.type = BEAGLEBONE_MIDI_NOTE_OFF;
          current_message.size = 3;
          break;
        case BEAGLEBONE_MIDI_NOTE_ON:
          current_message.type = BEAGLEBONE_MIDI_NOTE_ON;
          current_message.size = 3;
          break;
        case BEAGLEBONE_MIDI_CONTROL_CHANGE:
          current_message.type = BEAGLEBONE_MIDI_CONTROL_CHANGE;
          current_message.size = 3;
          break;
        case BEAGLEBONE_MIDI_PROGRAM_CHANGE:
          current_message.type = BEAGLEBONE_MIDI_PROGRAM_CHANGE;
          current_message.size = 2;
          break;
        case BEAGLEBONE_MIDI_PITCH_BEND:
          current_message.type = BEAGLEBONE_MIDI_PITCH_BEND;
          current_message.size = 3;
          break;
      } 
      current_message.data[0] = byte;
      current_message.channel = byte & 0x0F;
    }
    else{
      read_counter++;
      if(read_counter >= BEAGLEBONE_MIDI_MAX_MESSAGE_SIZE) read_counter=0;

      current_message.data[read_counter] = byte;

      if(current_message.size-1 == read_counter){
        messages[*num_messages] = current_message;
        read_counter = 0;
        (*num_messages)++;
      }
    }
  }
}

void beaglebone_midi_send_messages(beaglebone_midi_message* messages, int count){
  /* write(uart, data, size); */
}
