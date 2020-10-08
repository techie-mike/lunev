#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

int main () {
    printf ("----\n");
	int file = open ("temp_fifo", O_RDONLY | O_NONBLOCK);
    printf ("file - %d\n", file);

    while (1) {
        char buf[5] = {};
        int temp = read (file, buf, 5);
        printf ("temp %d\n", temp);

        if (temp == -1) {
            perror ("");
            // exit(1);
        }
        printf ("read\n");
        sleep (1);

    }
    return 0;
}