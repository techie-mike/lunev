CFLAGS = -Wall -pthread

all: main.o calc.o
	gcc main.o calc.o $(CFLAGS) -o prog

%.o: %.c %.h
	gcc $< -c -o $@ $(CFLAGS) 

main.o: main.c calc.h
	gcc $< -c -o $@ $(CFLAGS)

clean:
	rm *.o prog
