CFLAGS = -g -Wall
LDFLAGS = -g
CC = gcc

all: main 

main: main.o list.o
	$(CC) $(LDFLAGS) -o $@ $^

..c.o:
	gcc $(CFLAGS) -c $<

clean:
	rm *.o
	rm test
