CC = gcc
CFLAGS = -Wall -g -lm

BINS = activecontours

all: $(BINS)

activecontours:  lab5.c
	$(CC) -o $(BINS) lab5.c $(CFLAGS)

clean:
	rm $(BINS)

cleanorig:
	rm *.orig
