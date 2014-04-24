# PRU_COMPILER_DIR=../../vendors/pru_2.0.0B2
# PRU_C_FLAGS=--silicon_version=2 --hardware_mac=on -i$(PRU_COMPILER_DIR)/include -i$(PRU_COMPILER_DIR)/lib 
# PRU_LD_FLAGS=-llibc.a

HOST_C_FLAGS += -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a -Isrc/
# HOST_LD_FLAGS += /usr/local/lib/libprussdrv.a -lpthread
# HOST_LD_FLAGS += -lpthread

FIND_ADDRESS_COMMAND=`$(PRU_COMPILER_DIR)/bin/dispru pru.elf | grep _c_int00 | cut -f1 -d\  `

.PHONY: all
all:
# Compile pru.c into pro.obj
# $(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) -c pru.c
# Link pru.obj with libraries and output pru.map and pru.elf
# $(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) -z pru.obj $(PRU_LD_FLAGS) \
	-m pru.map -o pru.elf $(PRU_COMPILER_DIR)/example/AM3359_PRU.cmd
# Convert pru.elf into text.bin and data.bin
# $(PRU_COMPILER_DIR)/bin/hexpru $(PRU_COMPILER_DIR)/bin.cmd ./pru.elf
# Find address of start of program and compile host program
# export START_ADDR=0x$(FIND_ADDRESS_COMMAND) && 
# gcc $(HOST_C_FLAGS) -DSTART_ADDR=`echo $$START_ADDR` -c -o host.o host.c &&
	gcc $(HOST_C_FLAGS) -c -o src/bbb_pruio.o src/bbb_pruio.c
	ar rcs src/libbbb_pruio.a src/bbb_pruio.o

.PHONY: clean
clean:
# -rm *.obj
# -rm *.map
# -rm *.elf
# -rm *.bin
	-rm src/*.o
	-rm src/libbbb_pruio.a
