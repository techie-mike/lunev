const char* NAME_COMMON_FILE = "/tmp/common_shr_file"; 
enum
{
    SIZE_DATA_PRIVATE_SHR_MEM = 20
};

enum 
{
    SEM_CONNECT = 0,
    SEM_COMMON_MUTEX,
    SEM_MUTEX   = 0,
    SEM_FULL,
    SEM_EMPTY,
    SEM_ALIVE
};

struct shr_buffer
{
    char last_pack;
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