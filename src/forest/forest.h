#ifndef FOREST_H
#define FOREST_H

#include <stdio.h>
#include "../decision_tree/decision_tree.h"

typedef struct forest Forest;

struct forest {
    DecisionTree *trees;
    unsigned int n_trees;
    unsigned int capacity;
};


void forest_create(Forest *forest);

void forest_delete(Forest *forest);

void forest_add_tree(Forest *forest, const DecisionTree tree);

void forest_print(FILE *fh, const Forest forest);

#endif
