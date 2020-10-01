#include <stdio.h>
#include <stdlib.h>

int writeNumber (char *string);
void check (char *main_pointer, char *pointer);

//------------------------------------------------------------------------

/*
long long length (char *string) {
    long long i = 0;
    while (*pointer != '\0') { pointer++; }
    return i;
}
*/

int main (int n_args, char **arg) {
    char *point = 0;
    long long test = strtoll (arg[1], &point, 10);
    printf ("%llu, %p", test, point);
    
}

//--------------------------------------------------------------------------
/*
int main (int n_args, char **arg) {
    printf ("%s", arg[1]);
    
    char string[100] = {};
    char string[0] = 1;

    while (strcmp() == false) {
        writeNumber (string);
    }
    return 0;
}
*/

/*
int writeNumber (char *string) {
    char* pointer = string;
    while (*pointer != 0) { ; }

    // if (pointer[-1] == '9') {
    //     pointer[-2] = 
    // }
    
    return 0;
}

void func (char *string) {
    if (string[0] == "1" && string[1] == '\0') {
        printf (string);
        return;
    }

    char* pointer = string;
    while (*pointer != '\0') { ; }

    check (string, pointer);
    
}

void check (char *main_pointer, char *pointer) {
    if (pointer[-1] == '0') {
        // if (pointer - 1 < main_pointer && pointer = ) 
    }
}
*/