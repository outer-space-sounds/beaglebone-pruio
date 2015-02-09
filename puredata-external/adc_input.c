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

#include <m_pd.h>
#include <stdio.h>
#include <string.h>
#include <beaglebone_pruio_pins.h>

#ifdef IS_BEAGLEBONE
#include <beaglebone_pruio.h>
#endif 

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct adc_input {
   t_object x_obj;
   t_outlet *outlet_left;
   int channel;
   int bits;
   int ranges;
} t_adc_input;

// A pointer to the class object.
t_class *adc_input_class;

/////////////////////////////////////////////////////////////////////////
// Callback from lib beaglebone_pruio
//

void adc_input_callback(void* x, float value){
   t_adc_input* this = (t_adc_input*)x;

   #ifndef IS_BEAGLEBONE
      if(this->ranges != 0){
         value = (int)(value * 255) % this->ranges;
      }
      else{
         value = (int)(value * (1<<this->bits));
      }
   #endif 

   outlet_float(this->outlet_left, value);
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *adc_input_new(t_symbol *s, int argc, t_atom *argv) {
   (void)s;

   // 1.1 Parse channel number from creation parameters
   if(argc < 1){
      error("beaglebone/adc_input: You need to specify the channel number");
      return NULL;
   }
   
   // 1.1.1 Invalid floats will be parsed as zero so let's check if 
   //      that zero is actually valid
   float f = atom_getfloat(argv); // Argument number 0
   if( f==0 && strcmp("float", atom_getsymbol(argv)->s_name)!=0 ){
      error("beaglebone/adc_input: %s is not a valid ADC channel.", atom_getsymbol(argv)->s_name);
      return NULL;
   }

   // 1.1.2. Is channel number in range?
   if(f<0 || f>=BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS || (float)((int)f)!=(f)){
      error("beaglebone/adc_input: %s is not a valid ADC channel.", atom_getsymbol(argv)->s_name); 
      return NULL;
   }

   // Are there more parameters? 
   #ifdef IS_BEAGLEBONE
      beaglebone_pruio_adc_mode mode = BEAGLEBONE_PRUIO_ADC_MODE_NORMAL;
   #endif
   int bits = 7;
   int ranges = 0;
   if(argc == 3){ 
      char* param1 = atom_getsymbol(argv+1)->s_name;
      
      // 1.2 Parse number of bits
      if(strcmp(param1, "bits") == 0){
         bits = atom_getfloat(argv+2);
         if(bits<=0){
            bits = 7;
         }
         if(bits>12){
            bits = 12;
         }

         // Debug
         /* error("Bits: %i", bits); */
      }

      // 1.3 Parse number of ranges
      else if(strcmp(param1, "ranges") == 0){
         ranges = atom_getfloat(argv+2);
         if(ranges<=0){
            ranges = 12;
         }
         if(ranges>24){
            ranges = 24;
         }
         #ifdef IS_BEAGLEBONE
            mode = BEAGLEBONE_PRUIO_ADC_MODE_RANGES;
         #endif
            
         // Debug
         /* error("Ranges: %i", ranges); */
      }
   }
   
   // 2. Try to initialize the adc input
   #ifdef IS_BEAGLEBONE
      int err = 0;
      switch(mode){
         case BEAGLEBONE_PRUIO_ADC_MODE_NORMAL:
            err = beaglebone_pruio_init_adc_pin((int)f, bits);
            break;
         case BEAGLEBONE_PRUIO_ADC_MODE_RANGES:
            err = beaglebone_pruio_init_adc_pin_with_ranges((int)f, ranges);
            break;
         case BEAGLEBONE_PRUIO_ADC_MODE_OFF:
            //get rid of compiler warning
            break;
      };

      if(err){
         error("beaglebone/adc_input: Could not init adc channel %s (%f), is it already in use?", atom_getsymbol(argv)->s_name, f);
         return NULL;
      }
   #endif 

   // 3. Create pd object instance
   t_adc_input *x = (t_adc_input *)pd_new(adc_input_class);
   x->outlet_left = outlet_new(&x->x_obj, &s_float);
   x->channel = (int)f;
   x->bits = bits;
   x->ranges = ranges;

   // 4. Register the callback.
   if(beaglebone_register_callback(0, (int)f, x, adc_input_callback)){
      error("beaglebone/adc_input: Could not init adc channel %s (%f), is it already in use?", atom_getsymbol(argv)->s_name, f);
      return NULL;
   }

   return (void *)x;
}

static void adc_input_free(t_adc_input *x) { 
   beaglebone_unregister_callback(0, x->channel);
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
      A_GIMME,
      (t_atomtype)0
   );
}
