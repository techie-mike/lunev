#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int args, char* argv[]) {
    execvp(argv[1], &(argv[1]));
    return 0;
}