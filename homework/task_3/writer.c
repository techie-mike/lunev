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

    struct sembuf buf = {};

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


    //------------------COMMON-MEMORY----------------------
    int common_mem_id = shmget (key, sizeof (int), IPC_CREAT | 0666);
    if (common_mem_id == -1)
    {
        perror ("Error in shmget!\n");
        exit (EXIT_FAILURE);
    }

    int* common_mem = shmat (common_mem_id, NULL, 0);
    if (common_mem == NULL - 1)
    {
        perror ("Error in shmat!\n");
        exit (EXIT_FAILURE);
    }
    //------------------COMMON-MEMORY----------------------
    
    
    int pid_reader = 0;
    key_t private_key = 0;
    int sem_private_id = 0;

    while (1)
    {
        buf.sem_num = SEM_CONNECT;
        buf.sem_op  = -1;
        buf.sem_flg = SEM_UNDO;
        semOperator (sem_common_id, &buf, 1);

        pid_reader = *common_mem;

        //-----------------PRIVATE-----------------------------
        private_key = ftok (NAME_COMMON_FILE, pid_reader);

        sem_private_id = semget (private_key, 4, 0);
        if (sem_private_id == -1)
        {
            perror ("Error i semget!\n");
            exit (EXIT_FAILURE);
        }

        int val_alive = semctl (sem_private_id, SEM_WRITER_INIT, GETVAL);
        if (val_alive == 0)
        {
            break;
        }
    }
    
    int private_mem_id = shmget (private_key, sizeof (struct shr_buffer), 0);
    if (private_mem_id == -1)
    {
        perror ("Error in shmget");
        exit (EXIT_FAILURE);
    }
    
    struct shr_buffer* private_mem = shmat (private_mem_id, NULL, 0);
    if (private_mem == NULL - 1)
    {
        perror ("Error in shmat");
        exit (EXIT_FAILURE);
    }


    //-----------------PRIVATE-----------------------------

    //-----------------------------------------------------
    //     Сustomization of behavior in case of death
    //-----------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    struct sembuf array_sem[6] = {};

    array_sem[0].sem_num = SEM_FULL;
    array_sem[0].sem_op = 1;
    array_sem[0].sem_flg = 0;
    // semOperator (sem_private_id, &buf, 1);

    array_sem[1].sem_num = SEM_FULL;
    array_sem[1].sem_op = -1;
    array_sem[1].sem_flg = SEM_UNDO;
    // semOperator (sem_private_id, &buf, 1);

    //-----------------------------------------------------
    array_sem[2].sem_num = SEM_EMPTY;
    array_sem[2].sem_op = 1;
    array_sem[2].sem_flg = SEM_UNDO;
    // semOperator (sem_private_id, &buf, 1);

    array_sem[3].sem_num = SEM_EMPTY;
    array_sem[3].sem_op = -1;
    array_sem[3].sem_flg = 0;
    // semOperator (sem_private_id, &buf, 1);

    //-----------------------------------------------------
    //     Сustomization of behavior in case of death
    //-----------------------------------------------------

    

    array_sem[4].sem_num = SEM_WRITER_INIT;
    array_sem[4].sem_op  = 1;
    array_sem[4].sem_flg = 0;
    // semOperator (sem_private_id, &buf, 1);

    array_sem[5].sem_num = SEM_ALIVE;
    array_sem[5].sem_op = -1;
    array_sem[5].sem_flg = SEM_UNDO;
    semOperator (sem_private_id, array_sem, 6);


    buf.sem_num = SEM_CONNECT;
    buf.sem_op  = 1;
    buf.sem_flg = SEM_UNDO;
    semOperator (sem_common_id, &buf, 1);

    buf.sem_num = SEM_CONNECT;
    buf.sem_op  = -1;
    buf.sem_flg = 0;
    semOperator (sem_common_id, &buf, 1);
// Плюсы надо запихать в один массив, чтобы было атомарное выполнение
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //=====================================================
    //             The end of ALL initialisation
    //=====================================================

    
    int ret_read = 0;
    char not_time_to_die = 1;  

    checkAnotherProcessAlive (sem_private_id, private_mem_id, private_mem);
    
    while (1)
    {
        buf.sem_num = SEM_EMPTY;
        buf.sem_op  = -1;
        buf.sem_flg = 0;
        semOperator (sem_private_id, &buf, 1); // empty

        checkAnotherProcessAlive (sem_private_id, private_mem_id, private_mem);            

        if (not_time_to_die == 0)
        {
            break;
        }


        // Check, that another process alive

        //-------------------------------------------------
        // Load data
        ret_read = read (fd_text, private_mem->data, SIZE_DATA_PRIVATE_SHR_MEM);
        if (ret_read == -1)
        {
            fprintf (stderr, "Error in read!\n");
            exit (EXIT_FAILURE);
        }

        private_mem->byte_used = ret_read;
        private_mem->last_pack = 0;
        //-------------------------------------------------

        buf.sem_num = SEM_FULL;
        buf.sem_op  = 1;
        buf.sem_flg = 0;
        semOperator (sem_private_id, &buf, 1); // full

        if (ret_read < SIZE_DATA_PRIVATE_SHR_MEM)
        {
            not_time_to_die = 0;
            // It's need to go out in next turn of the cycle
        }
    }

    deleteResources (sem_private_id, private_mem_id, private_mem);
    // Delete private SHMMEM and SEMAPHORE

    return EXIT_SUCCESS;
}
