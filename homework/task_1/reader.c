#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "common.h"

int main (int argc, char *argv[]) {
    fprintf (stderr, "name_file: [%s]\n", argv[1]);

    int file_fifo = open (argv[1], O_RDONLY);

    _Bool i_master = 0;
    size_t package_loaded = 0;
    struct Message message = {};

    char* array_of_data_messages = 0;
    size_t num_bytes_in_last_package = 0;
    while (1) {
        read (file_fifo, &message, sizeof(message));

        if (i_master == 0) {
            if (message.packege_number == 0) {
                i_master = 1;
                array_of_data_messages = (char*) calloc (message.total_packages, NUM_BYTES_DATA * sizeof (char)); 
            }
            else {
                break;
            }    
        }
        
        package_loaded++;

        if (message.used_symbols != NUM_BYTES_DATA)
            num_bytes_in_last_package = message.used_symbols;
            
        memcpy (&array_of_data_messages[message.packege_number * NUM_BYTES_DATA], message.data, message.used_symbols);

        if (package_loaded == message.total_packages)
            break;
    }

    if (array_of_data_messages != 0) {
        fprintf (stdout, array_of_data_messages, message.packege_number * NUM_BYTES_DATA + num_bytes_in_last_package);
        free (array_of_data_messages);
    }

    if (i_master == 0) {
        int addition_fifo = open (argv[1], O_WRONLY);
        write (addition_fifo, &message, sizeof (message));
        close (addition_fifo);
    }
    // read (file_fifo, buf, 5);
    // fprintf (stdout, "%s\n", buf);

    close (file_fifo);
    return 0;
}
