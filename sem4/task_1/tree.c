#include "tree.h"

// Private declaration
void goRoundTree (struct BinTree* tree, int index,
                 void (*consumer)(struct BinTree*, int, void*),
                 void* data);

// Private declaration

int initTree (struct BinTree* tree)
{
    tree->capacity = 0;
    tree->used = 0;
    tree->number_increase_capacity = 10;
    tree->array_nodes = NULL;
    tree->root = -1;
    tree->free_node = -1;
    if (firstAlloc (tree) == -1)
        return -1;
    else
        return 0;
}

int addInTree (struct BinTree* tree, int new_data)
{
    int index = createNode (tree);
    if (tree->root == NO_VALUE)
    {
        tree->root = index;
    }
    else
    {
        struct InsertPlace place = {};
        int ret_search = searchParentForNewNode (
        	tree, &place, tree->root, new_data);
        printf ("\t\tret_search = %d\n", ret_search);
        if (ret_search == -1)
            return -1;

        if (place.turn == LEFT)
        {
            (tree->array_nodes[place.node]).left = index;
            (tree->array_nodes[index]).parent = place.node;
        }
        else
        {
            (tree->array_nodes[place.node]).right = index;
            (tree->array_nodes[index]).parent = place.node;
        }
        // (tree->array_nodes[tree->used]).data = new_data;
        tree->array_nodes[index].data = new_data;

    }

    return 0;
}

int increaseCapacity (struct BinTree* tree)
{
    printf ("cap = %d\tinc = %d\n", tree->capacity, tree->number_increase_capacity);
    struct Node* temp_pointer = realloc (tree->array_nodes, 
        (tree->capacity + tree->number_increase_capacity) * sizeof (struct Node));
    if (temp_pointer == NULL)
    {
        return -1;
    }
    else
    {
        tree->array_nodes = temp_pointer;
        int new_capacity = tree->capacity + tree->number_increase_capacity;
        int old_capacity = tree->capacity;
        fillDefaulValueNodes (tree, old_capacity, tree->number_increase_capacity);
        tree->capacity = new_capacity;
        return 0;
    }
}

int firstAlloc (struct BinTree* tree)
{
    struct Node* temp_pointer = calloc (DEFAULT_CAPACITY, sizeof (struct Node));
    if (temp_pointer != NULL)
    {
        tree->array_nodes = temp_pointer;
        fillDefaulValueNodes (tree, 0, DEFAULT_CAPACITY);


        tree->capacity = DEFAULT_CAPACITY;
        return 0;
    }
    else
    {
        return -1;
    }
}

int delTree (struct BinTree* tree)
{
    if (tree->array_nodes == NULL)
    {
        return -1;
    }

    free (tree->array_nodes);
    tree->array_nodes = NULL;
    tree->capacity = 0;
    tree->used = 0;
    return 0;
}

int searchParentForNewNode (struct BinTree* tree, struct InsertPlace* place,
    int node, int new_data)
{
    if (tree->array_nodes[node].data > new_data)
    {
        if (tree->array_nodes[node].left == NO_VALUE)
        {
            place->turn = LEFT;
            place->node = node;
            return 0;
        }
        else
        {
            return searchParentForNewNode (tree, place,
                (tree->array_nodes[node]).left, new_data);
        }
    }
    else if (tree->array_nodes[node].data < new_data)
    {
        if (tree->array_nodes[node].right == NO_VALUE)
        {
            place->turn = RIGHT;
            place->node = node;
            return 0;
        }
        else
        {
            return searchParentForNewNode (tree, place,
                (tree->array_nodes[node]).right, new_data);
        }
    }
    else
        return -1;
}

int createNode (struct BinTree* tree)
{
    printf ("capacity = %d\tused = %d\n", tree->capacity, tree->used);
    if (tree->capacity == tree->used)
    {
        if (increaseCapacity (tree))
            return -1;
    }
    // Index 'free_node' always have valid right branch with next free node
    tree->used++;

    int old_free_node = tree->free_node;
    tree->free_node = tree->array_nodes[tree->free_node].right;
    tree->array_nodes[old_free_node].right = NO_VALUE;
    return old_free_node;
}

void fillDefaulValueNodes (struct BinTree* tree, int from, int num_elem)
// Filling including start and end number
{
    struct Node* temp_pointer = tree->array_nodes;
    int to = from + num_elem; 
    for (int i = from; i < to; i++)
    {
        temp_pointer[i].left   = NO_VALUE;
        temp_pointer[i].right  = i + 1;
        temp_pointer[i].parent = NO_VALUE;
    }
    temp_pointer[to - 1].right = NO_VALUE;
    tree->free_node = from;
    return;
}

void goRoundTree (struct BinTree* tree, int index,
                 void (*consumer)(struct BinTree*, int, void*),
                 void* data)
{
    
    // printf ("Index root %d\n\tleft = %d\n\tright = %d\n", tree->root, tree->array_nodes[tree->root].left, tree->array_nodes[tree->root].right);
    dumpNode (tree, index);
    if (tree->array_nodes[index].left != NO_VALUE)
        goRoundTree (tree, (tree->array_nodes[index]).left, consumer, data);
    (*consumer) (tree, index, data);
    if (tree->array_nodes[index].right != NO_VALUE)
        goRoundTree (tree, (tree->array_nodes[index]).right, consumer, data);
}

int forEachTree (struct BinTree* tree,
                 void (*consumer)(struct BinTree*, int, void*),
                 void* data)
{
    if (tree == NULL || consumer == NULL)
        return 0;

    if (tree->root == NO_VALUE)
        return -1;
    printf ("Index root %d\n\tleft = %d\n\tright = %d\n", tree->root, tree->array_nodes[tree->root].left, tree->array_nodes[tree->root].right);

    goRoundTree (tree, tree->root, consumer, data);
    return 0;
}

void dumpNode (struct BinTree* tree, int index)
{
    printf ("----DUMP-NODE-[%d]----\n", index);
    printf ("\tdata = %d\n", tree->array_nodes[index].data);
    printf ("\tleft = %d\n", tree->array_nodes[index].left);
    printf ("\tright = %d\n", tree->array_nodes[index].right);
    printf ("----------------------\n");
    
}
