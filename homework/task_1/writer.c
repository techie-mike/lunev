#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

void createFifo (const char* name);

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        fprintf (stderr, "Didn't give name of file!\n");
        exit (EXIT_FAILURE);
    }

    int text_file = open (argv[1], O_RDONLY);

    createFifo ("temp_fifo");
    int fd_common_fifo = open ("temp_fifo", O_RDONLY);

    char name_feedback_fifo[15] = {};
    ssize_t ret_read = 0;
    ret_read = read (fd_common_fifo, name_feedback_fifo, 15);

    if (ret_read == 0) {
        fprintf (stderr, "Error in read new fifo!\n");
        exit (EXIT_FAILURE);
    }
    // printf ("ret = %d fifo - %s\n", ret_read, name_feedback_fifo);

    int fd_feedback_fifo = open (name_feedback_fifo, O_WRONLY | O_NONBLOCK);
    if (fd_feedback_fifo == -1) {
        fprintf (stderr, "Error in open!\n");
        exit (EXIT_FAILURE);
    }

    if (fcntl (fd_feedback_fifo, O_WRONLY) == -1) {
        fprintf (stderr, "Error in fcntl!\n");
        exit (EXIT_FAILURE);
    }

    char buffer[SIZE_BUFFER_PACKEGE] = {};
    while (1) {
        errno = 0;
        ret_read = read (text_file, buffer, SIZE_BUFFER_PACKEGE);
        if (ret_read < 0)
        {
            fprintf (stderr, "Read from file error\n");
            exit (EXIT_FAILURE);
        }

        errno = 0;
        ssize_t ret_write = write (fd_feedback_fifo, buffer, ret_read * sizeof(char));
        if (ret_write <= 0 && errno == EPIPE)
        {
            fprintf (stderr, "Died transfer fifo\n");
            exit (EXIT_FAILURE);
        }
        if (ret_write < 0)
        {
            fprintf (stderr, "Write to transfer fifo error\n");
            exit (EXIT_FAILURE);
        }

        if (ret_read <= 0) {
            break;
        }
    }

    close (text_file);
    close (fd_common_fifo);

    return 0;
}

void createFifo (const char* name) {
    if (mkfifo (name, 0666) == -1 && errno != EEXIST) {
        fprintf (stderr, "Error in mkfifo!\n");
        exit(EXIT_FAILURE);
    }
}

