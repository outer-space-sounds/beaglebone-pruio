#Lib BBB-PRU

TODO: THis readme file is not at all.


This is an example program to figure out how to program the PRU units in the Beagle Bone Black.

## 3rd party tools that are needed.

The vendors/am335x_pru_package directory includes a driver and assembler that can be used to generate and load binaries to be ran on the PRUs. The vendors/install.sh script takes care of compiling and installing it.

The vendors/pru_2.0.0B2 directory should contain the C compiler for generating binaries to run in the PRU. However TI's licence prevents redistribution of it so you have to manually download it from [here](http://software-dl.ti.com/codegen/non-esd/downloads/beta.htm) and run the installer script. This will generate a pru_2.0.0B2 directory which you'll have to move into vendors/pru_2.0.0B2. vendors/install.sh copies some files from here to system-wide directories for convenience.

## Writing PRU assembly code.

Look at the app-assembler example.

## Writing PRU C code

Look at the app-c example.

## Links:

http://www.embeddedrelated.com/showarticle/586.php   
http://www.embeddedrelated.com/showarticle/603.php   
https://github.com/texane/pru_sdk   
https://github.com/BeaglePilot/PRUSS-C   
https://github.com/beagleboard/am335x_pru_package   
https://github.com/VegetableAvenger/BBBIOlib
http://processors.wiki.ti.com/index.php/PRU_Assembly_Instructions   
http://processors.wiki.ti.com/index.php/PRU_Assembly_Reference_Guide   
[AM335x SitaraTM Processors Technical Reference Manual](http://www.ti.com/lit/ug/spruh73k/spruh73k.pdf)   


## Objectives

1. How to compile and run assembly code for the PRU?  [DONE]
2. How to communicate between a host application (linux app) and the code running in the PRU? Interrupt based, please. [DONE]
3. How to do all of this stuff in C instead of assembly?  [DONE]
4. How to use GPIOs? [DONE]
5. ADCs? 
5. Hardware Timers?

## License

GPLv3
