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

/**
 * README:
 * Polling of the midi port is done using the callback mechanism in 
 * beaglebone.c. All of the initialization of the uart is done here in 
 * init_midi()
 */

#include <m_pd.h>
#include <stdio.h>
#include <stdint.h> 
#include <fcntl.h>
#include <errno.h>
#include <stropts.h>
#include <sys/signal.h>
#include <asm/termios.h>

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/* #include <beaglebone_pruio_pins.h> */
#ifdef IS_BEAGLEBONE
   #include <beaglebone_pruio.h>
#endif 

/* #include "beaglebone.h" */

/////////////////////////////////////////////////////////////////////////
// MIDI_IN
//

static int midi_in_num_instances = 0;

typedef struct midi_in {
   t_object x_obj;
   t_outlet *outlet;
   t_clock* clock;
   beaglebone_midi_message incoming_messages[128];
} t_midi_in;

t_class *midi_in_class;

void midi_in_tick(void* x){
  t_midi_in* this = (t_midi_in*)x;
  int n = 0;
  int i = 0;
  t_atom output[3];

  beaglebone_midi_receive_messages(this->incoming_messages, &n);
  for(i=0; i<n; ++i){
    switch(this->incoming_messages[i].type){
      case BEAGLEBONE_MIDI_NOTE_ON: 
      case BEAGLEBONE_MIDI_NOTE_OFF:
        {
          SETFLOAT(&output[0], this->incoming_messages[i].data[1]);
          SETFLOAT(&output[1], this->incoming_messages[i].data[2]);
          SETFLOAT(&output[2], this->incoming_messages[i].channel);
          outlet_anything(this->outlet, gensym("note"), 3, &output[0]);
        }
        break;

      default: 
        break;
    }
  }

  clock_delay(this->clock, 0.76);
}

// Constructor
//
static void* midi_in_new() {
   if(midi_in_num_instances>0){
     error("beaglebone/midi_in: Only one instance of this object is allowed");
     return NULL;
   }
   if(beaglebone_midi_start()){
     error("beaglebone/midi_in: Could not init midi port (UART)");
     return NULL;
   }

   t_midi_in *x = (t_midi_in *)pd_new(midi_in_class);
   x->outlet = outlet_new(&x->x_obj, &s_list);

   x->clock = clock_new(x, (t_method)midi_in_tick); 
   clock_delay(x->clock, 0.8);

   midi_in_num_instances++;
   return (void *)x;
}

// Destructor
static void midi_in_free(t_midi_in* x) { 
  (void)x;
  beaglebone_midi_stop();
  midi_in_num_instances--;
}

// Class definition
void midi_in_setup(void){
   midi_in_class = class_new(
      gensym("midi_in"), 
      (t_newmethod)midi_in_new, 
      (t_method)midi_in_free, 
      sizeof(t_midi_in), 
      CLASS_NOINLET, 
      (t_atomtype)0
   );
}
