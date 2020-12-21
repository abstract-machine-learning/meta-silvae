#include "silva.h"

#include <string.h>
#include "../../data_structures/stack.h"


#define BUFFER_SIZE 32

/***********************************************************************
 * Internal functions.
 **********************************************************************/

static void parse_node(Node *node, FILE *fh);

static void skip_node(const unsigned int n_labels, FILE *fh);


static void parse_split(Node *node, FILE *fh) {
    unsigned int n, feature;
    double threshold;
    SplitResult split_result;

    n = fscanf(fh, "%u %lf", &feature, &threshold);
    if (n != 2) {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree node.\n", __FILE__, __LINE__);
        abort();
    }

    split_result = node_split(node, feature, threshold);
    switch (split_result) {
        case SPLIT_BOTH:
            parse_node(node->left, fh);
            parse_node(node->right, fh);
            break;
        case SPLIT_LEFT:
            parse_node(node, fh);
            skip_node(node->tree->dataset.n_labels, fh);
            break;
        case SPLIT_RIGHT:
            skip_node(node->tree->dataset.n_labels, fh);
            parse_node(node, fh);
            break;
    }
}



static void parse_leaf(Node *node, FILE *fh) {
    const unsigned int n_labels = node->tree->dataset.n_labels;
    unsigned int i, n, n_samples;

    for (i = 0; i < n_labels; ++i) {
        n = fscanf(fh, "%u", &n_samples);
        if (n != 1) {
            fprintf(stderr, "[%s: %d] Parse error.\n", __FILE__, __LINE__);
        }
    }
}



static void parse_logarithmic_leaf(Node *node, FILE *fh) {
    const unsigned int n_labels = node->tree->dataset.n_labels;
    unsigned int i, n;
    float value;

    for (i = 0; i < n_labels; ++i) {
        n = fscanf(fh, "%g", &value);
        if (n != 1) {
            fprintf(stderr, "[%s: %d] Parse error.\n", __FILE__, __LINE__);
        }
    }
}



static void parse_node(Node *node, FILE *fh) {
    unsigned int n;
    char node_type[BUFFER_SIZE];

    n = fscanf(fh, "%s", node_type);
    if (n != 1) {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree node.\n", __FILE__, __LINE__);
        abort();
    }

    if (strcmp(node_type, "LEAF") == 0) {
        parse_leaf(node, fh);
    }

    else if (strcmp(node_type, "LEAF_LOGARITHMIC") == 0) {
        parse_logarithmic_leaf(node, fh);
    }

    else if (strcmp(node_type, "SPLIT") == 0) {
        parse_split(node, fh);
    }

    else {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree node \"%s\".\n", __FILE__, __LINE__, node_type);
        abort();
    }
}



static void skip_split(const unsigned int n_labels, FILE *fh) {
    unsigned int n, feature;
    double threshold;

    n = fscanf(fh, "%u %lf", &feature, &threshold);
    if (n != 2) {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree node.\n", __FILE__, __LINE__);
        abort();
    }
    skip_node(n_labels, fh);
    skip_node(n_labels, fh);
}



static void skip_leaf(const unsigned int n_labels, FILE *fh) {
    unsigned int i, n_samples, n;

    for (i = 0; i < n_labels; ++i) {
        n = fscanf(fh, "%u", &n_samples);
        if (n != 1) {
            fprintf(stderr, "[%s: %d] Parse error.\n", __FILE__, __LINE__);
        }
    }
}



static void skip_logarithmic_leaf(const unsigned int n_labels, FILE *fh) {
    unsigned int i, n;
    float value;

    for (i = 0; i < n_labels; ++i) {
        n = fscanf(fh, "%g", &value);
        if (n != 1) {
            fprintf(stderr, "[%s: %d] Parse error.\n", __FILE__, __LINE__);
        }
    }
}



static void skip_node(const unsigned int n_labels, FILE *fh) {
    unsigned int n;
    char node_type[BUFFER_SIZE];

    n = fscanf(fh, "%s", node_type);
    if (n != 1) {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree node.\n", __FILE__, __LINE__);
        abort();
    }

    if (strcmp(node_type, "LEAF") == 0) {
        skip_leaf(n_labels, fh);
    }

    else if (strcmp(node_type, "LEAF_LOGARITHMIC") == 0) {
        skip_logarithmic_leaf(n_labels, fh);
    }

    else if (strcmp(node_type, "SPLIT") == 0) {
        skip_split(n_labels, fh);
    }

    else {
        fprintf(stderr, "[%s: %d] Cannot skip decision tree node \"%s\".\n", __FILE__, __LINE__, node_type);
        abort();
    }
}



/***********************************************************************
 * Pubblic functions.
 **********************************************************************/

void decision_tree_mapper_silva_load(FILE *fh, DecisionTree *tree, const Dataset dataset) {
    unsigned int n, i, space_size, n_labels;
    char buffer[BUFFER_SIZE];

    /* Parses header. */
    n = fscanf(fh, "%s", buffer);
    if (n != 1 || strcmp(buffer, "classifier-decision-tree") != 0) {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree.\n", __FILE__, __LINE__);
        abort();
    }

    /* Parses feature space size and number of labels */
    n = fscanf(fh, "%u %u", &space_size, &n_labels);
    if (n != 2) {
        fprintf(stderr, "[%s: %d] Cannot parse decision tree.\n", __FILE__, __LINE__);
        abort();
    }

    /* Parses labels */
    for (i = 0; i < n_labels; ++i) {
        n = fscanf(fh, "%s", buffer);
        if (n != 1) {
            fprintf(stderr, "[%s: %d] Cannot parse decision tree.\n", __FILE__, __LINE__);
            abort();
        }
    }

    /* Parses decision tree */
    decision_tree_create(tree, dataset);
    parse_node(tree->root, fh);
}



void decision_tree_mapper_silva_save(FILE *fh, const DecisionTree tree) {
    Stack S;
    unsigned int i;

    fprintf(fh, "classifier-decision-tree %u %u\n", tree.dataset.space_size, tree.dataset.n_labels);
    for (i = 0; i < tree.dataset.n_labels; ++i) {
        fprintf(fh, "%s ", tree.dataset.labels[i]);
    }
    fprintf(fh, "\n");

    stack_create(&S);
    stack_push(S, tree.root);
    while (!stack_is_empty(S)) {
        Node *node = stack_pop(S);

        if (node->left && node->right) {
            fprintf(fh, "SPLIT %u %g\n", node->feature, node->threshold);
            stack_push(S, node->right);
            stack_push(S, node->left);
        }
        else {
            fprintf(fh, "LEAF");
            for (i = 0; i < tree.dataset.n_labels; ++i) {
                fprintf(fh, " %u", node->frequencies[i]);
            }
            fprintf(fh, "\n");
        }
    }
    stack_delete(&S);
}
