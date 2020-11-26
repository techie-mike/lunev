const char* NAME_COMMON_FILE = "/tmp/common_shr_file"; 
enum
{
    SIZE_DATA_PRIVATE_SHR_MEM = 256
};

enum 
{
    NUM_SEMS = 6,
    SEM_CONNECT = 0,
    SEM_HAVE_WRITER,
    SEM_HAVE_READER,
    SEM_FULL,
    SEM_EMPTY,
    SEM_ALIVE,
    SEM_WRITER_INIT
};

struct shr_buffer
{
    char success_end;
    int byte_used;
    char data[SIZE_DATA_PRIVATE_SHR_MEM];
};


union semun 
{
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                        (Linux-specific) */
};

void semOperator (int semid, struct sembuf *sops, size_t nsops)
{
    if (semop (semid, sops, nsops) == -1)
    {
        perror ("Error in semop");
        exit (EXIT_FAILURE);
    }
}

void createCommonFile ()
{
    int fd_common = creat (NAME_COMMON_FILE, 0666);
    if (fd_common == -1)
    {
        fprintf (stderr, "Can't creat common file!\n");
        exit (EXIT_FAILURE);
    }
    close (fd_common);
}

void deleteResources (int semid, int shmid, const void* private_shrmem)
{
    if (shmdt (private_shrmem) == -1)
    {
        perror ("Error in \"deleteResources\" shmctl!\n");
        exit (EXIT_FAILURE);
    }

    if (shmctl (shmid, IPC_RMID, NULL) == -1)
    {
        perror ("Error in \"deleteResources\" shmctl!\n");
        exit (EXIT_FAILURE);
    }
    

    if (semctl (semid, 0, IPC_RMID) == -1)
    {
        perror ("Error in \"deleteResources\" semctl!\n");
        exit (EXIT_FAILURE);
    }
}

void checkAnotherProcessAlive (int semid, int shmid, const void* private_shrmem)
{
    if (semctl (semid, SEM_ALIVE, GETVAL) == 1)
    {
        fprintf (stderr, "\nError, another process died!\n");
        // deleteResources (semid, shmid, private_shrmem);
        exit (EXIT_FAILURE);
    }
}

