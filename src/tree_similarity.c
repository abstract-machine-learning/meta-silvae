#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/dataset.h"
#include "decision_tree/decision_tree.h"
#include "decision_tree/mapper/silva.h"
#include "data_structures/distribution.h"


/***********************************************************************
 * Main entry point.
 **********************************************************************/

int main(int argc, char *argv[]) {
    DecisionTree tree_1, tree_2;
    Dataset dataset;
    FILE *stream;

    /* Checks input */
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <first tree path> <second tree path> <dataset path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Reads dataset */
    stream = fopen(argv[3], "r");
    dataset_load(stream, &dataset);
    fclose(stream);

    /* Reads decision trees */
    stream = fopen(argv[1], "r");
    decision_tree_mapper_silva_load(stream, &tree_1, dataset);
    fclose(stream);
    stream = fopen(argv[2], "r");
    decision_tree_mapper_silva_load(stream, &tree_2, dataset);
    fclose(stream);

    /* Displays information */
    printf("Similarity: %g\n", decision_tree_similarity(tree_1, tree_2, dataset));

    /* Frees memory */
    dataset_delete(dataset);
    decision_tree_delete(tree_1);
    decision_tree_delete(tree_2);

    return EXIT_SUCCESS;
}

