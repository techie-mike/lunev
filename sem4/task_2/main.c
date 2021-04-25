#include <stdio.h>
#include "calc.h"

int main (int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf ("Error: don't give num threads!\n");
        exit (EXIT_FAILURE);
    }

    int64_t num_threads = GetNumber (argv[1]);
    if (num_threads == ERROR)
    {
        PRINT_ERROR("not valid num of threads!");
        return -1;
    }
    printf ("Num threads = %ld\n", num_threads);

    StartCalculation (num_threads);
}