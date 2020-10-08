#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <string.h>

#include "common.h"

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

    while (1) {
        int ret_num = read (file_fifo, &message, sizeof (message));
        if (ret_num == -1) {
            if (array_of_data_messages)
                free (array_of_data_messages);
                
            perror ("Error in read!");
            exit (EXIT_FAILURE);
        }

        fprintf (stderr, "Read zero!\n");
        
        if (ret_num == 0)
            continue;


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

        memcpy (&array_of_data_messages[message.packege_number * NUM_BYTES_DATA], message.data, message.used_symbols);
        if (package_loaded == message.total_packages)
            break;
    }

    if (array_of_data_messages != 0) {
        fprintf (stdout, "%s", array_of_data_messages);
        free (array_of_data_messages);
    }

    if (i_master == 0) {
        int addition_fifo = open (argv[1], O_WRONLY);
        if (addition_fifo == -1) {
            perror ("addition_fifo");
        }

        if (write (addition_fifo, &message, sizeof (message)) == -1) {
            perror ("Error");
        }
        close (addition_fifo);
    }

    if (i_master == 1) {
        remove (argv[1]);
    }

    close (file_fifo);
    return 0;
}
