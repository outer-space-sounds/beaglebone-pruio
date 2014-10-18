/////////////////////////////////////////////////////////////////////
// UTIL
//
#define HWREG(x) (*((volatile unsigned int *)(x)))
/* #define min(a,b) (a<b ? a : b) */

/////////////////////////////////////////////////////////////////////
// Addresses
//

// PRU Module Registers
#define PRU_ICSS_CFG 0x26000
#define PRU_ICSS_CFG_SYSCFG 0x04

// IEP Module Registers
#define IEP 0x2e000
#define IEP_TMR_GLB_CFG 0x00
#define IEP_TMR_GLB_STS 0x04
#define IEP_TMR_COMPEN 0x08
#define IEP_TMR_CNT 0x0C
#define IEP_TMR_CMP_CFG 0x40
#define IEP_TMR_CMP_STS 0x44
#define IEP_TMR_CMP0 0x48
#define IEP_TMR_CMP1 0x4C

// Control Module Registers
#define CONTROL_MODULE 0x44e10000
#define CONF_P9_11 0x870
#define CONF_P9_12 0x878
#define CONF_P9_13 0x874
#define CONF_P9_27 0x9a4
#define CONF_P9_30 0x998
#define CONF_P9_42A 0x964

// Clock Module registers
#define CM_PER 0x44e00000
#define CM_PER_GPIO3_CLKCTRL 0xb4
#define CM_WKUP 0x44e00400
#define CM_WKUP_GPIO0_CLKCTRL 0x08
#define CM_WKUP_ADC_TSK_CLKCTL 0xbc

// GPIO Module registers
#define GPIO0 0x44e07000
#define GPIO1 0x4804c000
#define GPIO3 0x481ae000
#define GPIO_CTRL 0x130
#define GPIO_OE 0x134
#define GPIO_DATAOUT 0x13c
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

// ADC Registers
#define ADC_TSC 0x44e0d000
#define ADC_TSC_IRQSTATUS 0x28
#define ADC_TSC_IRQENABLE_SET 0x2c
#define ADC_TSC_CTRL 0x40
#define ADC_TSC_ADCRANGE 0x48
#define ADC_TSC_CLKDIV 0x4c
#define ADC_TSC_STEPENABLE 0x54
#define ADC_TSC_STEPCONFIG1 0x64
#define ADC_TSC_STEPDELAY1 0x68
#define ADC_TSC_STEPCONFIG2 0x6c
#define ADC_TSC_STEPDELAY2 0x70
#define ADC_TSC_STEPCONFIG3 0x74
#define ADC_TSC_STEPDELAY3 0x78
#define ADC_TSC_STEPCONFIG4 0x7c
#define ADC_TSC_STEPDELAY4 0x80
#define ADC_TSC_STEPCONFIG5 0x84
#define ADC_TSC_STEPDELAY5 0x88
#define ADC_TSC_STEPCONFIG6 0x8c
#define ADC_TSC_STEPDELAY6 0x90
#define ADC_TSC_STEPCONFIG7 0x94
#define ADC_TSC_STEPDELAY7 0x98
#define ADC_TSC_STEPCONFIG8 0x9c
#define ADC_TSC_STEPDELAY8 0xa0
#define ADC_TSC_STEPCONFIG9 0xa4
#define ADC_TSC_STEPDELAY9 0xa8
#define ADC_TSC_STEPCONFIG10 0xac
#define ADC_TSC_STEPDELAY10 0xb0
#define ADC_TSC_STEPCONFIG11 0xb4
#define ADC_TSC_STEPDELAY11 0xb8
#define ADC_TSC_STEPCONFIG12 0xbc
#define ADC_TSC_STEPDELAY12 0xc0
#define ADC_TSC_STEPCONFIG13 0xc4
#define ADC_TSC_STEPDELAY13 0xc8
#define ADC_TSC_STEPCONFIG14 0xcc
#define ADC_TSC_STEPDELAY14 0xd0
#define ADC_TSC_STEPCONFIG15 0xd4
#define ADC_TSC_STEPDELAY15 0xd8
#define ADC_TSC_STEPCONFIG16 0xdc
#define ADC_TSC_STEPDELAY16 0xe0
#define ADC_TSC_FIFO0COUNT 0xe0
#define ADC_TSC_FIFO1COUNT 0xf0
#define ADC_TSC_FIFO0DATA 0x100
#define ADC_TSC_FIFO1DATA 0x200


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

/////////////////////////////////////////////////////////////////////
// ANALYZE VALUES AND SEND TO ARM PROCESSOR
//

inline void process_values(){
   int i;
   for(i=0; i<1200; i++); 
}

/////////////////////////////////////////////////////////////////////
// MAIN
//

volatile unsigned int* shared_ram;
volatile register unsigned int __R31;

void init_ocp(){
   // Enable OCP so we can access the whole memory map for the
   // device from the PRU. Clear bit 4 of SYSCFG register
   HWREG(PRU_ICSS_CFG + PRU_ICSS_CFG_SYSCFG) &= ~(1 << 4);

   // Pointer to shared memory region
   shared_ram = (volatile unsigned int *)0x10000;
}

int main(int argc, const char *argv[]){
   init_ocp();
   init_gpio();
   init_adc();
   init_iep_timer();

   unsigned int i;
   unsigned int mux_control = 7;

   while(1){
      mux_control>6 ? mux_control=0 : mux_control++;
      set_mux_control(mux_control);

      adc_start_sampling();

      // Debug:
      HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30);
      for(i=0; i<20; i++); 
      HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30);

      process_values();

      // Debug:
      HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30);
      for(i=0; i<20; i++); 
      HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30);

      // Sample 1 to 7, it ends after 45uSec mark.
      wait_for_adc();

      // Debug:
      /* wait_for_short_timer(); */
      HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30);
      for(i=0; i<20; i++); 
      HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30);

      wait_for_timer(); // Timer resets itself after this
   }

   /* __R31 = 35; */
   /* __halt(); */

   return 0;
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
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG1) = 0 | (0x0<<26) | (0x00<<19) | (0x00<<15) | (average<<2) | (0x00);
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
