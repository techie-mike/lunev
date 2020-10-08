#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

int main () {

    printf ("-----------\n");
    int file = -1;
    while (file == -1) {
	    file = open ("temp_fifo", O_WRONLY | O_NONBLOCK);
        printf ("file - %d\n", file);
        sleep (1);
    }
    while (1) {
        int temp = write (file, "hello", 5); 
        printf ("temp = %d\n", temp);
        if (temp == -1) {
            perror ("eerroorrr");
            // exit (1);
        }
        printf ("write\n");
        sleep (4);
        
    }
    close (file);
    printf ("norm exit\n");
    return 0;
}