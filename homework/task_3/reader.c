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


int main ()
{
    createCommonFile();
    //-----------------------------------------------------   

    struct sembuf buf = {};

    key_t key = ftok (NAME_COMMON_FILE, 0);

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
    else
    {
        buf.sem_num = SEM_COMMON_MUTEX;
        buf.sem_op = 1;
        buf.sem_flg = 0;
        semOperator (sem_common_id, &buf, 1);

        // It doesn't matter if someone takes possession from us
    }

    //-----------------------------------------------------
    // Get COMMON_MUTEX
    buf.sem_num = SEM_COMMON_MUTEX;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;
    semOperator (sem_common_id, &buf, 1);
    fprintf (stderr, "I have доступ\n");
    //-----------------------------------------------------
    
    union semun sem_union_common = {};
    sem_union_common.val = 0;
    if (semctl (sem_common_id, SEM_CONNECT, SETVAL, sem_union_common) == -1)
    {
        perror ("Error in semctl, when init semaphores");
        exit (EXIT_FAILURE);
    }
    // Semaphore initialization by zero 

    //--------------------------------
    // First  semaphore for connect
    // Second semaphore for mutex
    // Third  semaphore for full
    // Fourth semaphore for empty
    //--------------------------------
    
    // if (sem_id != -1)
    // {
    //     union semun sem_union = {};
    //     unsigned short array_set[4] = {0, 1, 0, 1};
    //     sem_union.array = array_set;
    //     if (semctl (sem_id, 0, SETALL, sem_union) == -1)
    //     {
    //         perror ("Error in semctl, when init semaphores");
    //         exit (EXIT_FAILURE);
    //     }
    // }
    // else if (errno == EEXIST)
    // {    
    //     sem_id = semget (key, 4, 0);
    // }
    // else
    // {
    //     perror ("Error in semget!\n");
    //     exit (EXIT_FAILURE);
    // }
    //-----------------INIT-SEMAPHORES---------------------


    //---------------INIT-SHARED-MEMORY--------------------
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
    //---------------INIT-SHARED-MEMORY--------------------
    
    int my_pid = getpid();
    *common_mem = my_pid;
    // fprintf (stderr, "My pid = %d\n", my_pid);


    key_t private_key = ftok (NAME_COMMON_FILE, my_pid);

    //---------------CREAT-PRIVATE-SEMAPHORE---------------
    int sem_private_id = semget (private_key, 4, IPC_CREAT | IPC_EXCL | 0666);
    // First  semaphore for mutex
    // Second semaphore for full
    // Third  semaphore for empty
    // Fourth semaphore for state (alive / die)

    union semun sem_union = {};
    unsigned short array_set[4] = {1, 0, 1, 2};
    sem_union.array = array_set;
    if (semctl (sem_private_id, 0, SETALL, sem_union) == -1)
    {
        perror ("Error in semctl, when init semaphores");
        exit (EXIT_FAILURE);
    }
    //---------------CREAT-PRIVATE-SEMAPHORE---------------


    //---------------CREAT-PRIVATE-MEMORY------------------

    int private_mem_id = shmget (private_key, sizeof (struct shr_buffer), IPC_CREAT | 0666);
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
    // fprintf (stderr, "Address shrmem = %p\n", private_mem);

    //---------------CREAT-PRIVATE-MEMORY------------------


    //-----------------------------------------------------
    //     Сustomization of behavior in case of death
    //-----------------------------------------------------

    buf.sem_num = SEM_FULL;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;
    semOperator (sem_private_id, &buf, 1);

    buf.sem_num = SEM_FULL;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semOperator (sem_private_id, &buf, 1);
    //-----------------------------------------------------
    buf.sem_num = SEM_EMPTY;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;
    semOperator (sem_private_id, &buf, 1);

    buf.sem_num = SEM_EMPTY;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    semOperator (sem_private_id, &buf, 1);

    //-----------------------------------------------------
    //     Сustomization of behavior in case of death
    //-----------------------------------------------------

    
    buf.sem_num = SEM_ALIVE;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;
    semOperator (sem_private_id, &buf, 1);
    // To check, that process alive

    buf.sem_num = SEM_CONNECT;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    semOperator (sem_common_id, &buf, 1);
    // State, that data in common shrmem to be
    // Здесь идёт передача противоположному процессу


    // Если здесь умрёт читатель - ничего страшного, так как пднимется
    // ALIVE, означающий, что второй процесс умер и первый сам выйдёт с ошибкой

    //-----------------------------------------------------
    // Ждём когда кто-нибудь придёт (заберёт данные).
    // Этот семафор в штатном режиме имеет 0 и 1, 
    // но если читатель c владением падает - он увеличивается,
    // что не страшно, так как 
    // buf.sem_num = SEM_CONNECT;
    // buf.sem_op = 1;
    // buf.sem_flg = 0;
    // semOperator (sem_common_id, &buf, 1);
    //-----------------------------------------------------
    
    buf.sem_num = SEM_CONNECT;
    buf.sem_op = 0;
    buf.sem_flg = 0;
    semOperator (sem_common_id, &buf, 1);
    // There is main wait

    buf.sem_num = SEM_ALIVE;
    buf.sem_op = 0;
    buf.sem_flg = 0;

    struct timespec wait_time = {};
    wait_time.tv_sec = 2;
    if (semtimedop (sem_private_id, &buf, 1, &wait_time) == -1)
    {
        if (errno == EAGAIN)
        {
            fprintf (stderr, "Another process die!\n");
            exit (EXIT_FAILURE);
        } 
    }

    semOperator (sem_private_id, &buf, 1);
    fprintf (stderr, "Have pair!\n");
    // Now wait, when something try read from memory

    //-----------------------------------------------------
    // Return COMMON_MUTEX

    buf.sem_num = SEM_COMMON_MUTEX;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;
    semOperator (sem_common_id, &buf, 1);
    //-----------------------------------------------------

    int last_num_bytes = 0;
    while (1)
    {
        //-------------------------------------------------
        // Check, that another process alive
        int val_alive = semctl (sem_private_id, SEM_ALIVE, GETVAL);
        // fprintf (stderr, "Val = %d\n", val_alive);
        if (val_alive != 0)
        {
            fprintf (stderr, "Error, another process died!\n");
            exit (EXIT_FAILURE);
        }
        //-------------------------------------------------

        buf.sem_num = SEM_FULL;
        buf.sem_op  = -1;
        buf.sem_flg = 0;
        semOperator (sem_private_id, &buf, 1); // full

        buf.sem_num = SEM_MUTEX;
        buf.sem_op  = -1;
        buf.sem_flg = SEM_UNDO;
        semOperator (sem_private_id, &buf, 1); // mutex

        //-------------------------------------------------
        // Upload data
        write (STDOUT_FILENO, private_mem->data, private_mem->byte_used);
        last_num_bytes = private_mem->byte_used;
        // fprintf (stderr, "data %d\n", *(int*)private_mem); 


        //-------------------------------------------------

        buf.sem_num = SEM_MUTEX;
        buf.sem_op  = 1;
        buf.sem_flg = SEM_UNDO;
        semOperator (sem_private_id, &buf, 1); // mutex



        buf.sem_num = SEM_EMPTY;
        buf.sem_op  = 1;
        buf.sem_flg = 0;
        semOperator (sem_private_id, &buf, 1); // empty

        if (last_num_bytes < SIZE_DATA_PRIVATE_SHR_MEM)
        {
            break;
        }
    }
    return 0;
}
