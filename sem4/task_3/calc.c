#include "calc.h"

// static const double c_start = 1;
// static const double c_step  = 0.0000001;
// static const double c_end   = 200;

static const char* PATH_CACHE_LINE_SIZE = \
    "/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size";

int StartCalculation (int64_t num_threads, double c_start, double c_end, double c_step, double* c_result)
{
    int num_cpus = get_nprocs();
    #ifdef DEBUG
    printf ("Num cpu = %d\n", num_cpus);
    #endif

    int size_block = 0;

    // I do 2 calloc, because I shouldn't align memory.
    bool* hyper_array    = calloc (num_cpus, sizeof (bool));
    int*  cpu_sort_array = calloc (num_cpus, sizeof (int));

    cpu_info_t cpus_info = 
    {
        .hyper     = hyper_array,
        .cpu_sort  = cpu_sort_array,
        .real_cpu  = 0,
        .num_cpus  = num_cpus
    };

    int ret = IdentifyingProcessors (&cpus_info, num_cpus);
    if (ret == ERROR)
        return ERROR;

    int num_all_threads = num_threads > num_cpus ? num_threads : num_cpus;

    void* array_threads = AllocThreadsData (num_all_threads, 
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

    pthread_t* pthr_array = (pthread_t*) calloc (num_all_threads,
                                                    sizeof (pthread_t));
    if (pthr_array == NULL)
        return ERROR;


    ret = PrepareThreads (array_threads, &cpus_info, size_block, num_all_threads, num_threads, c_start, c_end, c_step);
    if (ret == ERROR)
        return ERROR;

    for (size_t i = 0; i < num_all_threads; i++)
    {
        int ret_pthreat = pthread_create (&pthr_array[i], NULL, ThreadWork, (array_threads + i * size_block));
        if (ret_pthreat != 0)
        {
            printf ("Can't create thread!\n");
            return ERROR;
        }
    }
    double result = 0.0;

    for (int i = 0; i < num_threads; i++)
    {
        int ret_join = pthread_join (pthr_array[i], NULL);
        if (ret_join != 0)
            return ERROR;

        result += ((thread_data_t*)(array_threads + i * size_block))->result;
    }
    printf ("Integral result %lg\n", result);

    FreeAlignMem (array_threads);
    free (hyper_array);
    free (cpu_sort_array);
    *c_result = result;
    return 0;
}

int64_t GetNumber (const char* text)
{
    int64_t value = strtoll (text, NULL, 10);
    if (errno != 0 || value < 1)
        return ERROR;

    return value;
}

void* AllocThreadsData (int num_threads, int* size_block, int size_type)
{
    int size_line = GetSizeCacheLine();
    if (size_line == -1)
        return NULL;

    #ifdef DEBUG
    printf ("Cache line = %d\n", size_line);
    #endif

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

    #ifdef DEBUG
    printf ("-----Topology cpu-----\n");
    #endif

    for (size_t i = 0; i < num_cpus; i++)
    {
        ret = IdentifyingOneProcessor (cpus_info, i, &last_real, &last_hyper);
        if (ret == ERROR)
            return ERROR;
    }
    #ifdef DEBUG
    printf ("-----Topology cpu-----\n");
    #endif

    return 0;
}

int PrepareThreads (void* array_thr, cpu_info_t* cpus_info, int size_block, int num_all_threads, int num_work_threads,
                    double start, double end, double step)
{
    if (array_thr == NULL || size_block <= 0 || num_all_threads <= 0 || num_work_threads <= 0
        || isnan (start) || isnan (end) || isnan (step))
    {
        PRINT_ERROR("not valid arguments!");
        return ERROR;
    }

    double portion = (end - start) / num_work_threads;
    int num_cpus = cpus_info->num_cpus;
    for (int i = 0; i < num_work_threads; i++)
    {
        ((thread_data_t*)(array_thr + i * size_block))->start  = start + portion * i;
        ((thread_data_t*)(array_thr + i * size_block))->end    = start + portion * (i + 1);
        ((thread_data_t*)(array_thr + i * size_block))->step   = step;
        ((thread_data_t*)(array_thr + i * size_block))->id_cpu = cpus_info->cpu_sort[i % num_cpus];
    }

    for (int i = num_work_threads; i < num_all_threads; i++)
    {
        ((thread_data_t*)(array_thr + i * size_block))->start  = start + portion * i;
        ((thread_data_t*)(array_thr + i * size_block))->end    = start + portion * (i + 1);
        ((thread_data_t*)(array_thr + i * size_block))->step   = step;
        ((thread_data_t*)(array_thr + i * size_block))->id_cpu = cpus_info->cpu_sort[i % num_cpus];
    }
    return 0;
}

void* ThreadWork (void* info)
{
    if (info == NULL)
    {
        PRINT_ERROR("Not valid argument!");
        exit (EXIT_FAILURE);
    }

    cpu_set_t cpu;
    pthread_t thread = pthread_self();
    int my_cpu = ((thread_data_t*)info)->id_cpu;

    CPU_ZERO (&cpu);
    CPU_SET (my_cpu, &cpu);

    int ret_set = pthread_setaffinity_np (thread, sizeof (cpu_set_t), &cpu);
    if (ret_set != 0)
    {
        PRINT_ERROR("pthread_setaffinity_np!");
        exit (EXIT_FAILURE);
    }

    double step = ((thread_data_t*)info)->step;
    double x    = ((thread_data_t*)info)->start;
    double end  = ((thread_data_t*)info)->end - step;

    for (; x < end; x += step)
        ((thread_data_t*)info)->result += function (x) * step;

    ((thread_data_t*)info)->result += function (x) * (((thread_data_t*)info)->end - x);
    return NULL;
}


double function (double x)
{
    return x * x;
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
    printf ("\n------AllocAlignMem------\nMalloc \t\t= %p\nPoint \t\t= %p\nAddmall \t= %p\n------AllocAlignMem------\n\n", 
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
