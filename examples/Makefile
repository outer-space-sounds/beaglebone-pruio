CFLAGS = -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a -I../src
LDFLAGS = -L../src -lbbb_pruio -lpthread

all: example

example: example.o
	gcc $(CFLAGS) -o example example.o $(LDFLAGS)

example.o: example.c
	gcc $(CFLAGS) -c -o example.o example.c

.PHONY:clean
clean:
	-rm example.o
	-rm example
