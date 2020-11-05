#include <sys/types.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

void receiveMsg (int msqid, void *msgp, size_t msgsz,
    long int msgtyp, int msgflg);

void sendMsg (int msqid, const void *msgp, size_t msgsz,
        int msgflg);

struct msgpack {
    long type;
    int number;
};

int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf (stderr, "Didn't give number of children!\n");
        exit (EXIT_FAILURE);
    }

    int N = atoi (argv[1]);
    key_t key = ftok ("children.c", 0);
    
    int fd = msgget (key, IPC_CREAT | 0666);
    if (fd == -1) {
        perror ("Msgget");
        exit (EXIT_FAILURE);
    }

    int ret = 0;
    for (int i = 0; i < N; i++)
    {
        ret = fork();
        if (ret == 0)
        {
            break;
        }
    }

    struct msgpack pack = {};

    if (ret == 0)
    {
        receiveMsg (fd, &pack, sizeof (struct msgpack) - sizeof (long), 0, 0);
        fprintf (stderr, "Data - %d\n", pack.number);
        
        if (pack.number != N - 1)
        {    
            pack.type = 1;
            pack.number += 1;
            sendMsg (fd, &pack, sizeof (struct msgpack) - sizeof (long), 0);
        }
        else
        {
            if (msgctl (fd, IPC_RMID, NULL) == -1)
            {
                perror ("Msgctl");
                exit (EXIT_FAILURE);
            }
        }
    } 
    else
    {
        pack.type = 1;
        pack.number = 0;
        sendMsg (fd, &pack, sizeof (struct msgpack) - sizeof (long), 0);
    }

    return 0;
}

void receiveMsg (int msqid, void *msgp, size_t msgsz,
    long int msgtyp, int msgflg)
{
    if (msgrcv (msqid, msgp, msgsz, msgtyp, msgflg) == -1)
    {
        perror ("Msgrcv");
        exit (EXIT_FAILURE);
    }
}

void sendMsg (int msqid, const void *msgp, size_t msgsz,
    int msgflg)
{
    if (msgsnd (msqid, msgp, msgsz, msgflg) == -1)
    {
        perror ("Msgsnd");
        exit (EXIT_FAILURE);
    }
}
           