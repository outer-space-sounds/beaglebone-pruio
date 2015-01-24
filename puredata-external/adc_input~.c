#include <m_pd.h>
#include <stdio.h>
#include <string.h>

#include "beaglebone.h"

/////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct _adc_input_tilde {
   t_object x_obj;
   t_outlet *outlet_left;
   float target_value;
   char channel[2];
} t_adc_input_tilde;

// A pointer to the class object.
static t_class *adc_input_tilde_class;


/////////////////////////////////////////////////////////////////////////
// Callback from lib bbb_pruio
//

void adc_input_tilde_callback(void* this, float value){
   t_adc_input_tilde* x = (t_adc_input_tilde*)this;
   x->target_value = value;
   /* printf("%f\n", x->target_value); */
}


/////////////////////////////////////////////////////////////////////////
// DSP
//
static t_int *adc_input_tilde_perform(t_int *w){
   t_adc_input_tilde *x = (t_adc_input_tilde *)(w[1]);
   t_float *out = (t_float *)(w[2]);
   int n = (int)(w[3]);

   while(n>0){
      *out = x->target_value; 
      out++;
      n--;
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

   // Parse creation parameters
   if(argc < 1){
      error("beaglebone/adc_input~: You need to specify the channel number");
      return NULL;
   }

   float f = atom_getfloat(argv);
   
   // Invalid floats will be parsed as zero so let's check if that zero is actually valid
   if( f==0 && strcmp("float", atom_getsymbol(argv)->s_name)!=0 ){
      error("beaglebone/adc_input~: %s is not a valid ADC channel.", atom_getsymbol(argv)->s_name);
      return NULL;
   }

   if(f<0 || f>99 || (float)((int)f)!=(f)){
      error("beaglebone/adc_input~: %f is not a valid ADC channel.", f); 
      return NULL;
   }

   t_adc_input_tilde *x = (t_adc_input_tilde *)pd_new(adc_input_tilde_class);
   x->outlet_left = outlet_new(&x->x_obj, gensym("signal"));

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
