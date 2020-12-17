#pragma once

#define _GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#define CHECK(code)             \
    do {                        \
        if (code == -1) {       \
            perror (#code);     \
            exit (EXIT_FAILURE);\
        }                       \
    } while (0)

enum 
{
    RD_END = 0,
    WR_END,
    SIZE_DATA_BLOCK = 4096,
    INCORRECT_FD = -1
};

struct ChildInfo
{
    int fd_in;
    int fd_out;
};

struct Connection
{
    int fd_in;
    int fd_out;

    char* buffer;
    ssize_t size_buffer;

    size_t tail;
    size_t head;
    size_t count;
};

void createConnections (struct Connection *connectors, struct ChildInfo *children, size_t num_children);

size_t getSizeBuffer (size_t index, size_t num_children);

void mainJobAsServer (struct Connection *connectors, size_t num_children);

void readToBuffer (struct Connection* connectors);

void writeFromBuffer (struct Connection* connectors);