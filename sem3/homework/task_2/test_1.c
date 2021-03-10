#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <errno.h>

struct msgpack {
    long type;
    char data[8];
};


int main() {
    key_t key_1 = ftok ("test_1.c", 5);

    int fd_1 = msgget (key_1, 0 /* IPC_CREAT | 0666 */);
    printf ("file descriptor 1 = %d\n", fd_1);
    // perror ("e");

    struct msgpack pack = {};
    pack.type = 1;
    pack.data[0] = 16;

    msgsnd (fd_1, &pack, sizeof (struct msgpack) - sizeof (long), 0);
    return 0;
}