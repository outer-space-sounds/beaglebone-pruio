/////////////////////////////////////////////////////////////////////////
// 
//  Lib BBB Pruio
//  Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by 
//  the Free Software Foundation, either version 3 of the License, or (at
//  your option) any later version.
//  
//  This program is distributed in the hope that it will be useful, but 
//  WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.  
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
/////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <m_pd.h>
#include <bbb_pruio_pins.h>

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct gpio_output {
   t_object x_obj;
   char channel[6];
   int gpio_number;
} t_gpio_output;

// A pointer to the class object.
t_class *gpio_output_class;

/////////////////////////////////////////////////////////////////////////
// Float Message received
//

void gpio_output_float(t_gpio_output* x, t_floatarg f){
   if(f!=0 && f!=1){
      error("beaglebone/gpio_output: %f is not a valid output value, only 0 and 1 allowed.", f);
      return;
   }

   #ifdef IS_BEAGLEBONE
      //TODO
   #else
      (void)x;
   #endif 
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *gpio_output_new(t_symbol *s) {
   t_gpio_output *x = (t_gpio_output *)pd_new(gpio_output_class);

   strncpy(x->channel, s->s_name, 5);
   x->channel[5] = '\0';

   x->gpio_number = bbb_pruio_get_gpio_number(x->channel);

   #ifdef IS_BEAGLEBONE
      /* char err[256]; */
      /* if(beaglebone_clock_new(1, &x->channel[0], x, gpio_output_callback, &err[0])){ */
      /*    error("beaglebone/gpio_output: %s", err);  */
      /*    return NULL; */
      /* } */
   #else
   #endif

   return (void *)x;
}

static void gpio_output_free(t_gpio_output *x) { 
   (void)x;
   #ifdef IS_BEAGLEBONE
      //TODO
   #endif 
}

/////////////////////////////////////////////////////////////////////////
// Class definition
// 

void gpio_output_setup(void) {
   gpio_output_class = class_new(
      gensym("gpio_output"), 
      (t_newmethod)gpio_output_new, 
      (t_method)gpio_output_free, 
      sizeof(t_gpio_output), 
      CLASS_DEFAULT, 
      A_DEFSYMBOL,
      (t_atomtype)0
   );

   // Trigger gpio_output_float when a float message is received
   class_addfloat(gpio_output_class, gpio_output_float);
}
