#include "forest.h"

#define FOREST_INITIAL_CAPACITY 32

void forest_create(Forest *forest) {
    forest->trees = (DecisionTree *) malloc(FOREST_INITIAL_CAPACITY * sizeof(DecisionTree));
    forest->n_trees = 0;
    forest->capacity = FOREST_INITIAL_CAPACITY;
}



void forest_delete(Forest *forest) {
    free(forest->trees);
}



void forest_add_tree(Forest *forest, const DecisionTree tree) {
    if (forest->n_trees == forest->capacity) {
        forest->trees = (DecisionTree *) realloc(forest->trees, 2 * forest->capacity * sizeof(DecisionTree));
        forest->capacity *= 2;
    }

    forest->trees[forest->n_trees] = tree;
    ++forest->n_trees;
}



void forest_print(FILE *fh, const Forest forest) {
    unsigned int i;

    fprintf(fh, "FOREST (%u trees):\n", forest.n_trees);
    for (i = 0; i < forest.n_trees; ++i) {
        decision_tree_print(fh, forest.trees[i]);
    }
}
