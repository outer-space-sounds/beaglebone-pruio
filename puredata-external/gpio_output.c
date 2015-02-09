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

#include <string.h>
#include <m_pd.h>
#include <beaglebone_pruio_pins.h>

#ifdef IS_BEAGLEBONE
#include <beaglebone_pruio.h>
#endif

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct gpio_output {
   t_object x_obj;
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
      beaglebone_pruio_set_pin_value(x->gpio_number, (int)f);
   #else
      (void)x;
   #endif 
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *gpio_output_new(t_symbol *s) {

   int gpio_number = beaglebone_pruio_get_gpio_number(s->s_name);
   if(gpio_number == -1){
      error("beaglebone/gpio_output: Could not init pin %s (%i).", 
            s->s_name, 
            gpio_number
           );
      return NULL;
   }

   #ifdef IS_BEAGLEBONE
      if(beaglebone_pruio_init_gpio_pin(gpio_number, 0)){   // 0 for output
         error("beaglebone/gpio_output: Could not init pin %s (%i).", 
               s->s_name, 
               gpio_number
         );
         return NULL;
      }
   #endif

   t_gpio_output *x = (t_gpio_output *)pd_new(gpio_output_class);
   x->gpio_number = gpio_number;

   return (void *)x;
}

static void gpio_output_free(t_gpio_output *x) { 
   // TODO: Uninit pin?
   (void)x;
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
