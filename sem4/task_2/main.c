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
    printf ("Num threads = %ld", num_threads);

    StartCalculation (num_threads);
}