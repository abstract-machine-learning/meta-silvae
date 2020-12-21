#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/dataset.h"
#include "decision_tree/decision_tree.h"
#include "decision_tree/mapper/silva.h"
#include "forest/forest.h"
#include "forest/mapper/silva.h"

#define PATH_MAX_LENGTH 1024


/***********************************************************************
 * Support function.
 **********************************************************************/

static void display_usage(char *name) {
    fprintf(stderr, "Usage: %s assemble|disassemble <dataset path> <forest path> arguments\n", name);
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  for \"assemble\":  list of paths of tree to assemble into forest\n");
    fprintf(stderr, "  for \"dissemble\": path of directory to store trees\n");
}



static void assemble_forest(int argc, char **argv, const Dataset dataset) {
    Forest forest;
    FILE *stream;
    int i;

    /* Reads trees */
    forest_create(&forest);
    for (i = 1; i < argc; ++i) {
        DecisionTree tree;
        stream = fopen(argv[i], "r");
        decision_tree_mapper_silva_load(stream, &tree, dataset);
        fclose(stream);
        forest_add_tree(&forest, tree);
    }

    /* Exports forest */
    stream = fopen(argv[0], "w");
    forest_mapper_silva_save(stream, forest);
    fclose(stream);

    /* Frees memory */
    forest_delete(&forest);
}



static void disassemble_forest(int argc, char **argv, const Dataset dataset) {
    Forest forest;
    FILE *stream;
    unsigned int i;

    (void) argc;

    /* Reads forest */
    stream = fopen(argv[0], "r");
    forest_mapper_silva_load(stream, &forest, dataset);
    fclose(stream);

    /* Exports every tree */
    for (i = 0; i < forest.n_trees; ++i) {
        char path[PATH_MAX_LENGTH];
        sprintf(path, "%s/tree-%03d.silva", argv[1], i);
        stream = fopen(path, "w");
        decision_tree_mapper_silva_save(stream, forest.trees[i]);
        fclose(stream);
    }

    /* Frees memory */
    forest_delete(&forest);
}





/***********************************************************************
 * Main entry point
 **********************************************************************/
int main(int argc, char **argv) {
    FILE *stream;
    Dataset dataset;

    /* Checks input */
    if (argc < 5) {
        display_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Reads dataset */
    stream = fopen(argv[2], "r");
    dataset_load(stream, &dataset);
    fclose(stream);

    /* Executes operation */
    if (strcmp(argv[1], "assemble") == 0) {
        assemble_forest(argc - 3, argv + 3, dataset);
    }
    else if (strcmp(argv[1], "disassemble") == 0) {
        disassemble_forest(argc - 3, argv + 3, dataset);
    }
    else {
        display_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Frees memory */
    dataset_delete(dataset);

    return EXIT_SUCCESS;
}
