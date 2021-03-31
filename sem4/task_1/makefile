all: a.out

a.out: main.o tree.o test_tree.o
	gcc --coverage $^ -o $@ -Wall -Werror
	./$@
	lcov -c --base-directory .  --directory .  -o test.info
	genhtml test.info --output-directory out
	browse ./out/task_1/index.html

%.o: %.c
	gcc $< -c -o $@ --coverage -Wall -Werror

test_tree.o: test_tree.c
	gcc $< -c -o $@ -Wall -Werror


clean:
	rm -f *.gcda *.gcno *.o a.out
