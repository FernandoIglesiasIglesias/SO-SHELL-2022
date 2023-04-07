CFLAGS = -g -O0 -Wall
all: list.o p3.o
	gcc $(CFLAGS) -o shell list.o p3.o

list.o:
	gcc $(CFLAGS) -c list.c

p3.o:
	gcc $(CFLAGS) -c p3.c 

clean:
	rm -f *.o shell
