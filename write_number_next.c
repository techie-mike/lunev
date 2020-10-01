#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

int main (int n_args, char **arg) {
    char *endptr = 0;
    errno = 0;
    long long number = strtoll (arg[1], &endptr, 10);

    if ((errno == ERANGE && (number == LLONG_MAX || number == LLONG_MIN))
                   || (errno != 0 && number == 0)) {
               printf ("Error, number of range!\n");
               exit(1);
    }

    for (long long i = 1; i <= number; i++) {
        printf("%lli ", i);
    }
}