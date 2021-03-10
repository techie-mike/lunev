#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "common.h"

void createFifo (const char* name);

void waitResponse (const char* name_public_fifo, int fd_public_feedback_fifo);

int main() {
    //-------------------------------------------------------------------------
    createFifo ("temp_fifo");
    int fd_common_fifo = open ("temp_fifo", O_WRONLY);
    if (fd_common_fifo == -1) {
        fprintf (stderr, "Error in open!\n");
        exit (EXIT_FAILURE);
    }
    //-------------------------------------------------------------------------


    //-------------------------------------------------------------------------
    char name_public_feedback_fifo[15] = {};
    sprintf (name_public_feedback_fifo, "fifo%d", getpid());

    createFifo (name_public_feedback_fifo);
    int fd_public_feedback_fifo = open (name_public_feedback_fifo, O_RDONLY | O_NONBLOCK);
    if (fd_public_feedback_fifo == -1) {
        fprintf (stderr, "Error in open!\n");
        exit (EXIT_FAILURE);
    }



    write (fd_common_fifo, name_public_feedback_fifo, 15);
    //-------------------------------------------------------------------------

    waitResponse (name_public_feedback_fifo, fd_public_feedback_fifo);

    if (fcntl (fd_public_feedback_fifo, O_RDONLY) == -1) {
        fprintf (stderr, "Error in fcntl!\n");
        exit (EXIT_FAILURE);
    }

    ssize_t ret_val = 0;
    char buffer[SIZE_BUFFER_PACKEGE] = {};
    while (1) {
        errno = 0;
        ret_val = read (fd_public_feedback_fifo, buffer, SIZE_BUFFER_PACKEGE * sizeof(char));
        if (ret_val < 0 || errno != 0)
        {
            fprintf (stderr, "Read from buff error\n");
            exit (EXIT_FAILURE);
        }

        write (STDERR_FILENO, buffer, ret_val);
        
        if (ret_val == 0)
            break;
    }

    remove (name_public_feedback_fifo);
    return 0;
}

void createFifo (const char* name) {
    if (mkfifo (name, 0666) == -1 && errno != EEXIST) {
        fprintf (stderr, "Error in mkfifo!\n");
        exit (EXIT_FAILURE);
    }
}

void waitResponse (const char* name_public_fifo, int fd_public_fifo) {
    char buffer[SIZE_BUFFER_PACKEGE] = {};
    long ret = 0;

    for (size_t i = 0; i < NUMBERS_OF_ATTEMPTS; i++) {
        ret = read (fd_public_fifo, buffer, SIZE_BUFFER_PACKEGE);

        if (ret == -1) {
            fprintf (stderr, "Error in wait response!\n");
            exit (EXIT_FAILURE);
        }

        if (ret > 0) {
            write (STDERR_FILENO, buffer, ret);
            break;
        }
        sleep (1);
    }

    if (ret == 0) {
        fprintf (stderr, "Can't connect to writer!\n");
        exit (EXIT_FAILURE);
    }
}

