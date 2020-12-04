// #define _POSIX_C_SOURCE

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

// #include <sys/ipc.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>




#define CHECK(code)             \
    do {                        \
        if (code == -1) {       \
            perror (#code);     \
            exit (EXIT_FAILURE);\
        }                       \
    } while (0)

static char received_bit = 0;

void childReceiveBack (int signum);
void parentDie (int signum);
void childDie (int signum);

void receiveZero (int signum);
void receiveOne  (int signum);

int main (int argv, const char* argc[])
{
    if (argv < 2)
    {
        fprintf (stderr, "Didn't get text file\n");
        exit (EXIT_FAILURE);
    }

    sigset_t mask;
    CHECK (sigemptyset (&mask));

    CHECK (sigaddset (&mask, SIGUSR1));
    CHECK (sigaddset (&mask, SIGUSR2));
    CHECK (sigaddset (&mask, SIGCHLD));

    CHECK (sigprocmask (SIG_SETMASK, &mask, NULL));

    pid_t pid_true_parent = getpid();

    pid_t child_pid = fork();
    CHECK (child_pid);

    if (child_pid == 0)
    {
        CHECK (prctl (PR_SET_PDEATHSIG, SIGCHLD));

        if (pid_true_parent != getppid())
        {
            fprintf (stderr, "parent changed\n");
            exit (EXIT_FAILURE);
        }

        // fprintf (stderr, "open file\n");

        FILE* file = fopen (argc[1], "rb");
        if (file == NULL)
        {
            perror ("error in open file\n");
            exit (EXIT_FAILURE);
        }

        struct sigaction act_child = {};

        act_child.sa_handler = childReceiveBack;
        CHECK (sigaction (SIGUSR1, &act_child, NULL));

        // In SIGCHL will be signal, that parent die 
        // (see "prctl" before) 
        act_child.sa_handler = parentDie;
        CHECK (sigaction (SIGCHLD, &act_child, NULL));

        //-------------------------------------------------
        sigset_t child_mask;
        CHECK (sigfillset (&child_mask));
        CHECK (sigdelset  (&child_mask, SIGUSR1));
        CHECK (sigdelset  (&child_mask, SIGUSR2));
        CHECK (sigdelset  (&child_mask, SIGCHLD));
        // For working CTRL + C
        CHECK (sigdelset  (&child_mask, SIGINT));


        char byte = 0;

        while (fread (&byte, sizeof (char), 1, file) > 0)
        {
            // fprintf (stderr, "read bytes\n", byte);

            unsigned char bit_mask = 0x80;
            // fprintf (stderr, "Send byte %x\n", byte);

            for (int i = 0; i < 8; i++)
            {
                // fprintf (stderr, "\t\tSend bit %x\n", bit_mask & byte);
                if ((bit_mask & byte) == 0)
                    kill (pid_true_parent, SIGUSR1);
                else
                    kill (pid_true_parent, SIGUSR2);

                bit_mask /= 2;
                sigsuspend (&child_mask);
                errno = 0;
            }
        }
    }
    else
    {
        struct sigaction act_parent = {};

        act_parent.sa_handler = childDie;
        // Need to ignore SIGSTOP, SIGCONT and other by SIGCHLD
        act_parent.sa_flags = SA_NOCLDSTOP;
        CHECK (sigaction (SIGCHLD, &act_parent, NULL));

        act_parent.sa_handler = receiveZero;
        act_parent.sa_flags = 0;
        CHECK (sigaction (SIGUSR1, &act_parent, NULL));

        act_parent.sa_handler = receiveOne;
        act_parent.sa_flags = 0;
        CHECK (sigaction (SIGUSR2, &act_parent, NULL));

        //-------------------------------------------------
        sigset_t parent_mask;
        CHECK (sigfillset (&parent_mask));
        CHECK (sigdelset  (&parent_mask, SIGUSR1));
        CHECK (sigdelset  (&parent_mask, SIGUSR2));
        CHECK (sigdelset  (&parent_mask, SIGCHLD));
        // For working CTRL + C
        CHECK (sigdelset  (&parent_mask, SIGINT));

        // fprintf (stderr, "in while\n");
        while (1)
        {
            unsigned char finish_byte = 0;
            for (int i = 0; i < 8; i++)
            {
                sigsuspend (&parent_mask);
                errno = 0;
                // fprintf (stderr, "\tReceive bit %x\n", received_bit);

                finish_byte <<= 1;
                finish_byte |= received_bit;

                CHECK (kill (child_pid, SIGUSR1));
            }
            // fprintf (stderr, "Receive byte %x\n", finish_byte);
            CHECK (write (STDOUT_FILENO, &finish_byte, 1));
        }
    }

}

void childReceiveBack (int signum)
{
    return;
}


void parentDie (int signum)
{
    fprintf (stderr, "Parent is died\n");
    exit (EXIT_FAILURE);
}

void childDie (int signum)
{
    int stat_val = 0;

    CHECK (wait (&stat_val));

    if (WIFEXITED (stat_val))
    {
        exit (EXIT_SUCCESS);
    }
    else 
    {
        fprintf (stderr, "Child with text die!\n");
        exit (EXIT_FAILURE);
    }
}

void receiveZero (int signum)
{
    received_bit = 0;
}

void receiveOne (int signum)
{
    received_bit = 1;
}