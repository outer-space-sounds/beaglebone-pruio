/* Beaglebone Pru IO 
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

#ifndef BEAGLEBONE_PRUIO_H
#define BEAGLEBONE_PRUIO_H

#include <string.h>
#include <stdint.h>

/**
 * A structure for easy reading of incoming messages.
 */
typedef struct beaglebone_pruio_message{
   int is_gpio; // 1 if gpio, 0 if adc
   int value;
   int adc_channel;
   int gpio_number;
} beaglebone_pruio_message;

typedef enum{  
   BEAGLEBONE_PRUIO_GPIO_MODE_OUTPUT = 0,
   BEAGLEBONE_PRUIO_GPIO_MODE_INPUT = 1
} beaglebone_pruio_gpio_mode;

typedef enum{  
   BEAGLEBONE_PRUIO_ADC_MODE_OFF = 0,
   BEAGLEBONE_PRUIO_ADC_MODE_NORMAL = 1,
   BEAGLEBONE_PRUIO_ADC_MODE_RANGES = 2,
} beaglebone_pruio_adc_mode;

/**
 * Initializes PRU, GPIO and ADC hardware and starts sampling ADC channels.
 */
int beaglebone_pruio_start();

/**
 * Stops PRU and ADC hardware, no more samples are aquired.
 */
int beaglebone_pruio_stop();

/**
 * Configures how a GPIO channel is used
 */
int beaglebone_pruio_init_gpio_pin(int gpio_number, beaglebone_pruio_gpio_mode mode);  

/**
 * Sets the value of an output pin (0 or 1)
 */
void beaglebone_pruio_set_pin_value(int gpio_number, int value);

/**
 * Starts reading from an ADC pin.
 */
int beaglebone_pruio_init_adc_pin(int channel_number, int bits); 

/**
 * Starts reading from an ADC pin. The analog input range is broken
 * into n ranges. Useful, for example, for selecting one of n numbers
 * with a potentiometer.
 */
int beaglebone_pruio_init_adc_pin_with_ranges(int channel_number, int ranges); 

/**
 * Returns 1 if there is data available from the PRU
 */
static inline int beaglebone_pruio_messages_are_available();

/**
 * Puts the next message available from the PRU in the message address.
 */
static inline void beaglebone_pruio_read_message(beaglebone_pruio_message *message);

/**
 * Loads a DTO
 */
int beaglebone_pruio_load_device_tree_overlay(char* dto);

/**
 * Init MIDI port
 */
int beaglebone_midi_start();

/**
 * Close MIDI port
 */
void beaglebone_midi_stop();

/**
 * A structure for easy reading of MIDI messages
 */

typedef enum{  
   BEAGLEBONE_MIDI_UNKNOWN = 0x0,
   BEAGLEBONE_MIDI_NOTE_OFF = 0x8,
   BEAGLEBONE_MIDI_NOTE_ON = 0x9,
   BEAGLEBONE_MIDI_CONTROL_CHANGE = 0xB,
   BEAGLEBONE_MIDI_PROGRAM_CHANGE = 0xC,
   BEAGLEBONE_MIDI_PITCH_BEND = 0xE,
} beaglebone_midi_message_type;

#define BEAGLEBONE_MIDI_MAX_MESSAGE_SIZE 3

typedef struct { 
   beaglebone_midi_message_type type;
   int channel;
   int size;
   uint8_t data[BEAGLEBONE_MIDI_MAX_MESSAGE_SIZE];
} beaglebone_midi_message;

/**
 * Get MIDI messages
 */
void beaglebone_midi_receive_messages(beaglebone_midi_message* messages, int* count);

/**
 * Send MIDI messages
 */
void beaglebone_midi_send_messages(beaglebone_midi_message* messages, int count);

///////////////////////////////////////////////////////////////////////////////
// !!!
// Not safe to use anything below this line from client code.
// We've put it here just so the compiler can inline it.
///////////////////////////////////////////////////////////////////////////////


// Communication with PRU is done througn a ring buffer in the
// PRU shared memory area.
// shared_ram[0] to shared_ram[1023] is the buffer data.
// shared_ram[1024] is the start (read) pointer.
// shared_ram[1025] is the end (write) pointer.
//
// Messages are 32 bit unsigned ints.
// 
// Read these:
// * http://en.wikipedia.org/wiki/Circular_buffer#Mirroring
// * https://groups.google.com/forum/#!category-topic/beagleboard/F9JI8_vQ-mE

volatile unsigned int *beaglebone_pruio_shared_ram;
unsigned int beaglebone_pruio_buffer_size;
volatile unsigned int *beaglebone_pruio_buffer_start;
volatile unsigned int *beaglebone_pruio_buffer_end;

static inline __attribute__ ((always_inline)) int beaglebone_pruio_messages_are_available(){
   return (*beaglebone_pruio_buffer_start != *beaglebone_pruio_buffer_end);
}

static inline __attribute__ ((always_inline)) void beaglebone_pruio_read_message(beaglebone_pruio_message* message){
   unsigned int raw_message = beaglebone_pruio_shared_ram[*beaglebone_pruio_buffer_start & (beaglebone_pruio_buffer_size-1)];

   message->is_gpio = (raw_message&(1<<31))==0;
   if(message->is_gpio){
      message->value = (raw_message&(1<<8))==0;
      message->gpio_number = raw_message & 0xFF;
   }
   else{
      message->value = (raw_message >> 4) & 0xFFF; 
      message->adc_channel = raw_message & 0xF;
   }

   // Don't write buffer start before reading message (mem barrier)
   // http://stackoverflow.com/questions/982129/what-does-sync-synchronize-do
   // https://en.wikipedia.org/wiki/Memory_ordering#Compiler_memory_barrier
   __sync_synchronize();

   // Increment buffer start, wrap around 2*size
   *beaglebone_pruio_buffer_start = (*beaglebone_pruio_buffer_start+1) & (2*beaglebone_pruio_buffer_size - 1);
}

#endif // BEAGLEBONE_PRUIO_H
