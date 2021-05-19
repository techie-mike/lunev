#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

// #define DEBUG

static const uint32_t UDP_PORT = 9030;

static const uint32_t TCP_PORT = 9070;

typedef struct {
    double start;
    double end;
    double step;
} task_data_t;

#endif //COMMON_H