### 
# Beaglebone Pru IO 
# 
# Copyright (C) 2015 Rafael Vega <rvega@elsoftwarehamuerto.org> 
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###


# Make makefile silent, you can do `VERBOSE=1 make whatever` to get messages
ifndef VERBOSE
.SILENT:
endif

CFLAGS = -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a -I../library/include
LDFLAGS = -L../library/lib -lbeaglebone_pruio -lpthread

all: example

example: example.o
	cd ../library && make
	gcc $(CFLAGS) -o example example.o $(LDFLAGS)

example.o: example.c
	gcc $(CFLAGS) -c -o example.o example.c

library:
	cd ../library && make clean && make && make install

.PHONY:run
run:
	# LD_LIBRARY_PATH=../library/lib ./example
	./example

.PHONY:debugger
debugger:
	LD_LIBRARY_PATH=../library/lib gdb ./example

.PHONY:valgrind
valgrind:
	LD_LIBRARY_PATH=../library/lib valgrind ./example

.PHONY:clean
clean:
	-rm example.o 2> /dev/null
	-rm example 2> /dev/null
