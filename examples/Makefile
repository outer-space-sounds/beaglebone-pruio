CFLAGS = -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a -I../include
LDFLAGS = -L../lib -lbbb_pruio

all: example

example: example.o
	gcc $(CFLAGS) -o example example.o $(LDFLAGS)

example.o: example.c
	gcc $(CFLAGS) -c -o example.o example.c

library:
	cd .. && make 
# cd .. && make clean && make

.PHONY:run
run:
	LD_LIBRARY_PATH=../lib ./example

.PHONY:clean
clean:
	-rm example.o
	-rm example
