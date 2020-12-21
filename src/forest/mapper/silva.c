#include "silva.h"

#include <string.h>
#include "../../decision_tree/mapper/silva.h"

#define BUFFER_SIZE 64


void forest_mapper_silva_load(FILE *fh, Forest *forest, const Dataset dataset) {
    unsigned int n_trees, i, n;
    char buffer[BUFFER_SIZE];

    if (!fh) {
        fprintf(stderr, "[%s: %d] Cannot read file.\n", __FILE__, __LINE__);
        abort();
    }

    n = fscanf(fh, "%s %u", buffer, &n_trees);
    if (n != 2 || strcmp(buffer, "classifier-forest") != 0) {
        fprintf(stderr, "[%s: %d] Cannot parse random forest.\n", __FILE__, __LINE__);
        abort();
    }


    forest_create(forest);
    for (i = 0; i < n_trees; ++i) {
        DecisionTree tree;
        decision_tree_mapper_silva_load(fh, &tree, dataset);
        forest_add_tree(forest, tree);
    }
}



void forest_mapper_silva_save(FILE *fh, const Forest forest) {
    unsigned int i;

    fprintf(fh, "classifier-forest %u\n", forest.n_trees);
    for (i = 0; i < forest.n_trees; ++i) {
        decision_tree_mapper_silva_save(fh, forest.trees[i]);
    }
}
