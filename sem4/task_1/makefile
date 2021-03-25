all: a.out

a.out: main.o tree.o
	gcc --coverage $^ -o $@
	./$@
	lcov -c --base-directory .  --directory .  -o test.info
	genhtml test.info --output-directory out

%.o: %.c
	gcc $< -c -o $@ --coverage

clean:
	rm -f *.gcda *.gcno *.o