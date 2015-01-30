
#####################################################################
# Constants
#

PREFIX?=/usr
# PREFIX?=/usr/local
# PREFIX?=/root/libbeaglebone_pruio/repo/library

# Make makefile silent, you can do `VERBOSE=1 make whatever` 
# to get messages
ifndef VERBOSE
.SILENT:
endif

PRU_COMPILER_DIR=../vendors/pru_2.0.0B2
PRU_C_FLAGS=--silicon_version=2 --hardware_mac=on \
				-i$(PRU_COMPILER_DIR)/include -i$(PRU_COMPILER_DIR)/lib 
PRU_LD_FLAGS=-llibc.a

HOST_C_FLAGS += -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a \
					 -fPIC -Isrc/ 
HOST_LD_FLAGS += -shared -Wl,-soname,libbeaglebone_pruio.so
HOST_LIBS += -lprussdrv

FIND_ADDRESS_0_COMMAND=`$(PRU_COMPILER_DIR)/bin/dispru src/pru0.elf \
							  | grep _c_int00 | cut -f1 -d\  `

######################################################################
# Compilation
#

all: lib/libbeaglebone_pruio.a

# 0. Compile and install device tree overlay
device-tree-overlay/PRUIO-DTO-00A0.dtb:
	cd ../device-tree-overlay && make && make install

# 1. Compile pru0_main.c into pru0_main.obj
src/pru0_main.obj: src/pru0_main.c
	$(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) \
		--output_file=src/pru0_main.obj -c src/pru0_main.c

# 2. Link pru0_main.obj with libraries and output pru0.map 
#    and pru0.elf
src/pru0.elf: src/pru0_main.obj
	$(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) -z src/pru0_main.obj \
		$(PRU_LD_FLAGS) -m src/pru0.map -o src/pru0.elf AM3359_PRU.cmd

# 3. Convert pru0.elf into libbeaglebone_pruio_text0.bin and
#    libbeaglebone_pruio_data0.bin
lib/libbeaglebone_pruio_text0.bin: src/pru0.elf
	$(PRU_COMPILER_DIR)/bin/hexpru bin.cmd src/pru0.elf \
		&& mv text.bin lib/libbeaglebone_pruio_text0.bin \
		&& mv data.bin lib/libbeaglebone_pruio_data0.bin

# 4. Compile beaglebone_pruio.c into beaglebone_pruio.o. BEAGLEBONE_PRUIO_START_ADDR_0 must be 
# obtained from pru0.elf and passed to gcc as a preprocessor define.
src/beaglebone_pruio.o: src/beaglebone_pruio.c src/pru0.elf lib/libbeaglebone_pruio_text0.bin
	export START_ADDR_0=0x$(FIND_ADDRESS_0_COMMAND) \
		&& gcc $(HOST_C_FLAGS) -DBEAGLEBONE_PRUIO_PREFIX=\"$(PREFIX)\" \
		-DBEAGLEBONE_PRUIO_START_ADDR_0=`echo $$START_ADDR_0` \
		-c -o src/beaglebone_pruio.o src/beaglebone_pruio.c

# 5. Link library
lib/libbeaglebone_pruio.a: src/beaglebone_pruio.o device-tree-overlay/LIB-BEAGLEBONE-PRUIO-DTO-00A0.dtb
	ar rcs lib/libbeaglebone_pruio.a src/beaglebone_pruio.o
	cp src/beaglebone_pruio.h include/
	cp src/beaglebone_pruio_pins.h include/
	gcc $(HOST_LD_FLAGS) -o lib/libbeaglebone_pruio.so src/beaglebone_pruio.o $(HOST_LIBS)



#####################################################################
# Utility commands
#

.PHONY: clean
clean:
	-rm src/*.obj 2> /dev/null
	-rm src/*.map 2> /dev/null
	-rm src/*.elf 2> /dev/null
	-rm src/*.o 2> /dev/null
	-rm lib/* 2> /dev/null
	-rm include/* 2> /dev/null

.PHONY: install
install:
	cp -f lib/* $(PREFIX)/lib
	cp -f include/* $(PREFIX)/include
	cd ../device-tree-overlay && make && make install

.PHONY: uninstall
uninstall:
	-rm $(PREFIX)/lib/libbeaglebone_pruio*
	-rm $(PREFIX)/include/beaglebone_pruio.h
	-rm $(PREFIX)/include/beaglebone_pruio_pins.h
