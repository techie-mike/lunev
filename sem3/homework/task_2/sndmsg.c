#include <sys/types.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

struct msgpack {
    long type;
    int number;
};

int main() {
    key_t key = ftok ("children.c", 0);
    
    int fd = msgget (key, IPC_CREAT | 0666);
    if (fd == -1) {
        perror ("Msgget");
        exit (1);
    }

    struct msgpack pack = {};

    printf ("parent\n");
        pack.type = 1;
        pack.number = 0;
        msgsnd (fd, &pack, sizeof (struct msgpack) - sizeof (long), 0);
}