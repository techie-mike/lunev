#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include <unistd.h>
#include <stdlib.h>

#include "common.h"


#define PRINT_ERROR(str) printf ("ERROR in file \"%s\", finction \"%s\", line %d\n%s\n", __FILE__, __FUNCTION__, __LINE__, str);


typedef struct {
    int fd;
    unsigned int num_thread;
} tcp_connection;



static int SendMessForTcpConnect();
static int StartTcpSocket();

static int ListenTcpSocket (int server_socket, unsigned int num_computers, tcp_connection* connections);
static int SetKeepalive (int socket, int keepcnt, int keepidle, int keepintvl);
static int SetServerSocketOptions (int socket_server);

static int SplitSendWork (int socket, unsigned int num_computers, tcp_connection* connections, double start, double end, double step);

static int ReceiveResults (int socket, unsigned int num_computers, tcp_connection* connections, double* results);


int main (int argc, const char* argv[])
{
    if (argc < 2)
    {
        fprintf (stderr, "Not enough arguments!\n");
        return -1;
    }

    unsigned int num_pc = strtoul (argv[1], NULL, 10);
    if (num_pc == 0 || ((num_pc == LONG_MAX || num_pc == LONG_MIN) && errno == EAGAIN))
    {
        PRINT_ERROR ("strtoul");
        return -1;
    }

    tcp_connection* connections = calloc (num_pc, sizeof (tcp_connection));
    if (connections == NULL)
    {
        PRINT_ERROR ("calloc");
        return -1;
    }


    int tcp_socket = StartTcpSocket();
    SendMessForTcpConnect();
    ListenTcpSocket (tcp_socket, num_pc, connections);
    //-------------WORK---------------
    if (SplitSendWork (tcp_socket, num_pc, connections, 1, 200, 0.0000001))
    {
        fprintf (stderr, "Broke connect with worker!\n");
        return -1;
    }

    double result = 0;
    if (ReceiveResults (tcp_socket, num_pc, connections, &result))
    {
        fprintf (stderr, "Broke connect with worker! When receive result.\n");
        return -1;
    }
    printf ("Result %lg\n", result);
    return 0;
}

static int SendMessForTcpConnect()
{
    int sk = socket (AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = 
    {
        .sin_family = AF_INET,
        .sin_port = htons (UDP_PORT),
        .sin_addr.s_addr = htonl (INADDR_BROADCAST)
    };

    int num = 1;
    int ret_set = setsockopt (sk, SOL_SOCKET, SO_BROADCAST, &num, sizeof num);

    #ifdef DEBUG
    printf ("Ret_set = %d\n", ret_set);
    #endif

    char buf[6] = "Hello";
    int ret_send = sendto (sk, &buf, 5, 0, (struct sockaddr *) &addr, sizeof (addr));
    printf ("Ret send = %d\n", ret_send);
}

static int StartTcpSocket()
{
    int sk = socket(AF_INET, SOCK_STREAM, 0);

    int ret = SetServerSocketOptions (sk);
    printf ("ret = %d\n", ret);
    if (ret != 0)
    {
        PRINT_ERROR ("set server socket options");
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons (TCP_PORT),
        .sin_addr.s_addr = htonl (INADDR_ANY)
    };

    int ret_bind = bind (sk, (struct sockaddr *) &addr, sizeof(addr));
    if (ret_bind != 0)
    {
        PRINT_ERROR("bind returned error");
        perror("");
        return -1;
    }

    #ifdef DEBUG
    printf ("Ret bind = %d\n errno = %d\n", ret_bind, errno);
    #endif

    listen (sk, 256);
    return sk;
}

static int ListenTcpSocket (int server_socket, 
                            unsigned int num_computers,
                            tcp_connection* connections)
{
    for (size_t i = 0; i < num_computers; i++)
    {
        int sk = accept (server_socket, (struct sockaddr *) NULL, NULL);
        printf ("accept %d\n", sk);
        if (sk == -1)
        {
            fprintf (stderr, "Can't accept all computers!\n");
            return -1;
        }
        connections[i].fd = sk;
    }

    for (size_t i = 0; i < num_computers; i++)
    {
        uint32_t buf = 0;
        int ret = recv (connections[i].fd, &buf, sizeof (uint32_t), 0);
        if (ret == 0)
        {
            fprintf (stderr, "Another side close!\n");
            return -1;
        }
        connections[i].num_thread = buf;
    }
    return 0;
}

static int SetKeepalive (int socket, int keepcnt, int keepidle, int keepintvl) {
    int ret;
    int optTrue = 1;

    ret = setsockopt (socket, SOL_SOCKET, SO_KEEPALIVE, &optTrue, sizeof(int));
    if (ret != 0) {
        perror ("setsockopt");
        return -1;
    }

    ret = setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int));
    if (ret != 0) {
        perror ("tcp_keepcnt");
        return -1;
    }

    ret = setsockopt (socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int));
    if (ret != 0) {
        perror ("tcp_keepcnt");
        return -1;
    }

    ret = setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int));
    if (ret != 0) {
        perror ("tcp_keepcnt");
        return -1;
    }

    return 0;
}

static int SetServerSocketOptions (int socket_server) {
    assert (socket_server > 0);
    int optTrue = 1;

    if (setsockopt (socket_server, SOL_SOCKET, SO_REUSEADDR, &optTrue, sizeof(int)) != 0) {
        perror ("setsockopt for server socket:");
        return -1;
    }

    struct timeval timeout = {
            .tv_sec = 20,
            .tv_usec = 0
    };

    if (setsockopt (socket_server, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) != 0) {
        perror ("setsockopt (timeout) for server socket:");
        return -1;
    }

    if (SetKeepalive (socket_server, 4, 3, 1)) {
        return -1;
    }

    return 0;
}

static int SplitSendWork (int socket, unsigned int num_computers, tcp_connection* connections, double start, double end, double step)
{
    unsigned int all_threads = 0;
    for (size_t i = 0; i < num_computers; i++)
    {
        all_threads += connections[i].num_thread;
    }

    double work_per_thread = (end - start) / all_threads;

    unsigned int thread = 0;

    for (size_t i = 0; i < num_computers; i++)
    {
        printf ("thread %u\n", thread);
        task_data_t task = {
            .start = start + thread * work_per_thread,
            .end = start + (thread + connections[i].num_thread) * work_per_thread,
            .step = step
        };
        printf ("Task start %lg\nend %lg\n", task.start, task.end);


        int ret = send (connections[i].fd, &task, sizeof (task), 0);
        if (ret == 1)
        {
            fprintf (stderr, "Can't send!\n");
            return -1;
        }

        thread += connections[i].num_thread;
    }
    return 0;
}

static int ReceiveResults (int socket, unsigned int num_computers, tcp_connection* connections, double* results)
{
    for (size_t i = 0; i < num_computers; i++)
    {
        double result_worker = 0;
        int ret = recv (connections[i].fd, &result_worker, sizeof (double), 0);
        if (ret == 0)
        {
            fprintf (stderr, "Another side close!\n");
            return -1;
        }
        else if (ret == -1)
        {
            fprintf (stderr, "Broke connect with worker!\n");
        }
        
        *results += result_worker;
    }
    return 0;
}