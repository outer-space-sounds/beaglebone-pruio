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

typedef struct gpio_input {
   t_object x_obj;
   t_outlet *outlet_left;
   int gpio_number;
} t_gpio_input;

// A pointer to the class object.
t_class *gpio_input_class;

/////////////////////////////////////////////////////////////////////////
// Callback from lib beaglebone_pruio
//

void gpio_input_callback(void* x, float value){
   t_gpio_input* this = (t_gpio_input*)x;
   outlet_float(this->outlet_left, (float)value);
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *gpio_input_new(t_symbol *s) {
   int gpio_number = beaglebone_pruio_get_gpio_number(s->s_name);
   if(gpio_number==-1){
      error("beaglebone/gpio_input: %s is not a valid GPIO pin.", s->s_name);
      return NULL;
   }

   #ifdef IS_BEAGLEBONE
      if(beaglebone_pruio_init_gpio_pin(gpio_number, 1)){  // 1 for input
         error("beaglebone/gpio_input: Could not init pin %s (%i), is it already in use?", s->s_name, gpio_number); 
         return NULL;
      }
   #endif 

   t_gpio_input *x = (t_gpio_input *)pd_new(gpio_input_class);
   x->outlet_left = outlet_new(&x->x_obj, &s_float);
   x->gpio_number = gpio_number;

   if(beaglebone_register_callback(BB_GPIO_DIGITAL, gpio_number, x, gpio_input_callback)){
      error("beaglebone/gpio_input: Could not init pin %s (%i), is it already in use?", s->s_name, gpio_number); 
      return NULL;
   }

   return (void *)x;
}

static void gpio_input_free(t_gpio_input *x) { 
   beaglebone_unregister_callback(BB_GPIO_DIGITAL, x->gpio_number);
}

/////////////////////////////////////////////////////////////////////////
// Class definition
// 

void gpio_input_setup(void) {
   gpio_input_class = class_new(
      gensym("gpio_input"), 
      (t_newmethod)gpio_input_new, 
      (t_method)gpio_input_free, 
      sizeof(t_gpio_input), 
      CLASS_NOINLET, 
      A_DEFSYMBOL,
      (t_atomtype)0
   );
}
