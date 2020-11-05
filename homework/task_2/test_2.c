#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>



int main() {
    key_t key_1 = ftok("test_1.c", 5);

    int fd_1 = msgget (key_1, 0);
    printf ("file descriptor 1 = %d\n", fd_1);

    char m[16] = {};
    msgrcv (fd_1, m, 8, 0, 0);

    printf ("Massage value = %d\n", *((char*)m + sizeof (long)));
    return 0;
}