/*
 * Lib BBB Pruio
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

/**
 * Initializes PRU, GPIO and ADC hardware and starts sampling ADC channels.
 */
int bbb_pruio_start();

/**
 * Stops PRU and ADC hardware, no more samples are aquired.
 */
int bbb_pruio_stop();

/**
 * Configures how an ADC channel is read
 */
int bbb_pruio_init_adc_pin(unsigned int pin_number); 

/**
 * Returns 1 if there is data available from the PRU
 */
inline int bbb_pruio_messages_are_available();

/**
 * Puts the next message available from the PRU in the message address.
 */
inline void bbb_pruio_read_message(unsigned int *message);

/**
 * Sets the value of an output pin (0 or 1)
 */
inline void bbb_pruio_set_pin_value(int gpio_number, int value);


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
// Messages are 32 bit unsigned ints. The 16 MSbits are the channel 
// number and the 16 LSbits are the value.
// 
// Read these:
// * http://en.wikipedia.org/wiki/Circular_buffer#Mirroring
// * https://groups.google.com/forum/#!category-topic/beagleboard/F9JI8_vQ-mE

volatile unsigned int *bbb_pruio_shared_ram = NULL;
unsigned int bbb_pruio_buffer_size;
volatile unsigned int *bbb_pruio_buffer_start;
volatile unsigned int *bbb_pruio_buffer_end;

inline __attribute__ ((always_inline)) int bbb_pruio_messages_are_available(){
   return (*bbb_pruio_buffer_start != *bbb_pruio_buffer_end);
}

inline __attribute__ ((always_inline)) void bbb_pruio_read_message(unsigned int* message){
   *message = bbb_pruio_shared_ram[*bbb_pruio_buffer_start & (bbb_pruio_buffer_size-1)];

   // Don't write buffer start before reading message (mem barrier)
   // http://stackoverflow.com/questions/982129/what-does-sync-synchronize-do
   // https://en.wikipedia.org/wiki/Memory_ordering#Compiler_memory_barrier
   __sync_synchronize();

   // Increment buffer start, wrap around 2*size
   *bbb_pruio_buffer_start = (*bbb_pruio_buffer_start+1) & (2*bbb_pruio_buffer_size - 1);
}
