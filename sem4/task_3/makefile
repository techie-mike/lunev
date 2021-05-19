all: worker.out manager.out

manager.out: manager.o
	gcc -MD -Wall $< -o $@

worker.out: worker.o calc.o
	gcc -MD -Wall -pthread  $^ -o $@ 

calc.o: calc.c
	gcc -MD -Wall -pthread $< -c -o $@
%.o: %.c
	gcc -MD -Wall $< -c -o $@

clean:
	rm *.o *.d *.out