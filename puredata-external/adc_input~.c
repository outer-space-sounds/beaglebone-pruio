#include <m_pd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct _adc_input_tilde {
   t_object x_obj;
   t_outlet *outlet_left;
   char channel[3];

   int line_output;

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
   t_float f = atom_getfloat(argv);
   if( f==0 && strcmp("float", atom_getsymbol(argv)->s_name)!=0 ){ // Invalid floats will be parsed as zero so let's check if that zero is actually valid
      error("beaglebone/adc_input~: %s is not a valid ADC channel.", atom_getsymbol(argv)->s_name);
      return NULL;
   }
   if(f<0 || f>99 || (t_float)((int)f)!=(f)){
      error("beaglebone/adc_input~: %f is not a valid ADC channel.", f); 
      return NULL;
   }
   
   t_adc_input_tilde *x = (t_adc_input_tilde *)pd_new(adc_input_tilde_class);
   x->outlet_left = outlet_new(&x->x_obj, gensym("signal"));
   /* x->timeout_clock = clock_new(x,  (t_method)adc_input_tilde_timeout);  */

   x->current_value = 0;
   x->target_value = 0;
   x->increment = 0;

   // Parse creation parameter 2 (line)
   x->line_output = 0;
   if(argc >=2){
      t_symbol* param = atom_getsymbolarg(1, argc, argv);   
      if(strcmp(param->s_name, "line")==0){
         x->line_output = 1;
      }
   }

   sprintf(x->channel, "%i", (int)f);
   char err[256];
   if(beaglebone_clock_new(0, x->channel, x, adc_input_tilde_callback, err)){
      error("beaglebone/adc_input~: %s", err); 
      return NULL;
   }

   return (void *)x;
}

static void adc_input_tilde_free(t_adc_input_tilde *x) { 
   outlet_free(x->outlet_left);
   /* clock_free(x->timeout_clock); */
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
