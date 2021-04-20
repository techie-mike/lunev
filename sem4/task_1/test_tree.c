#include "test_tree.h"

#define PRINT_ERROR \
do {\
    printf ("\033[31m====ERROR=IN=TEST=%s====\033[0m\n", __FUNCTION__);\
    delTree (tree);\
    exit (1);\
} while (0)

#define TEST_PASSED \
do {\
    printf ("\033[32m====PASSED=%s====\033[0m\n", __FUNCTION__);\
} while (0)



void runTest ()
{
    testCreate ();
    testDel ();
    testAddInTree ();
    testChangeNumberIncrease ();
    testForEachTree ();
    testDelInTreeByData ();
}

void testAddInTree ()
{
    BinTree_t* tree = createTree ();
    changeNumberIncrease (tree, 1);

    int ret = 0;
    //-----------------------------------------------------
    for (int i = 0; i < 10 + 50; i++)
    {
        ret = addInTree (tree, 30 - i);
        if (ret != 0 && i != 59)
        {
            PRINT_ERROR;
        }

    }

    //-----------------------------------------------------
    ret = addInTree (tree, 30);
    if (ret != -1)
    {
        printf ("Wait -1, return %d\n", ret);
        PRINT_ERROR;
    }

    //-----------------------------------------------------
    for (int i = 31; i < 33; i++)
    {
        ret = addInTree (tree, i);

        if (ret != 0)
        {
            printf ("Wait 0, return %d\n", ret);
            PRINT_ERROR;
        }
    }

    delTree (tree);
    TEST_PASSED;
}

void testDel ()
{
    BinTree_t* tree = NULL;
    delTree (tree);

    if (delTree (NULL) != -1)
        PRINT_ERROR;
    TEST_PASSED;
}

void testCreate ()
{
    BinTree_t* tree = NULL;

    // int ret = createTree (NULL);
    // if (ret != -1)
    //     PRINT_ERROR;

    for (int i = 0; i < 50; i++)
    {
        tree = createTree ();
        delTree (tree);
    }
    TEST_PASSED;
}

void testChangeNumberIncrease ()
{
    BinTree_t* tree = createTree ();
    changeNumberIncrease (tree, 0);
    changeNumberIncrease (NULL, 0);
    delTree (tree);
    TEST_PASSED;
}

void emptyFunc (BinTree_t* tree, int index, void* data) {}

void testForEachTree ()
{
    BinTree_t* tree = createTree ();
    for (int i = 0; i < 2; i++)
    {
        addInTree (tree, i);
    }
    addInTree (tree, -1);
    forEachTree (tree, emptyFunc, NULL);
    //-----------------------------------------------------

    forEachTree (tree, NULL, NULL);
    forEachTree (NULL, NULL, NULL);
    delTree (tree);

    int ret = addInTree (NULL, 10);
    if (ret != -1)
        PRINT_ERROR;

    TEST_PASSED;
}

void testDelInTreeByData ()
{
    BinTree_t* tree = createTree ();
    addInTree (tree, 10);
    addInTree (tree, 7);
    addInTree (tree, 12);
    addInTree (tree, 11);

    int ret = delInTreeByData (tree, 10);
    if (ret != 0)
        PRINT_ERROR;

    ret = delInTreeByData (tree, 7);
    if (ret != 0)
        PRINT_ERROR;
    forEachTree (tree, emptyFunc, NULL);


    ret = delInTreeByData (tree, 11);
    if (ret != 0)
        PRINT_ERROR;


    ret = delInTreeByData (tree, 12);
    if (ret != 0)
        PRINT_ERROR;

    addInTree (tree, 10);
    addInTree (tree, 7);

    ret = delInTreeByData (tree, 10);
    if (ret != 0)
        PRINT_ERROR;

    addInTree (tree, 8);

    ret = delInTreeByData (tree, 8);
    if (ret != 0)
        PRINT_ERROR;
    //----------------------------------------------------
    // Left only 7 in tree
    ret = delInTreeByData (tree, 9);
    if (ret != -1)
        PRINT_ERROR;

    ret = delInTreeByData (tree, 7);
    if (ret != 0)
        PRINT_ERROR;
    //-----------------------------------------------------

    ret = delInTreeByData (tree, 8);
    if (ret != -1)
        PRINT_ERROR;

    //-----------------------------------------------------

    addInTree (tree, 10);
    addInTree (tree, 11);
    addInTree (tree, 12);


    ret = delInTreeByData (tree, 11);
        if (ret != 0)
            PRINT_ERROR;

    ret = delInTreeByData (tree, 12);
        if (ret != 0)
            PRINT_ERROR;

    addInTree (tree, 9);
    addInTree (tree, 8);

    ret = delInTreeByData (tree, 9);
        if (ret != 0)
            PRINT_ERROR;
    //-----------------------------------------------------

    ret = delInTreeByData (tree, 6);
        if (ret != -1)
            PRINT_ERROR;

    //-----------------------------------------------------

    ret = delInTreeByData (NULL, 8);
    if (ret != -1)
        PRINT_ERROR;

    delTree (tree);
    TEST_PASSED;
}
