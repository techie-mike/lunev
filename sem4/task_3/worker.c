#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#include <netinet/tcp.h>
#include <stdio.h>

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>

#include "common.h"
#include "calc.h"

// #include "common_private.h"


#define PRINT_ERROR(str) printf ("ERROR in file \"%s\", finction \"%s\", line %d\n%s\n", __FILE__, __FUNCTION__, __LINE__, str);

in_addr_t ReceiveMessForTcpConnect();

int SendTcpConnect (in_addr_t address);
static int SendWorkThreads (int sk, uint32_t num_thread); 

static int SetKeepalive (int socket, int keepcnt, int keepidle, int keepintvl);

static int WaitWork (int socket, task_data_t* task_data);

static int SendResult (int socket, double* result);

int main (int argc, const char* argv[])
{
    if (argc < 2)
    {
        fprintf (stderr, "Not enough arguments!\n");
        return -1;
    }

    unsigned int num_thread = strtoul (argv[1], NULL, 10);
    if (num_thread == 0 || ((num_thread == LONG_MAX || num_thread == LONG_MIN) && errno == EAGAIN))
    {
        PRINT_ERROR ("strtoul");
        return -1;
    }

    in_addr_t address_server = ReceiveMessForTcpConnect();
    int sk = SendTcpConnect (address_server);

    int ret = SendWorkThreads (sk, num_thread);
    if (ret == -1)
    {
        PRINT_ERROR ("send work threads");
        return -1;
    }
    //----------------WORK------------------

    task_data_t task = {};
    double result = 0;
    if (WaitWork (sk, &task))
    {
        fprintf (stderr, "Broke connect with manager!\n");
        return -1;
    }

    if (StartCalculation (num_thread, task.start, task.end, task.step, &result))
    {
        fprintf (stderr, "Broke connect with manager!\n");
        return -1;
    }
    

    if (SendResult (sk, &result))
    {
        fprintf (stderr, "Broke connect with manager!\n");
        return -1;
    }
}

in_addr_t ReceiveMessForTcpConnect()
{
    int sk = socket (PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr =
    {
        .sin_family = AF_INET,
        .sin_port = htons (UDP_PORT),
        .sin_addr.s_addr = htonl (INADDR_ANY)
    };

    int num = 1;
    int ret_set = setsockopt (sk, SOL_SOCKET, SO_REUSEADDR, &num, sizeof num);

    // The worker waits indefinitely until the manager arrives!!!

    int ret_bind = bind (sk, (struct sockaddr*) &addr, sizeof (addr));
    if (ret_bind != 0)
    {
        PRINT_ERROR ("when try to bind");
        perror("");
        return -1;
    }


    struct sockaddr_in addr_source = {};
    socklen_t size_source = sizeof (addr_source);

    char buf[6] = {};
    int ret_rec = recvfrom (sk, buf, 5, 0, (struct sockaddr *) &addr_source, &size_source);
    if (ret_rec != 5 || strcmp (buf, "Hello"))
    {
        PRINT_ERROR ("wrong return recvfrom");
        return -1;
    }
    printf ("Ret rec = %d\nData = %s\n", ret_rec, buf);
    printf ("Address source %x\n", addr_source.sin_addr.s_addr);

    return addr_source.sin_addr.s_addr;

    // addr_source.sin_family = AF_INET;
    // addr_source.sin_port   = htons (UDP_PORT);
    // //------------------------------------------------------
    // int ret = connect (sk, (struct sockaddr *) &addr, sizeof (addr));
    // perror("");
    // printf ("Connect ret = %d\n", ret);
}

int SendTcpConnect (in_addr_t address)
{
    int sk = socket (AF_INET, SOCK_STREAM, 0);

    if (SetKeepalive (sk, 2, 1, 1)) {
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons (TCP_PORT),
        .sin_addr.s_addr = address
    };

    int ret = connect (sk, (struct sockaddr *) &addr, sizeof (addr));
    perror("");
    printf ("Connect ret = %d\n", ret);
    return sk;
}

static int SendWorkThreads (int sk, uint32_t num_thread)
{
    int ret = send (sk, &num_thread, sizeof (num_thread), 0);
    if (ret != sizeof (num_thread))
        return -1;
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

static int WaitWork (int socket, task_data_t* task_data)
{
    int ret = recv (socket, task_data, sizeof (task_data_t), 0);
    if (ret == -1)
    {
        PRINT_ERROR("recv");
    }
    else if (ret == 0)
    {
        fprintf (stderr, "Server shutdown!\n");
        return -1;
    }
    printf ("Task start %lg\nend %lg\n", task_data->start, task_data->end);
    return 0;
}

static int SendResult (int socket, double* result)
{
    int ret = send (socket, result, sizeof (double), 0);
    if (ret != sizeof (double))
        return -1;
    return 0;
}