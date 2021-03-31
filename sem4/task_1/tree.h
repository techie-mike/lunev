#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>


struct BinTree
{
    struct Node* array_nodes;
    int capacity;
    int used;
    int root;
    int free_node;

    int number_increase_capacity;
};

int initTree  (struct BinTree* tree);
int addInTree (struct BinTree* tree, int data);
int delTree   (struct BinTree* tree);
int delInTreeByData (struct BinTree* tree, int data);

int changeNumberIncrease (struct BinTree* tree, int new_number);

int forEachTree (struct BinTree* tree,
                 void (*consumer)(struct BinTree*, int, void*),
                 void* data);

#endif