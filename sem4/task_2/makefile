CFLAGS = -Wall -D DEBUG

all: main.o calc.o
	gcc main.o calc.o -O3 -Wall -o prog

%.o: %.c
	gcc $< -c -o $@ -Wall -O3 -D DEBUG

clean:
	rm *.o prog
