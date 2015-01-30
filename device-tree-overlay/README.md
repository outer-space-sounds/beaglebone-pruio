This device tree overlay for the Beagle Bone Black turns on the PRU0 system and allows for changing the pinmux configuration during runtime by writing to the filesystem (no need to modify or load a DTO).

Setting a pin to input sets its pinmux register to 0x37 (input with pullup resistor). Setting it to output is 0x0F (output, no pullup, no pulldown).

To use:

    cd libbeaglebone_pruio/device-tree-overlay
    make load
    cd /sys/devices/ocp.3/P9_11_mux.12   # Each pin has it's directory
    echo input > state                   # input or output are valid options
    cat state

To verify that the register values are actually beign changed in the hardware, you can check the output of:

    cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins

This is largely based in [this](https://github.com/cdsteinkuehler/beaglebone-universal-io) and the dts file in [this project](http://www.freebasic-portal.de/downloads/fb-on-arm/libpruio-325.html). Also see [this](https://www.youtube.com/watch?v=wui_wU1AeQc) and [this](https://learn.adafruit.com/introduction-to-the-beaglebone-black-device-tree/device-tree-overlays) to understand how device tree overlays work.
