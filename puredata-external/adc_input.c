/////////////////////////////////////////////////////////////////////////
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

/* #include <string.h> */
#include <m_pd.h>
#include <stdio.h>

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct adc_input {
   t_object x_obj;
   t_outlet *outlet_left;
   char channel[2];
} t_adc_input;

// A pointer to the class object.
t_class *adc_input_class;

/////////////////////////////////////////////////////////////////////////
// Callback from lib bbb_pruio
//

void adc_input_callback(void* x, float value){
   t_adc_input* this = (t_adc_input*)x;
   outlet_float(this->outlet_left, (float)value);
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *adc_input_new(t_floatarg f){
   if(f<0 || f>99 || (float)((int)f)!=(f)){
      error("beaglebone/adc_input: %f is not a valid ADC channel.", f); 
      return NULL;
   }

   t_adc_input *x = (t_adc_input *)pd_new(adc_input_class);
   x->outlet_left = outlet_new(&x->x_obj, &s_float);

   sprintf(x->channel, "%i", (int)f);
   char err[256];
   if(beaglebone_clock_new(0, x->channel, x, adc_input_callback, err)){
      error("beaglebone/adc_input: %s", err); 
      return NULL;
   }

   return (void *)x;
}

static void adc_input_free(t_adc_input *x) { 
   beaglebone_clock_free(0, x->channel);
   (void)x;
}

/////////////////////////////////////////////////////////////////////////
// Class definition
// 

void adc_input_setup(void) {
   adc_input_class = class_new(
      gensym("adc_input"), 
      (t_newmethod)adc_input_new, 
      (t_method)adc_input_free, 
      sizeof(t_adc_input), 
      CLASS_NOINLET, 
      A_DEFFLOAT,
      (t_atomtype)0
   );
}
