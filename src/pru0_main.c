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
#define SYSCFG 0x04

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

// GPIO Module registers
#define GPIO0 0x44e07000
#define GPIO1 0x4804c000
#define GPIO3 0x481ae000
#define GPIO_CTRL 0x130
#define GPIO_OE 0x134
#define GPIO_DATAOUT 0x13c
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

/////////////////////////////////////////////////////////////////////
// GLOBALS
//
void init_ocp();
void init_adc();
void init_gpio();
inline void set_mux_control(unsigned int ctl);

/* volatile unsigned int* shared_ram; */
volatile register unsigned int __R31;

/////////////////////////////////////////////////////////////////////
// MAIN
//
int main(int argc, const char *argv[]){
   init_ocp();
   init_gpio();
   init_adc();

   unsigned int i;
   unsigned int mux_control = 7;
   while(1){
      mux_control>6 ? mux_control=0 : mux_control++;
      set_mux_control(mux_control);

      for(i=0; i<5000; i++); // Approx. 125uS = 1ms/8
   }

   /* __R31 = 35; */
   /* __halt(); */

   return 0;
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
// INIT ADC
//

void init_ocp(){
   // Enable OCP so we can access the whole memory map for the
   // device from the PRU. Clear bit 4 of SYSCFG register
   HWREG(PRU_ICSS_CFG + SYSCFG) &= ~(1 << 4);
}

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

void init_adc(){
/*    // Enable OCP so we can access the whole memory map for the */
/*    // device from the PRU. Clear bit 4 of SYSCFG register */
/*    HWREG(0x26004) &= 0xFFFFFFEF; */
/*  */
/*    // Enable clock for adc module. CM_WKUP_ADC_TSK_CLKCTL register */
/*    HWREG(0x44e004bc) = 0x02; */
/*  */
/*    // Disable ADC module temporarily. ADC_CTRL register */
/*    HWREG(0x44e0d040) &= ~(0x01); */
/*  */
/*    // To calculate sample rate: */
/*    // fs = 24MHz / (CLK_DIV*2*Channels*(OpenDly+Average*(14+SampleDly))) */
/*    // We want 48KHz. (Compromising to 50KHz) */
/*    unsigned int clock_divider = 4; */
/*    unsigned int open_delay = 4; */
/*    unsigned int average = 1;       // can be 0 (no average), 1 (2 samples),  */
/*                                    // 2 (4 samples),  3 (8 samples)  */
/*                                    // or 4 (16 samples) */
/*    unsigned int sample_delay = 4; */
/*  */
/*    // Set clock divider (set register to desired value minus one).  */
/*    // ADC_CLKDIV register */
/*    HWREG(0x44e0d04c) = clock_divider - 1; */
/*  */
/*    // Set values range from 0 to FFF. ADCRANGE register */
/*    HWREG(0x44e0d048) = (0xfff << 16) & (0x000); */
/*  */
/*    // Disable all steps. STEPENABLE register */
/*    HWREG(0x44e0d054) &= ~(0xff); */
/*  */
/*    // Unlock step config register. ACD_CTRL register */
/*    HWREG(0x44e0d040) |= (0x01 << 2); */
/*  */
/*    // Set config for step 1. sw mode, continuous mode,  */
/*    // use fifo0, use channel 0. STEPCONFIG1 register */
/*    HWREG(0x44e0d064) = 0x0000 | (0x0<<26) | (0x00<<19) | (0x00<<15) | (average<<2) | (0x01); */
/*  */
/*    // Set delays for step 1. STEPDELAY1 register */
/*    HWREG(0x44e0d068) = 0x0000 | (sample_delay - 1)<<24 | open_delay; */
/*  */
/*    // Set config for step 2. sw mode, continuous mode,  */
/*    // use fifo0, use channel 1. STEPCONFIG2 register */
/*    HWREG(0x44e0d06c) = 0x0000 | (0x0<<26) | (0x01<<19) | (0x01<<15) | (average<<2) | (0x01); */
/*  */
/*    // Set delays for step 2. STEPDELAY2 register */
/*    HWREG(0x44e0d070) = 0x0000 | (sample_delay - 1)<<24 | open_delay; */
/*  */
/*    // Set config for step 3. sw mode, continuous mode,  */
/*    // use fifo0, use channel 2. STEPCONFIG3 register */
/*    HWREG(0x44e0d074) = 0x0000 | (0x0<<26) | (0x02<<19) | (0x02<<15) | (average<<2) | (0x01); */
/*  */
/*    // Set delays for step 3. STEPDELAY3 register */
/*    HWREG(0x44e0d078) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */
/*  */
/*    // Set config for step 4. sw mode, continuous mode,  */
/*    // use fifo1, use channel 3. STEPCONFIG4 register */
/*    #<{(| HWREG(0x44e0d07c) = 0x0000 | (0x0<<26) | (0x03<<19) | (0x03<<15) | (average<<2) | (0x01); |)}># */
/*  */
/*    // Set delays for step 4. STEPDELAY4 register */
/*    #<{(| HWREG(0x44e0d080) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; |)}># */
/*  */
/*  */
/*    // Set config for step 5. sw mode, continuous mode,  */
/*    // use fifo1, use channel 4. STEPCONFIG5 register */
/*    #<{(| HWREG(0x44e0d084) = 0x0000  | (0x0<<26) | (0x04<<19) | (0x04<<15) | (average<<2) | (0x01); |)}># */
/*  */
/*    // Set delays for step 5. STEPDELAY5 register */
/*    #<{(| HWREG(0x44e0d088) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; |)}># */
/*  */
/*    // Set config for step 6. sw mode, continuous mode,  */
/*    // use fifo1, use channel 5. STEPCONFIG6 register */
/*    #<{(| HWREG(0x44e0d08c) = 0x0000 | (0x0<<26) | (0x05<<19) | (0x05<<15) | (average<<2) | (0x01); |)}># */
/*  */
/*    // Set delays for step 6. STEPDELAY6 register */
/*    #<{(| HWREG(0x44e0d090) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; |)}># */
/*  */
/*    #<{(| // Set config for step 7. Average 16, sw mode, continuous mode,  |)}># */
/*    #<{(| // use fifo0, use channel 6. STEPCONFIG7 register |)}># */
/*    #<{(| HWREG(0x44e0d094) |= ((0x0<<26) | (0x06<<19) | (0x06<<15) | (0x04<<2) | (0x01)); |)}># */
/*  */
/*    #<{(| // Set delays for step 7. STEPDELAY7 register |)}># */
/*    #<{(| HWREG(0x44e0d098) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; |)}># */
/*  */
/*    // Lock step config register. ACD_CTRL register */
/*    HWREG(0x44e0d040) &= ~(0x01 << 2); */
/*     */
/*    // Clear FIFO0 by reading from it. FIFO0COUNT, FIFO0DATA registers */
/*    unsigned int count = HWREG(0x44e0d0e4); */
/*    unsigned int data, i; */
/*    for(i=0; i<count; i++){ */
/*       data = HWREG(0x44e0d100); */
/*    } */
/*  */
/*    // Clear FIFO1 by reading from it. FIFO1COUNT, FIFO1DATA registers */
/*    count = HWREG(0x44e0d0f0); */
/*    for (i=0; i<count; i++){ */
/*       data = HWREG(0x44e0d200); */
/*    } */
/*    shared_ram[500] = data; // just remove unused value warning; */
/*  */
/*    // Enable tag channel id. ADC_CTRL register */
/*    HWREG(0x44e0d040) |= 0x02; */
/*  */
/*    // Enable steps 1-4. STEPENABLE register */
/*    #<{(| HWREG(0x44e0d054) = 0x1e; |)}># */
/*       // Enable steps 1-6. STEPENABLE register */
/*       HWREG(0x44e0d054) = 0x7e; */
/*       // Enable steps 1-3. STEPENABLE register */
/*       #<{(| HWREG(0x44e0d054) = 0xe; |)}># */
/*       // Enable all steps. STEPENABLE register */
/*       #<{(| HWREG(0x44e0d054) |= 0xfe; |)}># */
/*  */
/*    // Enable Module (start sampling). ADC_CTRL register */
/*    HWREG(0x44e0d040) |= 0x01; */
}
