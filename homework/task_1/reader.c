#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "common.h"
#include <errno.h>

int main (int argc, char *argv[]) {
    fprintf (stderr, "name_file: [%s]\n", argv[1]);

    int file_fifo = open (argv[1], O_RDONLY);

    if (file_fifo == -1) {
        fprintf (stderr, "Error in open!\n");
        exit (EXIT_FAILURE);
    }
    _Bool i_master = 0;
    size_t package_loaded = 0;
    struct Message message = {};

    char* array_of_data_messages = 0;
    size_t num_bytes_in_last_package = 0;

    // int count = 0;
    while (1) {
        int ret_num = read (file_fifo, &message, sizeof (message));
        if (ret_num == -1) {
            if (array_of_data_messages)
                free (array_of_data_messages);
            printf ("Error in read!\n");
            exit (EXIT_FAILURE);
        }

        if (ret_num == 0)
            continue;

        // for (int64_t i = 0; i < 100000000; i++);

        if (i_master == 0) {
            if (message.packege_number == 0) {
                i_master = 1;
                array_of_data_messages = (char*) calloc (message.total_packages * NUM_BYTES_DATA + 1 , sizeof (char));
                
                if (array_of_data_messages == NULL) {
                    printf ("Error in calloc!\n");
                    exit (EXIT_FAILURE);
                }            
            }
            else {
                break;
            }
        }


        if (ret_num != 0)
            package_loaded++;

        if (message.used_symbols != NUM_BYTES_DATA)
            num_bytes_in_last_package = message.used_symbols;

        memcpy (&array_of_data_messages[message.packege_number * NUM_BYTES_DATA], message.data, message.used_symbols);

        fprintf (stderr, "num_read = %d \t pack = %lu\n", ret_num, package_loaded);

        if (package_loaded == message.total_packages)
            break;

        // if (ret_num == 0) {
        //     count++;
        //     if (count > 10)
        //         exit (5);
        // }
    }

    if (array_of_data_messages != 0) {
        fprintf (stdout, "[%d] %s ------", getpid(),  array_of_data_messages/*, message.packege_number * NUM_BYTES_DATA + num_bytes_in_last_package */);
        free (array_of_data_messages);
    }

    if (i_master == 0) {
        int addition_fifo = open (argv[1], O_WRONLY);
        if (addition_fifo == -1) {
            perror ("addition_fifo");
        }
        // fcntl (file_fifo, F_SETFL, O_WRONLY);
        if (write (addition_fifo, &message, sizeof (message)) == -1) {
            fprintf (stderr, "error in NEW WRITE\n");
            perror ("ErroRRRR");
        }
        fprintf (stderr, "Open on write\nNEW mode: %d but should: %d\n", fcntl (addition_fifo, F_SETFL), O_WRONLY);
        close (addition_fifo);
    }
    // read (file_fifo, buf, 5);
    // fprintf (stdout, "%s\n", buf);

    if (i_master == 1) {
        remove (argv[1]);
        fprintf (stderr, "remove FIFO_FILE\n");
    }
    close (file_fifo);
    return 0;
}
