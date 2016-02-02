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

/* #include <beaglebone_pruio_pins.h> */
#ifdef IS_BEAGLEBONE
   #include <beaglebone_pruio.h>
#endif 

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct midi_notein {
   t_object x_obj;
   t_outlet *outlet_left;
   t_outlet *outlet_middle;
   t_outlet *outlet_right;
   beaglebone_midi_message incoming_message;
} t_midi_notein;

// A pointer to the class object.
t_class *midi_notein_class;

/////////////////////////////////////////////////////////////////////////
// Callback from lib beaglebone_pruio
//

void midi_notein_callback(void* x, beaglebone_midi_message* message){
   (void)(x);
   t_midi_notein* this = (t_midi_notein*)x;
   outlet_float(this->outlet_right, (float)message->channel);
   outlet_float(this->outlet_middle, (float)message->data[2]);
   outlet_float(this->outlet_left, (float)message->data[1]);
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *midi_notein_new() {
   t_midi_notein *x = (t_midi_notein *)pd_new(midi_notein_class);
   x->outlet_left = outlet_new(&x->x_obj, &s_float);
   x->outlet_middle = outlet_new(&x->x_obj, &s_float);
   x->outlet_right = outlet_new(&x->x_obj, &s_float);

   if(beaglebone_register_midi_callback(BB_MIDI_NOTE, 0, x, midi_notein_callback)){
     error("beaglebone/midi_notein: Could not register for receiving midi note events."); 
     return NULL;
   }

   return (void *)x;
}

static void midi_notein_free(t_midi_notein* x) { 
  (void)x;
  beaglebone_unregister_callback(BB_MIDI_NOTE, 0);
}

/////////////////////////////////////////////////////////////////////////
// Class definition
// 

void midi_notein_setup(void){
   midi_notein_class = class_new(
      gensym("midi_notein"), 
      (t_newmethod)midi_notein_new, 
      (t_method)midi_notein_free, 
      sizeof(t_midi_notein), 
      CLASS_NOINLET, 
      (t_atomtype)0
   );
}
