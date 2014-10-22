////////////////////////////////////////////////////////////////////////////////
// 
//  Pd-BeagleBoneBlack-Io  
//
//  Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
//
//  This program is free software: you can redistribute it and/or modify it 
//  under the terms of the GNU General Public License as published by the Free 
//  Software Foundation, either version 3 of the License, or (at your option) 
//  any later version.
//  
//  This program is distributed in the hope that it will be useful, but WITHOUT 
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
//  more details.  
//  
//  You should have received a copy of the GNU General Public License along 
//  with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include "m_pd.h"

/* #include <sys/time.h> */
#include <time.h>

/* #include <string.h> */
/* #include <pthread.h> */
/* #include <unistd.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include "libbbb_gpio.h" */

////////////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct _input_tilde {
   t_object x_obj;

   t_outlet *outlet_left;

   clock_t time;
   clock_t previous_time;
   unsigned int counter;
} t_input_tilde;

// A pointer to the class object.
static t_class *bbb_input_tilde_class;





///////////////////////////////////////////////////////////////////////////////
// DSP
//
static t_int *bbb_input_tilde_perform(t_int *w){
   t_input_tilde *x = (t_input_tilde *)(w[1]);
   t_float *out = (t_float *)(w[2]);
   int n = (int)(w[3]);

   x->previous_time = x->time;
   x->time = clock();

   if(x->counter > 1000){
      /* post("time: %lu, %lu, %lu", x->time, x->previous_time, (double)(x->time - x->previous_time)); */
      post("time: %f", ((double)(x->time - x->previous_time))/CLOCKS_PER_SEC);
      x->counter = 0;
   }
   x->counter ++;

   /* struct timespec t; */
   /* x->previous_time = x->time; */
   /* clock_gettime(CLOCK_MONOTONIC,&t); */
   /* x->time = 1000000000*t.tv_sec + t.tv_nsec; */
   /* if(x->counter > 1000){ */
   /*    post("time: %lu", (x->time - x->previous_time)/1000); */
   /*    x->counter = 0; */
   /* } */
   /* x->counter ++; */


   while(n>0){
      *out = 1.0; 
      out++;
      n--;
   }

   return (w+4);
}

static void bbb_input_tilde_dsp(t_input_tilde *x, t_signal **sp){
   dsp_add(bbb_input_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

///////////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *bbb_input_tilde_new(void) {
   t_input_tilde *x = (t_input_tilde *)pd_new(bbb_input_tilde_class);
   x->outlet_left = outlet_new(&x->x_obj, gensym("signal"));

   x->counter = 0;
   x->time = 0;
   x->previous_time = 0;

   return (void *)x;
}

static void bbb_input_tilde_free(t_input_tilde *x) { 
   outlet_free(x->outlet_left);
}

///////////////////////////////////////////////////////////////////////////////
// Class definition
// 

void bbb_input_tilde_setup(void) {
   bbb_input_tilde_class = class_new(gensym("input~"), (t_newmethod)bbb_input_tilde_new, (t_method)bbb_input_tilde_free, sizeof(t_input_tilde), CLASS_DEFAULT, (t_atomtype)0);
   class_addmethod(bbb_input_tilde_class, (t_method)bbb_input_tilde_dsp, gensym("dsp"), A_CANT, 0);
}
