#ifndef CALC_H
#define CALC_H

#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <errno.h>

typedef struct {
    double start;
    double end;
    double step;
    double result;
} thread_data_t;

typedef struct {
    bool* hyper;

    // If we don't have hyper cpus, all cpus in array as real.
    // If we have hyper cpus, then :
    //  [0, (num_cpus - 1) / 2] = real cpus
    //  [(num_cpus - 1) / 2 + 1, num_cpus - 1] = hyper cpus
    int* cpu_sort;
    int  real_cpu;
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
int PrepareThreads (void* array_thr, int size_block, int num_thread, 
                    double start, double end, double);

//=========================================================
// My realisation system function
void* AllocAlignMem (size_t alignment, size_t size);
void  FreeAlignMem (void* ptr);


#endif