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

int main()
{
    createCommonFile();

    key_t key = ftok (NAME_COMMON_FILE, 0);
    
    //-----------------CREATE-SEMAPHORES-------------------
    int sem_common_id = semget (key, NUM_SEMS, IPC_CREAT | IPC_EXCL | 0666);

    if (sem_common_id == -1)
    {
        if (errno == EEXIST)
        {    
            sem_common_id = semget (key, NUM_SEMS, 0);
        }
        else    
        {
            perror ("Error in semget!\n");
            exit (EXIT_FAILURE);
        }
    }
    //-----------------CREATE-SEMAPHORES-------------------

    struct sembuf mutex_buf[4] = {};


    //--------------------GET-MUTEX-----------------------
    mutex_buf[0].sem_num = SEM_HAVE_READER;
    mutex_buf[0].sem_op  = 0;
    mutex_buf[0].sem_flg = 0;

    mutex_buf[1].sem_num = SEM_HAVE_READER;
    mutex_buf[1].sem_op  = 1;
    mutex_buf[1].sem_flg = SEM_UNDO;

    mutex_buf[2].sem_num = SEM_ALIVE;
    mutex_buf[2].sem_op  = 0;
    mutex_buf[2].sem_flg = 0;

    mutex_buf[3].sem_num = SEM_ALIVE;
    mutex_buf[3].sem_op  = 1;
    mutex_buf[3].sem_flg = SEM_UNDO;

    semOperator (sem_common_id, mutex_buf, 4);
    //--------------------GET-MUTEX-----------------------
    
    //-----------------INIT-SEMAPHORES---------------------
    union semun sem_union_common = {};

    sem_union_common.val = 0;
    if (semctl (sem_common_id, SEM_FULL, SETVAL, sem_union_common) == -1)
    {
        perror ("Error in semctl, when init semaphores");
        exit (EXIT_FAILURE);
    }

    sem_union_common.val = 1;
    if (semctl (sem_common_id, SEM_EMPTY, SETVAL, sem_union_common) == -1)
    {
        perror ("Error in semctl, when init semaphores");
        exit (EXIT_FAILURE);
    }
    //-----------------INIT-SEMAPHORES---------------------


    //---------------INIT-SHARED-MEMORY--------------------
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
    //---------------INIT-SHARED-MEMORY--------------------

    struct sembuf array_buf[6] = {};
    
    array_buf[0].sem_num = SEM_EMPTY;
    array_buf[0].sem_op  = -1;
    array_buf[0].sem_flg = SEM_UNDO;

    array_buf[1].sem_num = SEM_EMPTY;
    array_buf[1].sem_op  = 1;
    array_buf[1].sem_flg = 0;
    
    array_buf[2].sem_num = SEM_CONNECT;
    array_buf[2].sem_op  = 1;
    array_buf[2].sem_flg = SEM_UNDO;
    semOperator (sem_common_id, array_buf, 3);

    //-----------------------------------------------------

    struct sembuf buf = {};
    int last_num_bytes = 0;

    while (1)
    {
        buf.sem_num = SEM_FULL;
        buf.sem_op  = -1;
        buf.sem_flg = 0;
        semOperator (sem_common_id, &buf, 1); // full

        // Check, that another process alive
        checkAnotherProcessAlive (sem_common_id, 0, 0);

        // Upload data
        write (STDOUT_FILENO, common_mem->data, common_mem->byte_used);
        last_num_bytes = common_mem->byte_used;

        if (last_num_bytes < SIZE_DATA_PRIVATE_SHR_MEM)
        {
            common_mem->success_end = 1;
            // We said, that transfer was successful
        }

        buf.sem_num = SEM_EMPTY;
        buf.sem_op  = 1;
        buf.sem_flg = 0;
        semOperator (sem_common_id, &buf, 1); // empty

        if (last_num_bytes < SIZE_DATA_PRIVATE_SHR_MEM)
        {
            break;
        }
    }

    mutex_buf[0].sem_num = SEM_CONNECT;
    mutex_buf[0].sem_op  = 1;
    mutex_buf[0].sem_flg = 0;

    mutex_buf[1].sem_num = SEM_CONNECT;
    mutex_buf[1].sem_op  = -1;
    mutex_buf[1].sem_flg = SEM_UNDO;
    
    mutex_buf[2].sem_num = SEM_ALIVE;
    mutex_buf[2].sem_op  = -1;
    mutex_buf[2].sem_flg = SEM_UNDO;
    
    mutex_buf[3].sem_num = SEM_HAVE_READER;
    mutex_buf[3].sem_op  = -1;
    mutex_buf[3].sem_flg = SEM_UNDO;    

    semOperator (sem_common_id, mutex_buf, 4);
    
    return EXIT_SUCCESS;
}