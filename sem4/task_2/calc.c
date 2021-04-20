#include "calc.h"

static const char* PATH_CACHE_LINE_SIZE = \
    "/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size";

int StartCalculation (int64_t num_threads)
{
    int num_cpus = get_nprocs();
    printf ("Num cpu = %d\n", num_cpus);

    int size_block = 0;

    // I do 2 calloc, because I shouldn't align memory.
    bool* hyper_array    = calloc (num_cpus, sizeof (bool));
    int*  cpu_sort_array = calloc (num_cpus, sizeof (int));

    cpu_info_t cpus_info = 
    {
        .hyper     = hyper_array,
        .cpu_sort  = cpu_sort_array,
        .real_cpu = 0
    };

    int ret = IdentifyingProcessors (&cpus_info, num_cpus);
    if (ret == ERROR)
        return ERROR;

    void* array_threads = AllocThreadsData (num_threads, 
                                            &size_block, 
                                            sizeof (thread_data_t));

    #ifdef DEBUG
    printf ("CPU SORT ARRAY\n");
    for (int i = 0; i < num_cpus; i++)
    {
        printf ("%d ", cpu_sort_array[i]);
    }
    printf ("\nReal cpu = %d\n", cpus_info.real_cpu);
    #endif

    int num_cpus_nw = 0;
    if (cpus_info.real_cpu > num_threads)
        num_cpus_nw = cpus_info.real_cpu - num_threads;

    pthread_t* threads_array = (pthread_t*) calloc (num_threads,
                                                    sizeof (pthread_t));
    if (threads_array == NULL)
        return ERROR;


    printf ("Pthr size %ld", sizeof (bool));

    FreeAlignMem (array_threads);
    free (hyper_array);
    free (cpu_sort_array);
}

int64_t GetNumber (const char* text)
{
    int64_t value = strtoll (text, NULL, 10);
    if (errno != 0 || value <= 1)
        return ERROR;

    return value;
}

void* AllocThreadsData (int num_threads, int* size_block, int size_type)
{
    int size_line = GetSizeCacheLine();
    if (size_line == -1)
        return NULL;

    printf ("%d\n", size_line);

    int tmp = size_type / size_line;
    *size_block = (tmp > 0) ? tmp + 1 : size_line;

    return AllocAlignMem (size_line, num_threads * (*size_block));
}

int GetSizeCacheLine()
{
    errno = 0;
    FILE* cache_file = fopen (PATH_CACHE_LINE_SIZE, "r");
    if (cache_file == NULL)
        return ERROR;

    int size_cache = 0;
    int ret = fscanf (cache_file, "%d", &size_cache);
    fclose (cache_file);

    if (ret == 0)
    {
        perror ("Didn't find cache size in \"PATH_CACHE_LINE_SIZE\"");
        return ERROR;
    }

    return size_cache;
}

int IdentifyingOneProcessor (cpu_info_t* cpus_info, 
                             int i_cpu, 
                             int* last_real, 
                             int* last_hyper)
{
    const char* PATH_CORES_FORMAT = "/sys/devices/system/cpu/cpu%d/topology/core_cpus_list";
    char buffer[70];

    int ret = sprintf (buffer, PATH_CORES_FORMAT, i_cpu);
    if (ret < 0)
        return ERROR;

    FILE* file = fopen (buffer, "r");
    int main_core = 0, hype_core = 0;
    ret = fscanf (file, "%d,%d", &main_core, &hype_core);
    fclose (file);

    if (ret <= 0)
        return ERROR;

    // I don't see in file we have 1 or 2 core, because I see everyone.
    // I can speed up this function
    if (i_cpu == main_core)
    {
        (*cpus_info).hyper[i_cpu] = false;
        (*cpus_info).cpu_sort[(*last_real)++] = i_cpu;
        (*cpus_info).real_cpu++;
    }
    else
    {
        (*cpus_info).hyper[i_cpu] = true;
        (*cpus_info).cpu_sort[(*last_hyper)++] = i_cpu;
    }

    #ifdef DEBUG
    printf ("Main core = %d\nhype core = %d\n\n", main_core, hype_core);
    #endif

    return 0;
}

int IdentifyingProcessors (cpu_info_t* cpus_info, int num_cpus)
{
    int ret        = 0,
        last_real  = 0,
        last_hyper = (num_cpus - 1) / 2 + 1;
    // printf ("hyper num %d\n", last_hyper);

    for (size_t i = 0; i < num_cpus; i++)
    {
        ret = IdentifyingOneProcessor (cpus_info, i, &last_real, &last_hyper);
        if (ret == ERROR)
            return ERROR;
    }
    return 0;
}

//=========================================================
// My realisation system function

void* AllocAlignMem (size_t alignment, size_t size)
{
    void* ret = malloc (size + (sizeof (void*) / 2) + alignment);
    if (ret == NULL)
        return NULL;

    uintptr_t point = (uintptr_t) ret;
    uintptr_t tmp = point % alignment;

    point += alignment - tmp;

    *((uintptr_t*) point - 1) = (uintptr_t) ret;

    #ifdef DEBUG
    printf ("Malloc \t\t= %p\nPoint \t\t= %p\nAddmall \t= %p\n", 
            (void*) ret, (void*) point, (void*)*((uintptr_t*) point - 1));
    #endif

    return (void*) point;
}

void FreeAlignMem (void* ptr)
{
    ptr = (void*)*((uintptr_t*) ptr - 1);
    free (ptr);

    #ifdef DEBUG
    printf ("free ptr \t= %p\n", ptr);
    #endif

}
