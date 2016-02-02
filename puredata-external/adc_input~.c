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
#include <math.h>
#include <beaglebone_pruio_pins.h>

#ifdef IS_BEAGLEBONE
#include <beaglebone_pruio.h>
#endif 

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct _adc_input_tilde {
   t_object x_obj;
   t_outlet *outlet_left;
   int channel;

   int line_output;
   int bits;

   t_float increment;
   t_float target_value;
   t_float current_value;
} t_adc_input_tilde;

// A pointer to the class object.
static t_class *adc_input_tilde_class;


/////////////////////////////////////////////////////////////////////////
// Callback from lib beaglebone_pruio
//

void adc_input_tilde_callback(void* this, t_float value){
   t_adc_input_tilde* x = (t_adc_input_tilde*)this;

   #ifndef IS_BEAGLEBONE
      value = (int)(value * (1<<x->bits));
   #endif 

   if(x->line_output){
      x->target_value = value;
      
      // 0.66666 milliseconds is the desired ramp time
      // sys_getsr() is puredata's audio sample rate:
      x->increment = (x->target_value - x->current_value) / (0.00066666*sys_getsr()); 
   }
   else{
      x->current_value = value;
   }
}


/////////////////////////////////////////////////////////////////////////
// DSP
//
static t_int *adc_input_tilde_perform(t_int *w){
   t_adc_input_tilde *x = (t_adc_input_tilde *)(w[1]);
   t_float *out = (t_float *)(w[2]);
   int n = (int)(w[3]);

   if(!x->line_output){
      while(n--){
         *out++ = x->current_value; 
      }
   }
   else{
      while(n--){
         if(fabs(x->target_value - x->current_value) <= fabs(x->increment)){
            x->current_value = x->target_value;
         }
         else{
            x->current_value += x->increment;
         }
         *out++ = x->current_value; 
      }
   }

   return (w+4);
}

static void adc_input_tilde_dsp(t_adc_input_tilde *x, t_signal **sp){
   dsp_add(adc_input_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

/////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *adc_input_tilde_new(t_symbol *s, int argc, t_atom *argv) {
   (void)s;

   // Parse creation parameter 1 (channel number)
   if(argc < 1){
      error("beaglebone/adc_input~: You need to specify the channel number");
      return NULL;
   }
   
   // Invalid floats will be parsed as zero so let's check if that 
   // zero is actually valid
   t_float f = atom_getfloat(argv);
   if( f==0 && strcmp("float", atom_getsymbol(argv)->s_name)!=0 ){ 
      error("beaglebone/adc_input~: %s is not a valid ADC channel.", atom_getsymbol(argv)->s_name);
      return NULL;
   }
   if(f<0 || f>BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS || (t_float)((int)f)!=(f)){
      error("beaglebone/adc_input~: %f is not a valid ADC channel.", f); 
      return NULL;
   }

   // other parameters? 
   int i;
   int bits = 7;
   int line_output = 0;
   for(i=1; i<argc; ++i){
      char* param = atom_getsymbol(argv+i)->s_name;
      // Bits
      if(strcmp(param, "bits") == 0){
         bits = atom_getfloat(argv+i+1);
         if(bits<=0){
            bits = 7;
         }
         if(bits>12){
            bits = 12;
         }
      }
      // Line
      else if(strcmp(param, "line") == 0){
         line_output = 1;
      }
   }

   // Debug
   /* error("c:%i, b:%i, l:%i", (int)f, bits, line_output); */

   #ifdef IS_BEAGLEBONE
      if(beaglebone_pruio_init_adc_pin((int)f, bits)){
         error("beaglebone/adc_input: Could not init adc channel %s (%f), is it already in use?", atom_getsymbol(argv)->s_name, f);
         return NULL;
      }
   #endif 
   
   t_adc_input_tilde *x = (t_adc_input_tilde *)pd_new(adc_input_tilde_class);
   x->outlet_left = outlet_new(&x->x_obj, gensym("signal"));
   x->channel = f;
   x->bits = bits;
   x->line_output = line_output;
   x->current_value = 0;
   x->target_value = 0;
   x->increment = 0;

   if(beaglebone_register_callback(BB_GPIO_ANALOG, (int)f, x, adc_input_tilde_callback)){
      error("beaglebone/adc_input: Could not init adc channel %s (%f), is it already in use?", atom_getsymbol(argv)->s_name, f);
      return NULL;
   }

   return (void *)x;
}

static void adc_input_tilde_free(t_adc_input_tilde *x) { 
   beaglebone_unregister_callback(BB_GPIO_ANALOG, x->channel);
}

/////////////////////////////////////////////////////////////////////////
// Class definition
// 

void adc_input_tilde_setup(void) {
   adc_input_tilde_class = class_new(
      gensym("adc_input~"),
      (t_newmethod)adc_input_tilde_new,
      (t_method)adc_input_tilde_free,
      sizeof(t_adc_input_tilde),
      CLASS_NOINLET,
      A_GIMME,
      (t_atomtype)0
   );

   class_addmethod(
      adc_input_tilde_class,
      (t_method)adc_input_tilde_dsp,
      gensym("dsp"),
      A_CANT,
      0
   );
}
