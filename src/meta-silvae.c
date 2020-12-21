#include <stdio.h>
#include <stdlib.h>

#include "data/dataset.h"
#include "decision_tree/decision_tree.h"
#include "decision_tree/mapper/silva.h"
#include "decision_tree/training/genetic_algorithm.h"


int main(int argc, char *argv[]) {
    Dataset training_set;
    DecisionTree tree;
    Status status;
    FILE *fh;

    /* Checks input */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <training set path> <output model path> [options]\n", argv[0]);
        ga_options_print(stderr);
        exit(EXIT_FAILURE);
    }


    /* Reads training set */
    fh = fopen(argv[1], "r");
    dataset_load(fh, &training_set);
    fclose(fh);

    /* Trains model */
    ga_status_init(&status);
    ga_parse_options(&status, argc, argv);
    status.callback_after_iter = callback_status_print;
    tree = ga_train(&status, training_set);

    /* Exports model */
    fh = fopen(argv[2], "w");
    decision_tree_mapper_silva_save(fh, tree);
    fclose(fh);

    /* Frees memory */
    dataset_delete(training_set);
    ga_status_clear(&status);

    return 0;
}
