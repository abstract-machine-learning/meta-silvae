#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_structures/stack.h"
#include "data/dataset.h"
#include "decision_tree/decision_tree.h"
#include "decision_tree/mapper/silva.h"
#include "data_structures/distribution.h"


/***********************************************************************
 * Support functions
 **********************************************************************/

static void print_distribution(FILE *stream, Distribution *distribution) {
    fprintf(stream, "  min:          %g\n", distribution_min(distribution));
    fprintf(stream, "  1st quartile: %g\n", distribution_nth_percentile(distribution, 25));
    fprintf(stream, "  median:       %g\n", distribution_median(distribution));
    fprintf(stream, "  mean:         %g\n", distribution_mean(distribution));
    fprintf(stream, "  3rd quartile: %g\n", distribution_nth_percentile(distribution, 75));
    fprintf(stream, "  max:          %g\n", distribution_max(distribution));
    fprintf(stream, "  variance:     %g\n", distribution_variance(distribution));
}



static void compute_feature_frequencies(unsigned int *frequencies, const DecisionTree tree) {
    Stack S;

    memset(frequencies, 0, tree.dataset.space_size * sizeof(unsigned int));
    stack_create(&S);
    stack_push(S, tree.root);
    while (!stack_is_empty(S)) {
        Node *node = stack_pop(S);

        if (!node_is_leaf(*node)) {
            ++frequencies[node->feature];
            stack_push(S, node->left);
            stack_push(S, node->right);
        }
    }
    stack_delete(&S);
}



static void display_information(FILE *stream, const DecisionTree tree) {
    const unsigned int n_nodes = decision_tree_n_nodes(tree),
                       n_leaves = decision_tree_n_leaves(tree);
    unsigned int i, *feature_frequencies;
    Node *leaves = (Node *) malloc(n_leaves * sizeof(Node));
    Distribution heights, n_samples, entropies;

    distribution_create(&heights, n_leaves);
    distribution_create(&n_samples, n_leaves);
    distribution_create(&entropies, n_leaves);

    decision_tree_leaves(leaves, tree);
    for (i = 0; i < n_leaves; ++i) {
        heights.values[i] = (double) node_depth(leaves[i]);
        n_samples.values[i] = (double) (leaves[i].last_sample_idx - leaves[i].first_sample_idx + 1);
        entropies.values[i] = node_entropy(leaves[i]);
    }

    feature_frequencies = (unsigned int *) malloc(tree.dataset.space_size * sizeof(unsigned int));
    compute_feature_frequencies(feature_frequencies, tree);

    fprintf(stream, "Tree Info: %u leaves / %u nodes\n", n_leaves, n_nodes);
    fprintf(stream, "Heights:\n");
    print_distribution(stream, &heights);
    fprintf(stream, "Number of samples per leaf:\n");
    print_distribution(stream, &n_samples);
    fprintf(stream, "Entropy on leaves:\n");
    print_distribution(stream, &entropies);
    fprintf(stream, "Feature frequencies: [");
    for (i = 0; i < tree.dataset.space_size; ++i) {
        fprintf(stream, " %u", feature_frequencies[i]);
    }
    fprintf(stream, " ]\n");

    free(leaves);
    distribution_delete(heights);
    distribution_delete(n_samples);
    distribution_delete(entropies);
    free(feature_frequencies);
}





/***********************************************************************
 * Main entry point.
 **********************************************************************/

int main(int argc, char *argv[]) {
    DecisionTree tree;
    Dataset dataset;
    FILE *stream;

    /* Checks input */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <model path> <dataset path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Reads dataset */
    stream = fopen(argv[2], "r");
    dataset_load(stream, &dataset);
    fclose(stream);

    /* Reads decision tree */
    stream = fopen(argv[1], "r");
    decision_tree_mapper_silva_load(stream, &tree, dataset);
    fclose(stream);

    /* Displays information */
    display_information(stdout, tree);

    /* Frees memory */
    dataset_delete(dataset);
    decision_tree_delete(tree);

    return EXIT_SUCCESS;
}
