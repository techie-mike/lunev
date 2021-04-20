#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>


typedef struct bin_tree BinTree_t;

BinTree_t* createTree  ();
int addInTree (BinTree_t* tree, int data);
int delTree   (BinTree_t* tree);
int delInTreeByData (BinTree_t* tree, int data);

int changeNumberIncrease (BinTree_t* tree, int new_number);

int forEachTree (BinTree_t* tree,
                 void (*consumer)(BinTree_t*, int, void*),
                 void* data);

#endif