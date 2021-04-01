#ifndef TEST_TREE_H
#define TEST_TREE_H

#include "tree.h"

void runTest ();
void testCreate ();
void testDel ();
void testAddInTree ();
void testChangeNumberIncrease ();
void testForEachTree ();
void testDelInTreeByData ();



void* myCalloc (size_t num, size_t size);
void* myRealloc ( void *ptr, size_t new_size);


#endif