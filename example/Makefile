CFLAGS = -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a -I../library/include
LDFLAGS = -L../library/lib -lbbb_pruio

all: example

example: example.o
	cd ../library && make
	gcc $(CFLAGS) -o example example.o $(LDFLAGS)

example.o: example.c
	gcc $(CFLAGS) -c -o example.o example.c

library:
	cd ../library && make clean && make

.PHONY:run
run:
	LD_LIBRARY_PATH=../library/lib ./example

.PHONY:clean
clean:
	-rm example.o
	-rm example 
