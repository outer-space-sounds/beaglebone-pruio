#ifndef PINS_H
#define PINS_H

// These defines map the beagle bone's pin names to the AM335X's 
// GPIO numbers. GPIO numbers can be used like:
// 
//    int gpioModule = P9_11 >> 5;
//    int gpioPin = P9_11 % 32;

// #define P8_03 38 // emmc2
// #define P8_04 39 // emmc2
// #define P8_05 34 // emmc2
// #define P8_06 35 // emmc2
// #define P8_07 66 
// #define P8_08 67 
// #define P8_09 69 
// #define P8_10 68 
// #define P8_11 45 
// #define P8_12 44 
// #define P8_13 23 
// #define P8_14 26 
// #define P8_15 47 
// #define P8_16 46 
// #define P8_17 27 
// #define P8_18 65 
// #define P8_19 22 
// #define P8_20 63 // emmc2
// #define P8_21 62 // emmc2
// #define P8_22 37 // emmc2
// #define P8_23 36 // emmc2
// #define P8_24 33 // emmc2
// #define P8_25 32 // emmc2
// #define P8_26 61 
// #define P8_27 86 // hdmi
// #define P8_28 88 // hdmi
// #define P8_29 87 // hdmi
// #define P8_30 89 // hdmi
// #define P8_31 10 // hdmi
// #define P8_32 11 // hdmi
// #define P8_33 9  // hdmi
// #define P8_34 81 // hdmi
// #define P8_35 8  // hdmi
// #define P8_36 80 // hdmi
// #define P8_37 78 // hdmi
// #define P8_38 79 // hdmi
// #define P8_39 76 // hdmi
// #define P8_40 77 // hdmi
// #define P8_41 74 // hdmi
// #define P8_42 75 // hdmi
// #define P8_43 72 // hdmi
// #define P8_44 73 // hdmi
// #define P8_45 70 // hdmi
// #define P8_46 71 // hdmi

#define P9_11  30 
#define P9_12  60 
#define P9_13  31 
#define P9_14  50 
#define P9_15  48 
#define P9_16  51 
#define P9_17  5 
#define P9_18  4 
// #define P9_19  13 // i2c2
// #define P9_20  12 // i2c2
#define P9_21  3 
#define P9_22  2 
#define P9_23  49 
#define P9_24  15 
// #define P9_25  117 // mcasp0
#define P9_26  14 
#define P9_27  115 // mux_control
// #define P9_28  113 // mcasp0
// #define P9_29  111 // mcasp0
#define P9_30  112 // mux_control
// #define P9_31  110 // mcasp0
#define P9_41A 20  // ?? usable     ??
#define P9_41B 116 // ?? usable     ??
#define P9_42A 7   // mux_control
#define P9_42B 114 // mcasp0

#endif //PINS_H
