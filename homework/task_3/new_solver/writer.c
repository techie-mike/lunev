#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "common.h"

int main (int argc, const char* argv[])
{
    createCommonFile();

    if (argc < 2)
    {
        fprintf (stderr, "Didn't give file!\n");
        exit (EXIT_FAILURE);
    }

    int fd_text = open (argv[1], O_RDONLY);
    if (fd_text == -1)
    {
        fprintf (stderr, "Can't open file with data!\n");
        exit (EXIT_FAILURE);
    }

    key_t key = ftok (NAME_COMMON_FILE, 0);
    if (key == -1)
    {
        perror ("Error in ftok");
        exit (EXIT_FAILURE);
    }

    //-----------------INIT-SEMAPHORES---------------------
    int sem_common_id = semget (key, 2, IPC_CREAT | IPC_EXCL | 0666);
    // If he creater, default value == 0, so he get MUTEX

    if (sem_common_id == -1)
    {
        if (errno == EEXIST)
        {    
            sem_common_id = semget (key, 2, 0);
        }
        else
        {
            perror ("Error in semget!\n");
            exit (EXIT_FAILURE);
        }
    }    
    //-----------------INIT-SEMAPHORES---------------------

    struct sembuf mutex_buf[10] = {};

    mutex_buf[0].sem_num = SEM_HAVE_WRITER;
    mutex_buf[0].sem_op  = 0;
    mutex_buf[0].sem_flg = 0;

    mutex_buf[1].sem_num = SEM_HAVE_WRITER;
    mutex_buf[1].sem_op  = 1;
    mutex_buf[1].sem_flg = SEM_UNDO;

    mutex_buf[2].sem_num = SEM_HAVE_READER;
    mutex_buf[2].sem_op  = -1;
    mutex_buf[2].sem_flg = 0;

    mutex_buf[3].sem_num = SEM_HAVE_READER;
    mutex_buf[3].sem_op  = 1;
    mutex_buf[3].sem_flg = 0;

    mutex_buf[4].sem_num = SEM_CONNECT;
    mutex_buf[4].sem_op  = -1;
    mutex_buf[4].sem_flg = 0;
    
    //-----------------------------------------------------

    mutex_buf[5].sem_num = SEM_FULL;
    mutex_buf[5].sem_op  = 1;
    mutex_buf[5].sem_flg = 0;

    mutex_buf[6].sem_num = SEM_FULL;
    mutex_buf[6].sem_op  = -1;
    mutex_buf[6].sem_flg = SEM_UNDO;

    mutex_buf[7].sem_num = SEM_EMPTY;
    mutex_buf[7].sem_op  = 1;
    mutex_buf[7].sem_flg = SEM_UNDO;

    mutex_buf[8].sem_num = SEM_EMPTY;
    mutex_buf[8].sem_op  = -1;
    mutex_buf[8].sem_flg = 0;


    mutex_buf[9].sem_num = SEM_ALIVE;
    mutex_buf[9].sem_op  = 1;
    mutex_buf[9].sem_flg = SEM_UNDO;

    semOperator (sem_common_id, mutex_buf, 10);

    fprintf (stderr, "I am in code!\n");

    //------------------COMMON-MEMORY----------------------
    int common_mem_id = shmget (key, sizeof (struct shr_buffer), IPC_CREAT | 0666);
    if (common_mem_id == -1)
    {
        perror ("Error in shmget!\n");
        exit (EXIT_FAILURE);
    }

    struct shr_buffer* common_mem = shmat (common_mem_id, NULL, 0);
    if (common_mem == NULL - 1)
    {
        perror ("Error in shmat!\n");
        exit (EXIT_FAILURE);
    }
    //------------------COMMON-MEMORY----------------------
    // sleep (10);

    int ret_read = 0;
    char not_time_to_die = 1;  
    struct sembuf buf = {};

    while (1)
    {
        buf.sem_num = SEM_EMPTY;
        buf.sem_op  = -1;
        buf.sem_flg = 0;
        semOperator (sem_common_id, &buf, 1); // empty

        if (not_time_to_die == 0 && common_mem->success_end == 1)
        {
            break;
        }

        checkAnotherProcessAlive (sem_common_id, 0, 0);            
        // Check, that another process alive

        //-------------------------------------------------
        // Load data
        ret_read = read (fd_text, common_mem->data, SIZE_DATA_PRIVATE_SHR_MEM);
        if (ret_read == -1)
        {
            fprintf (stderr, "Error in read!\n");
            exit (EXIT_FAILURE);
        }

        common_mem->byte_used = ret_read;
        common_mem->success_end = 0;
        //-------------------------------------------------

        buf.sem_num = SEM_FULL;
        buf.sem_op  = 1;
        buf.sem_flg = 0;
        semOperator (sem_common_id, &buf, 1); // full

        if (ret_read < SIZE_DATA_PRIVATE_SHR_MEM)
        {
            not_time_to_die = 0;
            // It's need to go out in next turn of the cycle
        }
    }

    return EXIT_SUCCESS;
}