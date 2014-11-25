# Lib BBB Pruio

This is a software library that allows access to the General Purpose Inputs and Outputs and the Digital Analog Converters in the [Beagle Bone Black single board computer](http://beagleboard.org/black).  

## Features

* Can be used as a library for programs written in the C language or as an external for [PureData](http://puredata.info) patches.
* Analog Digital Converters and General Purpose IO pins can be configured without the need of writing or configuring [Device Tree Overlays](https://learn.adafruit.com/introduction-to-the-beaglebone-black-device-tree?view=all).
* To avoid high CPU usage, input polling is done using one of the Programmable Real Time Units [PRUs](https://github.com/beagleboard/am335x_pru_package/blob/master/Documentation/01-AM335x_PRU_ICSS_Overview.pdf?raw=true) available in the [AM335X](http://www.ti.com/product/am3358) chip (the Beagle Bone Black's main processor).
* Easy to use. Users don't have to learn how to access the hardware features, no need to compile PRU code, etc.
* Tested only on a Beagle Bone Black running Debian Linux.

## Usage
 
### From a PD patch

![img](docs/images/lib-bbb-pruio-example.pd.png)

### From a C program

```C
#include <bbb_pruio.h>

int main(int argc, const char *argv[]){
    // Initialize library and hardware.
    bbb_pruio_start();

    // Get input from ADC channel 2.
    int bbb_pruio_init_adc_pin(2); 

    // Use pin P9_11 as an input.
    int bbb_pruio_init_gpio_pin("P9_11", 1); 

    // Use pin P9_12 as an output.
    int bbb_pruio_init_gpio_pin("P9_12", 0); 

    int output_value = 0;
    while(!exit_condition){
        // Get available messages from digital and analog inputs and parse them:
        while(bbb_pruio_messages_are_available()){
            unsigned int message;
            bbb_pruio_read_message(&message);

            // If input is from a gpio pin.
            if(bbb_pruio_message_is_gpio(&message)){
                int pin_number = bbb_pruio_get_gpio_number(&message);
                char pin_name[256];
                pin_name = bbb_pruio_get_pin_name(pin_number);
                int value = bbb_pruio_get_gpio_value(&message);
                printf("Digital pin %s changed to value %i \n", pin_name, value);
            }
            // Else, input is from an analog pin.
            else{
                int channel_number = bbb_pruio_get_adc_channel(&message);
                int value = bbb_pruio_get_adc_value(&message);
                printf("Analog pin %i changed to value %i \n", channel_number, value);
            }
        }

        // Toggle output pin
        output_value = !output_value;
        bbb_pruio_set_gpio_pin("P9_12", output_value); 

        sleep(1);
    }

    bbb_pruio_stop();
}

```

## More Info

More info in the [docs directory](docs/manual.md).

## License

Lib BBB Pruio

Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehemuerto.org>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.  

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
