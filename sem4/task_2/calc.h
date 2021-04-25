#ifndef CALC_H
#define CALC_H

#define _GNU_SOURCE

#include <errno.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <sched.h>
#include <math.h>

#define PRINT_ERROR(str) printf ("ERROR in file \"%s\", finction \"%s\", line %d\n%s\n", __FILE__, __FUNCTION__, __LINE__, str);

typedef struct {
    double start;
    double end;
    double step;
    double result;
    int    id_cpu;
} thread_data_t;

typedef struct {
    bool* hyper;

    // If we don't have hyper cpus, all cpus in array as real.
    // If we have hyper cpus, then :
    //  [0, (num_cpus - 1) / 2] = real cpus
    //  [(num_cpus - 1) / 2 + 1, num_cpus - 1] = hyper cpus
    int* cpu_sort;
    int  real_cpu;
    int  num_cpus;
} cpu_info_t;

enum {
    ERROR = -1
};

// static const char* PATH_TO_NUM_CPU = "/sys/devices/system/cpu/online";
// static const char* PATH_TO_NUM_CPU = "online";

int64_t GetNumber (const char* text);

int StartCalculation (int64_t num_threads);

void* AllocThreadsData (int num_threads, int* size_block, int size_type);

int GetSizeCacheLine();
int IdentifyingOneProcessor (cpu_info_t* cpus_info, int i_cpu, int* last_real, int* last_hyper);
int IdentifyingProcessors (cpu_info_t* cpus_info, int num_cpus);
int PrepareThreads (void* array_thr, cpu_info_t* cpus_info, int size_block, int num_all_threads, int num_work_threads,
                    double start, double end, double step);

void* ThreadWork (void* info);

double function (double x);

//=========================================================
// My realisation system function
void* AllocAlignMem (size_t alignment, size_t size);
void  FreeAlignMem (void* ptr);


#endif