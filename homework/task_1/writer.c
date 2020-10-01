#include <linux/random.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>

#include "common.h"


void createSpecialNameFile (char name_file[]);
long lengthOfFile (int file);
void checkOpenFile (int file);


int main (int argc, char *argv[]) {
    if (argc != 2) {
        printf ("Didn't give name of file!");
        exit (EXIT_FAILURE);
    }

    // Number "fifo" and 33 bytes
    char name_file[37] = {};

    createSpecialNameFile (name_file);

    fprintf (stdout, "%s\n", name_file);

    if (mkfifo (name_file, S_IRUSR | S_IWUSR)) {
        printf ("Error in mkfifo!\n");
        exit (1);
    }


    int file_fifo = open (name_file, O_WRONLY);
    checkOpenFile (file_fifo);

    struct Message message = {};

    int file_with_text = open (argv[1], O_RDONLY);
    checkOpenFile (file_with_text);


    long length_of_file = lengthOfFile (file_with_text);

    fprintf (stdout, "Length file %ld\n", length_of_file);
    

    if (length_of_file == -1) {
        printf ("Error in stat!\n");
        close (file_fifo);
        close (file_with_text);
        exit (EXIT_FAILURE);
    }

    long num_message = length_of_file / NUM_BYTES_DATA;

    fprintf (stderr, "num_message: %li\n", num_message + 1);
    for (size_t i = 0; i <= num_message; i++) {
        size_t num_read = read (file_with_text, message.data,  NUM_BYTES_DATA);
        
        if (num_read == 0)
            break;
        
        message.used_symbols = num_read;
        message.total_packages = num_message + 1;
        message.packege_number = i;
        message.serial_number = 1;

        if (write (file_fifo, &message, sizeof (message)) == -1) {
            printf ("Error in write!\n");
            exit (EXIT_FAILURE);
        }
    }

    close (file_with_text);
    close (file_fifo);
    return 0;
}

void createSpecialNameFile (char name_file[]) {

    sprintf (name_file, "fifo%d", 12312/*getpid()*/);
}

long lengthOfFile (int file) {
    struct stat stat_file = {};

    if (fstat (file, &stat_file) == -1) {
        return -1;
    }

    return stat_file.st_size;
}

void checkOpenFile (int file) {
    if (file == -1) {
        printf ("Error in open!\n");
        exit (EXIT_FAILURE);
    }
}
