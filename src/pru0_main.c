#include "registers.h"

/////////////////////////////////////////////////////////////////////
// UTIL
//
#define HWREG(x) (*((volatile unsigned int *)(x)))
/* #define min(a,b) (a<b ? a : b) */


/////////////////////////////////////////////////////////////////////
// DECLARATIONS
//

volatile unsigned int* shared_ram;
volatile register unsigned int __R31;

void init_ocp();
void init_adc();
void init_gpio();
void init_iep_timer();
inline void set_mux_control(unsigned int ctl);
inline void wait_for_timer();
inline void wait_for_short_timer();
inline void wait_for_adc();
inline void adc_start_sampling();
inline void process_values();
void init_buffer();
inline void buffer_write(unsigned int message);

typedef enum channel_mode{
   CHANNEL_MODE_OFF = 0,
   CHANNEL_MODE_RAW,
   CHANNEL_MODE_STEPPED
} channel_mode;

typedef struct channel{
   channel_mode mode; 
   unsigned int value; 
   unsigned int steps;
   unsigned int lower_bound;
   unsigned int upper_bound;
} channel;


/////////////////////////////////////////////////////////////////////
// ANALYZE VALUES 
//

unsigned int mux_control;
channel channels[14];
/* unsigned int need_interrupt; */
unsigned int interrupt_counter=0;
unsigned int interrupt_counter2=0;

inline void process_value(unsigned int channel_number, unsigned int value){
   if(channels[channel_number].mode == CHANNEL_MODE_OFF){
      return;
   }

   unsigned int truncated_value, message;
   if(channels[channel_number].mode == CHANNEL_MODE_RAW){
      truncated_value = value >> 5;
      if(channels[channel_number].value != truncated_value){
         channels[channel_number].value = truncated_value;
         /* need_interrupt = 1; */
         message = (channel_number << 16) | (truncated_value);
         buffer_write(message);

         /* int i; */
         /* for(i=0; i<10000; i++); */
      }
   }
}

inline void process_values(){
   // Read available samples from fifo0 in blocks of seven
   unsigned int data, i, step_id, value, channel_number;
   /* need_interrupt = 0; */
   unsigned int count = HWREG(ADC_TSC + ADC_TSC_FIFO0COUNT);
   while(count > 6){
      for(i=0; i<7; i++){
         data = HWREG(ADC_TSC + ADC_TSC_FIFO0DATA);
         step_id = (data & (0xf0000)) >> 16;
         value = (data & 0xfff);
         if(step_id==5){ // The channel with the mux, channel 6 on adc.
            if(mux_control == 0)
               channel_number = 13;
            else
               channel_number = 6+mux_control-1;
         }
         else if(step_id==6){ // Last step is actually channel 5
            channel_number = 5;
         }
         else{ // Other channels
            channel_number = step_id;
         }
         process_value(channel_number, value);
      }
      count = HWREG(ADC_TSC + ADC_TSC_FIFO0COUNT);
   }

   interrupt_counter++;
   if(interrupt_counter>12){
      interrupt_counter = 0;

      /* interrupt_counter2++; */
      /* if(interrupt_counter2 < 10000){ */
         __R31 = 35; 
      /* } */
      /* else if(interrupt_counter2 >= 100000){ */
      /*    interrupt_counter2=0; */
      /* } */
   }
}

void init_values(){
   channel new_channel;
   new_channel.mode = CHANNEL_MODE_RAW;
   new_channel.value = 0;
   new_channel.steps = 0;
   new_channel.upper_bound = 0;
   new_channel.lower_bound = 0;

   int i;
   for(i=0; i<14; i++){
      channels[i] = new_channel; 
   }
}

/////////////////////////////////////////////////////////////////////
// MAIN
//

int main(int argc, const char *argv[]){
   init_values();
   init_ocp();
   init_buffer();
   init_gpio();
   init_adc();
   init_iep_timer();

   unsigned int i;
   mux_control = 7;

   // TODO: exit condition
   while(1){
      mux_control>6 ? mux_control=0 : mux_control++;
      set_mux_control(mux_control);

      adc_start_sampling();
      process_values();

      // Debug:
      HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30);
      for(i=0; i<20; i++); 
      HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30);

      // Debug:
      /* wait_for_short_timer(); */
      /* HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30); */
      /* for(i=0; i<20; i++);  */
      /* HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30); */

      // Debug:
      wait_for_adc();
      HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30);
      for(i=0; i<20; i++); 
      HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30);

      wait_for_timer(); // Timer resets itself after this
   }

   __halt();
   return 0;
}

void init_ocp(){
   // Enable OCP so we can access the whole memory map for the
   // device from the PRU. Clear bit 4 of SYSCFG register
   HWREG(PRU_ICSS_CFG + PRU_ICSS_CFG_SYSCFG) &= ~(1 << 4);

   // Pointer to shared memory region
   shared_ram = (volatile unsigned int *)0x10000;
}

/////////////////////////////////////////////////////////////////////
// RING BUFFER
//

// Communication with ARM processor is througn a ring buffer in the
// PRU shared memory area.
// shared_ram[0] to shared_ram[1023] is the buffer data.
// shared_ram[1024] is the start (read) pointer.
// shared_ram[1025] is the end (write) pointer.
// shared_ram[1026] is the number of overflows.
//
// Messages are 32 bit unsigned ints. The 16 MSbits are the channel 
// number and the 16 LSbits are the value.
// 
// Read these:
// * http://en.wikipedia.org/wiki/Circular_buffer#Use_a_Fill_Count
// * https://groups.google.com/forum/#!category-topic/beagleboard/F9JI8_vQ-mE

unsigned int buffer_size;
volatile unsigned int *buffer_start;
volatile unsigned int *buffer_end;

void init_buffer(){
   // data in shared_ram[0] to shared_ram[127]
   buffer_size = 1024; 
   buffer_start = &(shared_ram[1024]);
   buffer_end = &(shared_ram[1025]);
   *buffer_start = 0;
   *buffer_end = 0;
   shared_ram[1026] = 0;
}

inline void buffer_write(unsigned int message){
   // Note that if buffer is full, messages will be dropped
   unsigned int is_full = (*buffer_end == (*buffer_start^buffer_size)); // ^ is orex
   if(!is_full){
      shared_ram[*buffer_end & (buffer_size-1)] = message;
      // Increment buffer end, wrap around size
      *buffer_end = (*buffer_end+1) & (2*buffer_size - 1);
   }
   else{
      shared_ram[1026] ++; 
   }
}

/////////////////////////////////////////////////////////////////////
// TIMER
//
void init_iep_timer(){
   // We'll count 83.333 micro seconds with compare0 register and 45uSec
   // with compare1 register.
   // clock is 200MHz, use increment value of 5, 
   // compare values are then 83333 and 45000

   // 1. Initialize timer to known state
   // 1.1 Disable timer counter
   HWREG(IEP + IEP_TMR_GLB_CFG) &= ~(1); 
   // 1.2 Reset counter (write 1 to clear)
   HWREG(IEP + IEP_TMR_CNT) = 0xffffffff; 
   // 1.3 Clear overflow status
   HWREG(IEP + IEP_TMR_GLB_STS) = 0; 
   // 1.4Clear compare status (write 1 to clear)
   HWREG(IEP + IEP_TMR_CMP_STS) = 0xf; 


   // 2. Set compare values 
   HWREG(IEP + IEP_TMR_CMP0) = 83333; 
   // 2.1 Compare register 1 to 45000
   /* HWREG(IEP + IEP_TMR_CMP1) = 45000; // Used when debugging timing */ 

   // 3. Enable compare events and reset counter when 
   // compare 0 event happens
   HWREG(IEP + IEP_TMR_CMP_CFG) = (1 << 1) | 1; // Compare event 0 only
   /* HWREG(IEP + IEP_TMR_CMP_CFG) = (1 << 2) | (1 << 1) | 1; // Compare evts 0 and 1 */ 
   
   // 4. Set increment value (5)
   HWREG(IEP + IEP_TMR_GLB_CFG) |= 5<<4; 

   // 5. Set compensation value (not needed now)
   HWREG(IEP + IEP_TMR_COMPEN) = 0; 
   
   // 6. Enable counter
   HWREG(IEP + IEP_TMR_GLB_CFG) |= 1; 
}

inline void wait_for_timer(){
   // Wait for compare 0 status to go high
   while((HWREG(IEP+IEP_TMR_CMP_STS) & 1) == 0){
      // nothing 
   }

   // Clear compare 0 status (write 1)
   HWREG(IEP+IEP_TMR_CMP_STS) |= 1;
}

inline void wait_for_short_timer(){
   // Wait for compare 1 status to go high
   while((HWREG(IEP+IEP_TMR_CMP_STS) & (1<<1)) == 0){
      // nothing 
   }

   // Clear compare 1 status (write 1)
   HWREG(IEP+IEP_TMR_CMP_STS) |= (1<<1);
}

/////////////////////////////////////////////////////////////////////
// GPIO (Mux control)
//
void init_gpio(){
   // See BeagleboneBlackP9HeaderTable.pdf from derekmolloy.ie
   // Way easier to read than TI's manual

   // Enable GPIO Module.
   HWREG(GPIO0 + GPIO_CTRL) = 0x00;

   // Enable GPI3 Module.
   HWREG(GPIO3 + GPIO_CTRL) = 0x00;

   // Enable clock for GPIO0 module. 
   HWREG(CM_WKUP + CM_WKUP_GPIO0_CLKCTRL) = (0x02) | (1<<18);

   // Enable clock for GPIO3 module. 
   HWREG(CM_PER + CM_PER_GPIO3_CLKCTRL) = (0x02) | (1<<18);
   
   // P9_11 pin as an output, GPIO0[30], no pull. 
   // We'll use it for debugging DAC timing
   HWREG(CONTROL_MODULE + CONF_P9_11) = 0x0F;
   HWREG(GPIO0 + GPIO_OE) &= ~(1<<30);

   // P9_27 pin as an output, GPIO3[19], no pull.
   HWREG(CONTROL_MODULE + CONF_P9_27) = 0x0F;
   HWREG(GPIO3 + GPIO_OE) &= ~(1<<19);

   // P9_30 pin as output, GPIO3[16], no pull.
   HWREG(CONTROL_MODULE + CONF_P9_30) = 0x0F;
   HWREG(GPIO3 + GPIO_OE) &= ~(1<<16);

   // P9_42A pin as output, GPIO0[7], no pull.
   HWREG(CONTROL_MODULE + CONF_P9_42A) = 0x0F;
   HWREG(GPIO0 + GPIO_OE) &= ~(1<<7);
}

inline void set_mux_control(unsigned int ctl){
   // 3 bits for mux control: 
   // [P9_27 GPIO3[19], P9_30 GPIO3[16], P9_42A GPIO0[7]]
   switch(ctl){
      case 0:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      case 1:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;

      case 2:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      case 3:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;

      case 4:
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      case 5:
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;

      case 6:
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      default: //7
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;
   }
}

/////////////////////////////////////////////////////////////////////
// Analog Digital Conversion
//
inline void wait_for_adc(){
   // Wait for irqstatus[1] to go high
   while((HWREG(ADC_TSC + ADC_TSC_IRQSTATUS) & (1<<1)) == 0){
      // nothing 
   }

   // Clear status (write 1)
   HWREG(ADC_TSC + ADC_TSC_IRQSTATUS) |= (1<<1);
}

inline void adc_start_sampling(){
   // Enable steps 1 to 7
   HWREG(ADC_TSC + ADC_TSC_STEPENABLE) = 0xfe;
   /* HWREG(ADC_TSC + ADC_TSC_STEPENABLE) = 0b111110; // 1 to 5 */
}

void init_adc(){
   // Enable clock for adc module.
   HWREG(CM_WKUP + CM_WKUP_ADC_TSK_CLKCTL) = 0x02;

   // Disable ADC module temporarily.
   HWREG(ADC_TSC + ADC_TSC_CTRL) &= ~(0x01);

   // To calculate sample rate:
   // fs = 24MHz / (CLK_DIV*2*Channels*(OpenDly+Average*(14+SampleDly)))
   // We want 48KHz. (Compromising to 50KHz)
   unsigned int clock_divider = 1;
   unsigned int open_delay = 0;
   unsigned int average = 0;       // can be 0 (no average), 1 (2 samples), 
                                   // 2 (4 samples),  3 (8 samples) 
                                   // or 4 (16 samples)
   unsigned int sample_delay = 0;

   // Set clock divider (set register to desired value minus one). 
   HWREG(ADC_TSC + ADC_TSC_CLKDIV) = clock_divider - 1;

   // Set values range from 0 to FFF.
   HWREG(ADC_TSC + ADC_TSC_ADCRANGE) = (0xfff << 16) & (0x000);

   // Disable all steps. STEPENABLE register
   HWREG(ADC_TSC + ADC_TSC_STEPENABLE) &= ~(0xff);

   // Unlock step config register.
   HWREG(ADC_TSC + ADC_TSC_CTRL) |= (1 << 2);

   // Set config and delays for step 1: 
   // Sw mode, one shot mode, fifo0, channel 0.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG1) = 0 | (0<<26) | (0<<19) | (0<<15) | (average<<2) | (0);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY1)  = 0 | (sample_delay - 1)<<24 | open_delay;

   // Set config and delays for step 2: 
   // Sw mode, one shot mode, fifo0, channel 1.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG2) = 0 | (0x0<<26) | (0x01<<19) | (0x01<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY2)  = 0 | (sample_delay - 1)<<24 | open_delay;

   // Set config and delays for step 3: 
   // Sw mode, one shot mode, fifo0, channel 2.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG3) = 0 | (0x0<<26) | (0x02<<19) | (0x02<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY3)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 4: 
   // Sw mode, one shot mode, fifo0, channel 3.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG4) = 0 | (0x0<<26) | (0x03<<19) | (0x03<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY4)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 5: 
   // Sw mode, one shot mode, fifo0, channel 4.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG5) = 0 | (0x0<<26) | (0x04<<19) | (0x04<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY5)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 6: 
   // Sw mode, one shot mode, fifo0, CHANNEL 6!
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG6) = 0 | (0x0<<26) | (0x06<<19) | (0x06<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY6)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 7: 
   // Sw mode, one shot mode, fifo0, CHANNEL 5!
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG7) = 0 | (0x0<<26) | (0x05<<19) | (0x05<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY7)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Enable tag channel id. Samples in fifo will have channel id bits ADC_CTRL register
   HWREG(ADC_TSC + ADC_TSC_CTRL) |= (1 << 1);

   // Clear End_of_sequence interrupt
   HWREG(ADC_TSC + ADC_TSC_IRQSTATUS) |= (1<<1);

   // Enable End_of_sequence interrupt
   HWREG(ADC_TSC + ADC_TSC_IRQENABLE_SET) |= (1 << 1);
   
   // Lock step config register. ACD_CTRL register
   HWREG(ADC_TSC + ADC_TSC_CTRL) &= ~(1 << 2);
   
   // Clear FIFO0 by reading from it.
   unsigned int count = HWREG(ADC_TSC + ADC_TSC_FIFO0COUNT);
   unsigned int data, i;
   for(i=0; i<count; i++){
      data = HWREG(ADC_TSC + ADC_TSC_FIFO0DATA);
   }

   // Clear FIFO1 by reading from it.
   count = HWREG(ADC_TSC + ADC_TSC_FIFO1COUNT);
   for(i=0; i<count; i++){
      data = HWREG(ADC_TSC + ADC_TSC_FIFO1DATA);
   }
   shared_ram[500] = data; // just remove unused value warning;

   // Enable ADC Module. ADC_CTRL register
   HWREG(ADC_TSC + ADC_TSC_CTRL) |= 1;
}
