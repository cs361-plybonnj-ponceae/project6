CC=gcc
CFLAGS=-Wall -I../include -pthread
MAKE=make

all: mt_mapper

mt_mapper: project6.o classify.o intqueue.o
	$(CC) $(CFLAGS) -o $@ project6.o classify.o intqueue.o -lm -lrt -pthread

project6.o: project6.c classify.h common.h intqueue.h
	$(CC) $(CFLAGS) -c $< -o $@

classify.o: classify.c classify.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

intqueue.o: intqueue.c intqueue.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY clean:
	@rm *.o mt_mapper
