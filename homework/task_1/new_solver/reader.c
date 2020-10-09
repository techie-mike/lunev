#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

#include <string.h>
#include <stdbool.h>
//-------------------------------------------------------------------

#include "common.h"

int openFifo (const char *name_fifo, const int flags);
void readNameFifo (char *buffer, int file);
int waitConnection (int write_fifo, char *name_fifo, char *key);
int openFifoWrite (const char *name_fifo);

int receivingMessages (int write_fifo, int read_fifo, char key); 

void sendKey (char key, int file);

int main() {
    int common_fifo = openFifo (NAME_COMMON_FIFO, O_RDONLY | O_NONBLOCK);
    char public_name_of_write[15] = {};
    readNameFifo (public_name_of_write, common_fifo);
    printf ("namefifo %s\n", public_name_of_write);
    //-------------------------------------------------------------------
    int public_writer_fifo = openFifo (public_name_of_write, O_WRONLY | O_NONBLOCK);
    
    printf ("open public fifo\n");

    char name_private_fifo_on_read[15] = {};
    sprintf (name_private_fifo_on_read, "fifo%d", getpid());

    char key = 0;
    int private_fifo_on_read = waitConnection (public_writer_fifo, name_private_fifo_on_read, &key);
    // We get fifo for read from it. 
    printf ("key %d\n", key);

    char name_private_fifo_on_write[16] = {};
    memcpy (name_private_fifo_on_write, public_name_of_write, 15);
    name_private_fifo_on_write[14] = '-';

    int private_fifo_on_write = openFifoWrite (name_private_fifo_on_write);

    printf ("open fifo write %d\n", private_fifo_on_write);

    // sendKey (key + 1, private_fifo_write);
    
    int ret_value = receivingMessages (private_fifo_on_write, private_fifo_on_read, key + 1);

    // write (file_description, &key, 1);
    close (common_fifo);
    close (public_writer_fifo);
    close (private_fifo_on_read);
    close (private_fifo_on_write);

    return ret_value;
}

//------------------------------------------------------_------------
int openFifo (const char* name_fifo, const int flags) {
    int file = open (name_fifo, flags);

    if (file == -1) {
        printf ("In \"%s\". ", name_fifo);
        perror ("Error in open fifo. May be writer busy");
        exit (EXIT_FAILURE);
        // It's enough, because fifo will be close to read only if writer found pair.
    }
    return file;
}

int openFifoWrite (const char* name_fifo) {
    int file = open (name_fifo, O_WRONLY | O_NONBLOCK);
    if (file == -1) {
        if (errno == ENOENT) {
            printf ("In \"%s\". ", name_fifo);
            perror ("");
            exit (EXIT_FAILURE);
        }

        if (errno == ENXIO) {
            while (open (name_fifo, O_WRONLY | O_NONBLOCK) == -1 && errno == ENXIO)
                printf ("Try open common fifo. Wait reader.\n");
            errno = 0;
        }

        if (errno != 0) {
            printf ("In \"%s\". ", name_fifo);
            perror ("Error in open fifo");
            exit (EXIT_FAILURE);
        }
    }
    return file;
}

void readNameFifo (char *buffer, int file) {
    while (1) {
        int ret_value = read (file, buffer, 15);

        if (ret_value > 0)
            return;
    }
}


int waitConnection (int write_fifo, char* name_fifo, char* key) {
    int ret_open = -1;
    
    for (size_t i = NUMBER_OF_ERRONEOUS_SENDS; 1; i++) {
        if (i == NUMBER_OF_ERRONEOUS_SENDS) { 
            i = 0;
            write (write_fifo, name_fifo, 15);

            // If write return error it doesn't matter to us,
            // because if all readers died, this process will be closed through "write".
        }
        
        if (ret_open == -1) {
            ret_open = open (name_fifo, O_RDONLY | O_NONBLOCK);
            if (ret_open == -1 && errno != ENOENT) {
                perror ("");
                exit (EXIT_FAILURE);
            }
        }
        else {
            int ret_read = read (ret_open, key, 1);
            printf ("ret_read %d\n", ret_read);
            
            if (ret_read > 0)
                return ret_open;
        }
        errno = 0;
    }
}

/*
void sendKey (char key, int file_description) {
    
    for (size_t i = NUMBER_OF_ERRONEOUS_SENDS; 1; i++) {
        if (i == NUMBER_OF_ERRONEOUS_SENDS) { 
            i = 0;
            write (file_description, &key, 1);

            // If write return error it doesn't matter to us,
            // because if all readers died, this process will be closed through "write".
        }
        
        int ret_read = read (fifo_to_read, received_name_fifo, 15);
        errno = 0;

        if (ret_read > 0)
            return;
    }
    


    int ret_value = write (file_description, &key, 1);

    if (ret_value == -1) {
        perror ("");
        exit (EXIT_FAILURE);
    }
}
*/

int receivingMessages (int write_fifo, int read_fifo, char key) {
    bool key_sended = false;
    size_t package_number = 0;
    struct Message message = {};

    for (size_t i = NUMBER_OF_ERRONEOUS_SENDS; 1; i++) {
        if (i == NUMBER_OF_ERRONEOUS_SENDS) { 
            i = 0;
            if (!key_sended) {
                write (write_fifo, &key, 1);
            }
            else {
                write (write_fifo, &package_number, sizeof (size_t));
                printf ("write %lu\n", package_number);
            }

            // If write return error it doesn't matter to us,
            // because if all readers died, this process will be closed through "write".
        }
        
        int ret_read = read (read_fifo, &message, sizeof (message));
        errno = 0;

        if (ret_read == 0) {
            fprintf (stderr, "Disconnect, data doesn't true!\n");
            return EXIT_FAILURE;
        }

        if (ret_read > 0) {
            if (package_number != message.package_number) {
                printf ("received data num pack %lu used %lu\n", message.package_number, message.used_symbols);
                fwrite (message.data, sizeof (char), message.used_symbols, stderr);
                package_number = message.package_number;
                if (message.package_number == 1 && !key_sended) {
                    // printf ("switch key sender\n");
                    key_sended = true;
                }
            }
            if (package_number == 0) 
                return 0;
            // exit (1);
        }
    }
}