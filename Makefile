CC = gcc
CFLAGS = -Wall -g -std=c99 -Werror

all: myprog

myprog: stats_functions.o a1_main.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c stats_functions.h 
	$(CC) $(CFlAGS) -c  $<


.PHONY: clean
clean:
	rm *.o
	rm myprog


