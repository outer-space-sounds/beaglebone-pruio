
# PREFIX=/usr/local
PREFIX=/root/libbbb_pruio/repo

################################################################################
# Constants
#

PRU_COMPILER_DIR=vendors/pru_2.0.0B2
PRU_C_FLAGS=--silicon_version=2 --hardware_mac=on -i$(PRU_COMPILER_DIR)/include \
				-i$(PRU_COMPILER_DIR)/lib 
PRU_LD_FLAGS=-llibc.a

HOST_C_FLAGS += -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a -fPIC -Isrc/ 
HOST_LD_FLAGS += -shared -Wl,-soname,libbbb_pruio.so
HOST_LIBS += -lprussdrv -lpthread

FIND_ADDRESS_0_COMMAND=`$(PRU_COMPILER_DIR)/bin/dispru src/pru0.elf \
							  | grep _c_int00 | cut -f1 -d\  `

################################################################################
# Compilation
#

all: lib/libbbb_pruio.a

# 0. Compile and install device tree overlay
device-tree-overlay/LIB-BBB-PRUIO-DTO-00A0.dtb:
	cd device-tree-overlay && make && make install

# 1. Compile pru0_main.c into pru0_main.obj
src/pru0_main.obj: src/pru0_main.c
	$(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) --output_file=src/pru0_main.obj \
		-c src/pru0_main.c

# 2. Link pru0_main.obj with libraries and output pru0.map and pru0.elf
src/pru0.elf: src/pru0_main.obj
	$(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) -z src/pru0_main.obj \
		$(PRU_LD_FLAGS) -m src/pru0.map -o src/pru0.elf \
		$(PRU_COMPILER_DIR)/example/AM3359_PRU.cmd

# 3. Convert pru0.elf into libbbb_pruio_text0.bin and libbbb_pruio_data0.bin
lib/libbbb_pruio_text0.bin: src/pru0.elf
	$(PRU_COMPILER_DIR)/bin/hexpru $(PRU_COMPILER_DIR)/bin.cmd src/pru0.elf \
		&& mv text.bin lib/libbbb_pruio_text0.bin \
		&& mv data.bin lib/libbbb_pruio_data0.bin

# 4. Compile bbb_pruio.c into bbb_pruio.o. BBB_PRUIO_START_ADDR_0 must be 
# obtained from pru0.elf and passed to gcc as a preprocessor define.
src/bbb_pruio.o: src/bbb_pruio.c src/pru0.elf lib/libbbb_pruio_text0.bin
	export START_ADDR_0=0x$(FIND_ADDRESS_0_COMMAND) \
		&& gcc $(HOST_C_FLAGS) -DBBB_PRUIO_PREFIX=\"$(PREFIX)\" \
		-DBBB_PRUIO_START_ADDR_0=`echo $$START_ADDR_0` \
		-c -o src/bbb_pruio.o src/bbb_pruio.c

# 5. Link library
lib/libbbb_pruio.a: src/bbb_pruio.o device-tree-overlay/LIB-BBB-PRUIO-DTO-00A0.dtb
	ar rcs lib/libbbb_pruio.a src/bbb_pruio.o
	cp src/bbb_pruio.h include/
	gcc $(HOST_LD_FLAGS) -o lib/libbbb_pruio.so src/bbb_pruio.o $(HOST_LIBS)

#####################################################################
# Util
#

.PHONY: clean
clean:
	-rm src/*.obj
	-rm src/*.map
	-rm src/*.elf
	-rm src/*.o
	-rm lib/*
	-rm include/*

.PHONY: install
install:
	cp lib/* $(PREFIX)/lib
	cp include/* $(PREFIX)/include
	cd device-tree-overlay && make && make install

.PHONY: uninstall
uninstall:
	rm $(PREFIX)/lib/libbbb_pruio*
	rm $(PREFIX)/include/bbb_pruio.h
	cd make uninstall
