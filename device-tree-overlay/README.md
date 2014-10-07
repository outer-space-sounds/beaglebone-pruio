This device tree overlay enables the PRU system hardware and loads the appropriate kernel module.

*PIN CONFIGURATION IS DONE IN RUNTIME* from the PRU code so you have to be careful not to use the same pins that other capes/programs/kernel are using.

See [this](https://www.youtube.com/watch?v=wui_wU1AeQc) and [this](https://learn.adafruit.com/introduction-to-the-beaglebone-black-device-tree/device-tree-overlays) to understand how device tree overlays work. This one enables the PRU system so that it can be used by libbbb_pruio.

Also, take a look at the makefile to understand what's going on.
