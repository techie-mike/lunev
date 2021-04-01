#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>


typedef struct bin_tree BinTree;

BinTree* createTree  ();
int addInTree (BinTree* tree, int data);
int delTree   (BinTree* tree);
int delInTreeByData (BinTree* tree, int data);

int changeNumberIncrease (BinTree* tree, int new_number);

int forEachTree (BinTree* tree,
                 void (*consumer)(BinTree*, int, void*),
                 void* data);

#endif