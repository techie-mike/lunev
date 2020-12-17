#include "src.h"

size_t getSizeBuffer (size_t index, size_t num_children)
{
    return (size_t) pow (3, num_children - index - 1) * 1024;
}

void createConnections (struct Connection *connectors, struct ChildInfo *children, size_t num_children)
{
    for (size_t i = 0; i < num_children; i++)
    {
        connectors[i].size_buffer = getSizeBuffer (i, num_children);
        connectors[i].count = 0;
        connectors[i].head  = 0;
        connectors[i].tail  = 0;

        connectors[i].buffer = (char*) calloc (connectors[i].size_buffer,
                                               sizeof (char));

        if (connectors[i].buffer == NULL)
        {
            fprintf (stderr, "Error in calloc in createConnections!\n");
            fprintf (stderr, "size_buffer should be = %ld", connectors[i].size_buffer);
            free (children);
            children = NULL;
            exit (EXIT_FAILURE);
        }

        connectors[i].fd_in = children[i].fd_out;
        if (i < num_children - 1)
        {
            connectors[i].fd_out = children[i + 1].fd_in;
        }
        else
        {
            connectors[num_children - 1].fd_out = STDOUT_FILENO;
        }
    }
}

void mainJobAsServer (struct Connection *connectors, size_t num_children)
{
    int max_fd = -1;
    size_t index_of_finished = 0;
    fd_set readfds;
    fd_set writefds;

    while (index_of_finished < num_children)
    {
        FD_ZERO (&readfds);
        FD_ZERO (&writefds);

        for (size_t i = index_of_finished; i < num_children; i++)
        {
            // Find ready to write
            if (connectors[i].count < connectors[i].size_buffer)
            {
                FD_SET (connectors[i].fd_in, &readfds);
                if (max_fd < connectors[i].fd_in)
                    max_fd = connectors[i].fd_in;
            }

            // Find ready to read
            if (connectors[i].count != 0)
            {
                FD_SET (connectors[i].fd_out, &writefds);
                if (max_fd < connectors[i].fd_out)
                    max_fd = connectors[i].fd_out;
            }
        }

        // Wait some pipes ready to read or write
        CHECK (select (max_fd + 1, &readfds, &writefds, NULL, NULL));

        // Find pipes that ready
        for (size_t i = index_of_finished; i < num_children; i++)
        {
            if (FD_ISSET (connectors[i].fd_in, &readfds)
                && (connectors[i].count < connectors[i].size_buffer))
            {
                readToBuffer (&connectors[i]);
            }

            if (FD_ISSET (connectors[i].fd_out, &writefds)
                && (connectors[i].count != 0))
            {
                writeFromBuffer (&connectors[i]);
            }

            if (connectors[i].fd_in == INCORRECT_FD && connectors[i].count == 0)
            {
                if (i != index_of_finished)
                {
                    fprintf (stderr, "Child died while work!\n");
                    free (connectors);
                    exit (EXIT_FAILURE);
                }

                CHECK (close (connectors[i].fd_out));
                connectors[i].fd_out = INCORRECT_FD;

                index_of_finished++;
            }
        }
    }
}

void readToBuffer (struct Connection* conn)
{
    ssize_t read_ret = 0;
    if (conn->head > conn->tail)
    {
        read_ret = read (conn->fd_in,
                         conn->buffer + conn->tail,
                         conn->head - conn->tail);

    }
    else
    {
        read_ret = read (conn->fd_in,
                         conn->buffer + conn->tail,
                         conn->size_buffer - conn->tail);
    }

    CHECK (read_ret);

    if (read_ret == 0)
    {
        CHECK (close (conn->fd_in));
        conn->fd_in = INCORRECT_FD;
        return;
    }

    conn->count += read_ret;
    conn->tail  += read_ret;

    if (conn->tail == conn->size_buffer)
    {
        conn->tail = 0;
    }

}

void writeFromBuffer (struct Connection* conn)
{
    ssize_t write_ret = 0;

    if (conn->head >= conn->tail)
    {
        write_ret = write (conn->fd_out,
                           conn->buffer + conn->head,
                           conn->size_buffer - conn->head);
    }
    else
    {
        write_ret = write (conn->fd_out,
                           conn->buffer + conn->head,
                           conn->tail - conn->head);
    }

    if (write_ret == -1)
    {
        if (errno != EAGAIN)
        {
            perror ("Error in write!\n");
            exit (EXIT_FAILURE);
        }
        errno = 0;
        write_ret = 0;
    }

    conn->count -= write_ret;
    conn->head  += write_ret;

    if (conn->head == conn->size_buffer)
    {
        conn->head = 0;
    }
}
