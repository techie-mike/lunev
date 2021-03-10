#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_CAPACITY 10
#define NO_VALUE -1
#define LEFT 1
#define RIGHT 2



// enum
// {
//     default_capacity = 10
// } tree_constants;

struct Node
{
    int data;
    int left;
    int right;
    int parent;
};

struct InsertPlace
{
    int turn;
    int node;
};

struct BinTree
{
    struct Node* array_nodes;
    int capacity;
    int used;
    int root;
    int free_node;

    int number_increase_capacity;
};

int initTree (struct BinTree* tree);
int addInTree (struct BinTree* tree, int data);
int delTree (struct BinTree* tree);


void changeNumberIncrease (int new_number);



// ----------------PRIVARE----------------
int increaseCapacity (struct BinTree* tree);
int firstAlloc (struct BinTree* tree);

int searchParentForNewNode (struct BinTree* tree, struct InsertPlace* place,
    int node, int new_data);

int createNode (struct BinTree* tree);
void fillDefaulValueNodes (struct BinTree* tree, int from, int to);



#endif