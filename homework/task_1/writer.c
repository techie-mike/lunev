#include <linux/random.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>

#include "common.h"


void createSpecialNameFile (char name_file[], size_t num_symbols);
long lengthOfFile (FILE* file);
void checkOpenFile (FILE* file);

int main (int argc, char *argv[]) {

    // if (argc != 2) {
    //     fprintf(stderr, "Didn't give name of file!");
    //     exit(EXIT_FAILURE);
    // }

    char name_file[11] = {};

    createSpecialNameFile (name_file, 10);

    fprintf (stdout, "%s\n", name_file);

    if (mkfifo (name_file, S_IRUSR | S_IWUSR)) {
        printf ("Error in mkfifo!\n");
        exit (1);
    }

    // for (int i = 0; i < 1000000; i++);


    // write (stdout, "UVBHVECVHN", 11)
    // for (int i = 0; i < 9000000; i++);

    int file_fifo = open (name_file, O_WRONLY);
    checkOpenFile (file_fifo);

    struct Message message = {};

    FILE* file_with_text = fopen (argv[1], "rb");
    checkOpenFile (file_with_text);

    long num_message = lengthOfFile (file_with_text) / NUM_BYTES_DATA;

    fprintf (stderr, "num_message: %li\n", num_message + 1);
    for (size_t i = 0; i <= num_message; i++) {
        size_t num_read = fread (message.data, sizeof(char),  NUM_BYTES_DATA, file_with_text);
        
        if (num_read == 0)
            break;
        
        message.used_symbols = num_read;
        message.total_packages = num_message + 1;
        message.packege_number = i;
        message.serial_number = 1;

        write (file_fifo, &message, sizeof(message));
    }

    fclose (file_with_text);
    close (file_fifo);
    remove (name_file);
    return 0;
}

void createSpecialNameFile (char name_file[], size_t num_symbols) {
    if (num_symbols != getrandom (name_file, num_symbols, GRND_RANDOM)) {
        printf ("Error in getrandom\n");
        exit(1);
    }

    for (size_t i = 0; i < num_symbols; i++) {
        if (name_file[i] < 0)
            name_file[i] = 256 - name_file[i];

        name_file[i] = 'A' + name_file[i] % ('Z' - 'A');
    }
    // sprintf (name_file, "%d", key);
    // printf ("name %s: ", name_file);
}

long lengthOfFile (FILE* file) {
    fseek (file, 0, SEEK_END);
    long length_of_file = ftell (file);
    fseek (file, 0, SEEK_SET);
    return length_of_file;
}

void checkOpenFile (FILE* file) {
    if (file == -1) {
        printf ("Error in open!\n");
        exit(1);
    }
}