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

// const int   NUMBER_OF_ERRONEOUS_SENDS = 10000;
const char  SECURITY_KEY              = 6;
// const char* NAME_COMMON_FIFO          = "/tmp/common_fifo"; 



//-------------------------------------------------------------------
int openCommonFifo();
int openFifoWithCreate (const char* name_fifo, const int flags);
int openFeedbackFifo   (const char* name_fifo);

void waitFeedbackToFirstConnect (int fifo_to_read,
                                 int fifo_to_write,
                                 char *received_name_fifo, 
                                 const char *my_name_fifo);

void waitFeedbackToFinishingConnect (int fifo_to_read, int fifo_to_write);

void sendData (int fifo_to_read, int fifo_to_write, const char* name_file);

int uploadText (FILE* file, struct Message* message, unsigned int package_number);

//-------------------------------------------------------------------

int main (int argv, const char *argc[]) {
    if (argv < 2) {
        printf ("Don't write arguments for \"writer\"!\nDon't give name of file!\n");
        exit (EXIT_FAILURE);
    }

    int common_fifo = openFifoWithCreate (NAME_COMMON_FIFO, O_WRONLY | O_NONBLOCK);

    pid_t my_pid = getpid();

    char name_public_feedback_fifo[15] = {};
    sprintf (name_public_feedback_fifo, "fifo%d", my_pid);

    int feedback_fifo = openFifoWithCreate (name_public_feedback_fifo, O_RDONLY | O_NONBLOCK);
    printf ("Common fifo %d\nSecond fifo %d\n", common_fifo, feedback_fifo);

    char name_fifo_of_writer[15] = {};

    waitFeedbackToFirstConnect (feedback_fifo, common_fifo,
        name_fifo_of_writer, name_public_feedback_fifo);

    printf ("name - %s\n", name_fifo_of_writer);


    char name_private_feedback_fifo[16] = {};
    memcpy (name_private_feedback_fifo, name_public_feedback_fifo,
        sizeof (name_public_feedback_fifo));
    
    name_private_feedback_fifo[14] = '-';

    int private_fifo_forward = openFifoWithCreate (name_fifo_of_writer,        O_WRONLY | O_NONBLOCK);
    int private_fifo_back    = openFifoWithCreate (name_private_feedback_fifo, O_RDONLY | O_NONBLOCK);

    printf ("First fifo - %d, Second fifo - %d\n", private_fifo_forward, private_fifo_back);
    waitFeedbackToFinishingConnect (private_fifo_back, private_fifo_forward);
    printf ("end wait\n");

    remove (name_public_feedback_fifo);
    close  (feedback_fifo);
    feedback_fifo = 0;

    printf ("some close done\n");
    sendData (private_fifo_back, private_fifo_forward, argc[1]);
    
    close (private_fifo_back);
    close (private_fifo_forward);
    remove (name_private_feedback_fifo);
    remove (name_fifo_of_writer);
    
    return 0;
}

/*
int openCommonFifo() {
    int file = open (NAME_COMMON_FIFO, O_WRONLY | O_NONBLOCK);
    if (file == -1) {
        if (errno == ENOENT) {
            if (mkfifo (NAME_COMMON_FIFO, 0666)) {
                perror ("Error in mkfifo");
                exit (EXIT_FAILURE);
            }
            errno = 0;
            
            file = open (NAME_COMMON_FIFO, O_WRONLY | O_NONBLOCK);

            if (file == -1 && errno == ENOENT) {
                perror ("Can't create and open file");
                exit (EXIT_FAILURE);
            }
        }

        if (errno == ENXIO) {
            while (open (NAME_COMMON_FIFO, O_WRONLY | O_NONBLOCK) == -1 && errno == ENXIO)
                printf ("Try open common fifo. Wait reader.\n");
            errno = 0;
        }

        if (errno != 0) {
            perror ("Error in open common fifo");
            exit (EXIT_FAILURE);
        }
    }
    return file;
}
*/

int openCommonFifo() {
    return openFifoWithCreate (NAME_COMMON_FIFO, O_WRONLY | O_NONBLOCK);
}


int openFifoWithCreate (const char* name_fifo, const int flags) {
    int file = open (name_fifo, flags);
    if (file == -1) {
        if (errno == ENOENT) {
            if (mkfifo (name_fifo, 0666)) {
                printf ("In \"%s\". ", name_fifo);
                perror ("Error in mkfifo");
                exit (EXIT_FAILURE);
            }
            errno = 0;
            
            file = open (name_fifo, flags);

            if (file == -1 && errno == ENOENT) {
                printf ("In \"%s\". ", name_fifo);
                perror ("Can't create and open file");
                exit (EXIT_FAILURE);
            }
        }

        if (errno == ENXIO) {
            while ((file = open (name_fifo, flags)) == -1 && errno == ENXIO);
            errno = 0;
            fprintf (stderr, "Open on write!\n");
        }

        if (errno != 0) {
            printf ("In \"%s\". ", name_fifo);
            perror ("Error in open fifo");
            exit (EXIT_FAILURE);
        }
    }
    return file;
}

int openFeedbackFifo (const char* name_fifo) {
    return openFifoWithCreate (name_fifo, O_RDONLY | O_NONBLOCK);
}

void waitFeedbackToFirstConnect (int fifo_to_read, int fifo_to_write,
        char *received_name_fifo, const char *my_name_fifo) {
    // int buffer;
    printf ("start feedback\n");
    for (size_t i = NUMBER_OF_ERRONEOUS_SENDS; 1; i++) {
        if (i == NUMBER_OF_ERRONEOUS_SENDS) { 
            i = 0;
            write (fifo_to_write, my_name_fifo, 15);
            // printf ("descriptor %d\n", fifo_to_write)
            // perror ("");

            // If write return error it doesn't matter to us,
            // because if all readers died, this process will be closed through "write".
        }
        
        int ret_read = read (fifo_to_read, received_name_fifo, 15);
        errno = 0;

        if (ret_read > 0)
            return;
    }
}

void waitFeedbackToFinishingConnect (int fifo_to_read, int fifo_to_write) {
    char buffer = 0;
    for (size_t i = NUMBER_OF_ERRONEOUS_SENDS; 1; i++) {
        if (i == NUMBER_OF_ERRONEOUS_SENDS) { 
            i = 0;
            write (fifo_to_write, &SECURITY_KEY, 1);
        }
        
        int ret_read = read (fifo_to_read, &buffer, sizeof (char));
        errno = 0;

        if (ret_read > 0) {
            printf("p------\n");
            if (buffer != (SECURITY_KEY + 1)) {
                printf ("Error on private fifo %d!\n", buffer);
                exit (EXIT_FAILURE);
            }
            printf ("received key %d\n", buffer);
            return;
        }
    }
}

void sendData (int fifo_to_read, int fifo_to_write, const char *name_file) {
    fprintf (stderr, "want open\n");

    FILE* file_with_text = fopen (name_file, "rb");
    if (file_with_text == 0) {
        perror (name_file);
        exit (EXIT_FAILURE);
    }
    
    fprintf (stderr, "open file\n");
    size_t package_number = 1;  // "0" using for "exit" message
    struct Message message = {};
    size_t buffer = 0;
    bool exit_flag = false;
    
    if (uploadText (file_with_text, &message, package_number) == -1) {
        fclose (file_with_text);
        return;
    }



    for (size_t i = NUMBER_OF_ERRONEOUS_SENDS; 1; i++) {
        // if (i == NUMBER_OF_ERRONEOUS_SENDS)
        //     printf ("i = %lu\n", i);
        if (i == NUMBER_OF_ERRONEOUS_SENDS) { 
            i = 0;
            // printf ("send mes %lu\n", message.package_number);
            write (fifo_to_write, &message, sizeof (message));
        }
        
        int ret_read = read (fifo_to_read, &buffer, sizeof (size_t));
        errno = 0;
        
        if (ret_read == 0) {
            if (!exit_flag) {
                fprintf (stderr, "Disconnect, data doesn't true!\n");
            }
            break;
        }

        if (ret_read > 0) {
            // printf ("read %lu\n", buffer);
            if (buffer == package_number) {
                package_number++;
                if (uploadText (file_with_text, &message, package_number) == -1) {
                    if (exit_flag == false) {
                        message.package_number = 0;
                        message.used_symbols   = 0;
                        exit_flag = true;
                    }
                    else {
                        break;
                    }
                } 
                else {
                    printf ("upload data pack_num %lu\n", package_number);
                }
            }
            
            
            i = NUMBER_OF_ERRONEOUS_SENDS - 1; // Because for() will do i++.
            // printf ("package_number %lu\n", package_number);
        }
    }
    fclose (file_with_text);
}

int uploadText (FILE* file, struct Message* message, unsigned int package_number) {
    size_t num_read = fread (message->data, sizeof (char), 10, file);
    if (num_read == 0) {
        return -1;
    }
    
    message->package_number = package_number;
    message->used_symbols   = num_read;
    return 0;
}
