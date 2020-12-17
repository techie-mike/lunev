#include "src.h"


int main (int argc, const char* argv[])
{
    if (argc < 3)
    {
        fprintf (stderr, "Fogot about file or number children!\n");
        exit (EXIT_FAILURE);
    }
    size_t num_children = strtoul (argv[1], 0, 10);

    CHECK (num_children);

    struct ChildInfo *children = (struct ChildInfo*) calloc (num_children, sizeof (struct ChildInfo));
    if (children == NULL)
    {
        fprintf (stderr, "Error in calloc!\n");
        exit (EXIT_FAILURE);
    }

    for (size_t i = 0; i < num_children; i++)
    {
        int fd_child_in[2]  = {};
        int fd_child_out[2] = {};

        CHECK (pipe (fd_child_in));
        CHECK (pipe (fd_child_out));

        int pid = fork ();
        CHECK (pid);

        if (pid == 0)
        {
            CHECK (close (fd_child_in[WR_END]));
            CHECK (close (fd_child_out[RD_END]));

            struct ChildInfo my_info = { fd_child_in[RD_END],
                                         fd_child_out[WR_END] };


            // Close previous children
            for (long k = 0; k < i; k++)
            {
                CHECK (close (children[k].fd_in));
                CHECK (close (children[k].fd_out));
            }

            if (i == 0)
            {
                int fd_file = open (argv[2], O_RDONLY);

                if (fd_file == -1)
                {
                    perror ("Error in open text file!");
                    free (children);
                    exit (EXIT_FAILURE);
                }

                close (my_info.fd_in);
                my_info.fd_in = fd_file;
            }

            free (children);
            children = NULL;

            //---------------------------------------------
            ssize_t splice_ret = 1;
            while (splice_ret != 0)
            {
                splice_ret = splice (my_info.fd_in,   NULL,
                                     my_info.fd_out,  NULL,
                                     SIZE_DATA_BLOCK, 0    );
                CHECK (splice_ret);
            }
            CHECK (close (my_info.fd_in));
            CHECK (close (my_info.fd_out));
            //-----------------------------------------
            exit (EXIT_SUCCESS);
        }
        else
        {
            children[i].fd_in  = fd_child_in[WR_END];
            children[i].fd_out = fd_child_out[RD_END];
            CHECK (fcntl (fd_child_in[WR_END],  F_SETFL, O_NONBLOCK));
            CHECK (fcntl (fd_child_out[RD_END], F_SETFL, O_NONBLOCK));

            CHECK (close (fd_child_in[RD_END]));
            CHECK (close (fd_child_out[WR_END]));
        }
    }

    struct Connection *connectors = (struct Connection*) calloc (num_children, sizeof (struct Connection));
    if (connectors == NULL)
    {
        fprintf (stderr, "Error in calloc connection!\n");
        free (children);
        children = NULL;
        exit (EXIT_FAILURE);
    }

    createConnections (connectors, children, num_children);
    free (children);
    children = NULL;

    // Main work of parent
    mainJobAsServer (connectors, num_children);

    for (size_t i = 0; i < num_children; i++)
    {
        free (connectors[i].buffer);
    }
    free (connectors);
}

