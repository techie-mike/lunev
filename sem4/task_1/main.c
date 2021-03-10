#include "tree.h"
#include <stdio.h>

int main()
{
    struct BinTree tree;

    int ret = initTree (&tree);
    printf ("----------- ret = %d\n", ret);
    // printf ("%p %d %d", tree.array_nodes, tree.used, tree.capacity);
    
    ret = addInTree (&tree, 10);
    printf ("----------- ret add = %d\n", ret);

    ret = delTree (&tree);
    printf ("----------- ret del = %d\n", ret);
}
