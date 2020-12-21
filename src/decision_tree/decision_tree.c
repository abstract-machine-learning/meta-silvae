#include "decision_tree.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../data_structures/stack.h"


/***********************************************************************
 * Internal functions.
 **********************************************************************/

static void node_create(
    Node *node,
    const DecisionTree *tree,
    const Node *parent
) {
    unsigned int i;

    node->tree = (DecisionTree *) tree;
    node->parent = (Node *) parent;
    node->left = NULL;
    node->right = NULL;
    node->feature = 0;
    node->threshold = 0.0;
    node->frequencies = (unsigned int *) malloc(tree->dataset.n_labels * sizeof(unsigned int));
    node->probabilities = (double *) malloc(tree->dataset.n_labels * sizeof(double));
    node->labels = BITMASK_NOTHING;
    node->first_sample_idx = 0;
    node->last_sample_idx = 0;

    for (i = 0; i < tree->dataset.n_labels; ++i) {
        node->frequencies[i] = 0;
        node->probabilities[i] = 0.0;
    }
}


static void node_delete(Node *node) {
    Stack S;

    stack_create(&S);
    stack_push(S, node);
    while (!stack_is_empty(S)) {
        node = stack_pop(S);
        free(node->frequencies);
        free(node->probabilities);
        if (node->left) {
            stack_push(S, node->left);
        }
        if (node->right) {
            stack_push(S, node->right);
        }
        free(node);
    }
    stack_delete(&S);
}



/***********************************************************************
 * Public functions.
 **********************************************************************/

void decision_tree_create(DecisionTree *tree, const Dataset dataset) {
    unsigned int i, max = 0;
    Node *root;

    root = (Node *) malloc(sizeof(Node));
    tree->root = root;
    tree->dataset = dataset;
    tree->samples = (double **) malloc(dataset.size * sizeof(double *));

    node_create(root, tree, NULL);
    for (i = 0; i < dataset.size; ++i) {
        const unsigned int frequency = ++root->frequencies[dataset.label_lookup[i]];
        root->probabilities[dataset.label_lookup[i]] += 1.0 / dataset.size;
        if (frequency > max) {
            max = frequency;
        }
        tree->samples[i] = dataset.points + i * dataset.space_size;
    }

    for (i = 0; i < dataset.n_labels; ++i) {
        if (root->frequencies[i] == max) {
            bitmask_set(root->labels, i);
        }
    }

    root->last_sample_idx = dataset.size - 1;
}



void decision_tree_delete(DecisionTree tree) {
    node_delete(tree.root);
    free(tree.samples);
}



unsigned int decision_tree_n_nodes(const DecisionTree tree) {
    return node_n_reachable_nodes(*tree.root);
}



unsigned int decision_tree_n_leaves(const DecisionTree tree) {
    return node_n_reachable_leaves(*tree.root);
}



void decision_tree_leaves(Node *leaves, const DecisionTree tree) {
    node_reachable_leaves(leaves, *tree.root);
}



Bitmask decision_tree_classify(const DecisionTree tree, const double *point) {
    Node *node = tree.root;

    while (!node_is_leaf(*node)) {
        node = point[node->feature] <= node->threshold ? node->left : node->right;
    }

    return node->labels;
}



double decision_tree_similarity(const DecisionTree tree_1, const DecisionTree tree_2, const Dataset dataset) {
    unsigned int i, n_matches = 0;

    for (i = 0; i < dataset.size; ++i) {
        const double *point = dataset.points + i * dataset.space_size;
        n_matches += decision_tree_classify(tree_1, point) == decision_tree_classify(tree_2, point);
    }

    return (double) n_matches / dataset.size;
}



unsigned int node_is_root(const Node node) {
    return node.parent == NULL;
}



unsigned int node_is_leaf(const Node node) {
    return node.left == NULL && node.right == NULL;
}



unsigned int node_is_pure(const Node node) {
    unsigned int n_labels;

    bitmask_cardinality(node.labels, n_labels);

    return n_labels == 1;
}



unsigned int node_n_reachable_nodes(const Node node) {
    unsigned int n = 0;
    Node *current_node = (Node *) &node;
    Stack S;

    stack_create(&S);
    stack_push(S, current_node);
    while (!stack_is_empty(S)) {
        current_node = stack_pop(S);
        ++n;
        if (current_node->left != NULL) {
            stack_push(S, current_node->left);
        }
        if (current_node->right != NULL) {
            stack_push(S, current_node->right);
        }
    }
    stack_delete(&S);

    return n;
}



unsigned int node_n_reachable_leaves(const Node node) {
    unsigned int n = 0;
    Node *current_node = (Node *) &node;
    Stack S;

    stack_create(&S);
    stack_push(S, current_node);
    while (!stack_is_empty(S)) {
        current_node = stack_pop(S);
        n += node_is_leaf(*current_node);
        if (current_node->left != NULL) {
            stack_push(S, current_node->left);
        }
        if (current_node->right != NULL) {
            stack_push(S, current_node->right);
        }
    }
    stack_delete(&S);

    return n;
}



void node_reachable_leaves(Node *leaves, const Node node) {
    unsigned int n = 0;
    Node *current_node = (Node *) &node;
    Stack S;

    stack_create(&S);
    stack_push(S, current_node);
    while (!stack_is_empty(S)) {
        current_node = stack_pop(S);
        if (node_is_leaf(*current_node)) {
            leaves[n] = *current_node;
            ++n;
        }
        if (current_node->left != NULL) {
            stack_push(S, current_node->left);
        }
        if (current_node->right != NULL) {
            stack_push(S, current_node->right);
        }
    }
    stack_delete(&S);
}



double node_gini_impurity(const Node node) {
    unsigned int i;
    double gini_impurity = 1.0;

    for (i = 0; i < node.tree->dataset.n_labels; ++i) {
        gini_impurity -= node.probabilities[i] * node.probabilities[i];
    }

    return gini_impurity;
}



double node_entropy(const Node node) {
    unsigned int i;
    double entropy = 0.0;

    for (i = 0; i < node.tree->dataset.n_labels; ++i) {
        if (node.probabilities[i] > 0.0) {
            entropy -= node.probabilities[i] * log(node.probabilities[i]) / log(node.tree->dataset.n_labels);
        }
    }

    return entropy;
}



unsigned int node_depth(const Node node) {
    unsigned int depth = 0;
    Node *parent = node.parent;

    while (parent != NULL) {
        ++depth;
        parent = parent->parent;
    }

    return depth;
}



void node_prune(Node *node) {
    if (node->left) {
        node_delete(node->left);
        node->left = NULL;
    }
    if (node->right) {
        node_delete(node->right);
        node->right = NULL;
    }
}


SplitResult node_split(
    Node *node,
    const unsigned int feature,
    const double threshold
) {
    const unsigned int space_size = node->tree->dataset.space_size;
    const unsigned int *label_lookup = node->tree->dataset.label_lookup;
    const double *samples = node->tree->dataset.points;
    double **samples_lookup = node->tree->samples;
    unsigned int i, j, max_left = 0, max_right = 0;
    Node *left, *right;

    left = (Node *) malloc(sizeof(Node));
    right = (Node *) malloc(sizeof(Node));
    node_create(left, node->tree, node);
    node_create(right, node->tree, node);

    j = node->last_sample_idx;
    for (i = node->first_sample_idx; i <= j; ++i) {
        const double *sample_i = samples_lookup[i];
        const unsigned int label_i = label_lookup[(sample_i - samples) / space_size];
        double *sample_j = samples_lookup[j];
        unsigned int label_j = label_lookup[(sample_j - samples) / space_size];

        if (sample_i[feature] <= threshold) {
            ++left->frequencies[label_i];
            continue;
        }

        for (; j > i; --j) {
            sample_j = (double *) samples_lookup[j];
            label_j = label_lookup[(sample_j - samples) / space_size];

            if (sample_j[feature] <= threshold) {
                break;
            }

            ++right->frequencies[label_j];
        }

        if (i == j) {
            ++right->frequencies[label_j];
            --j;
            break;
        }
        else {
            ++left->frequencies[label_j];
            ++right->frequencies[label_i];
            samples_lookup[i] = sample_j;
            samples_lookup[j] = (double *) sample_i;
            --j;
        }
    }

    if (node->first_sample_idx >= j || j == node->last_sample_idx) {
        node_delete(left);
        node_delete(right);

        return node->last_sample_idx == j ? SPLIT_LEFT : SPLIT_RIGHT;
    }

    for (i = 0; i < node->tree->dataset.n_labels; ++i) {
        left->probabilities[i] = (double) left->frequencies[i] / (j - node->first_sample_idx + 1);
        right->probabilities[i] = (double) right->frequencies[i] / (node->last_sample_idx - j);
        if (left->frequencies[i] > max_left) {
            max_left = left->frequencies[i];
        }
        if (right->frequencies[i] > max_right) {
            max_right = right->frequencies[i];
        }
    }
    for (i = 0; i < node->tree->dataset.n_labels; ++i) {
        if (left->frequencies[i] == max_left) {
            bitmask_set(left->labels, i);
        }
        if (right->frequencies[i] == max_right) {
            bitmask_set(right->labels, i);
        }
    }

    node->feature = feature;
    node->threshold = threshold;
    node->left = left;
    node->right = right;
    left->parent = node;
    right->parent = node;
    left->first_sample_idx = node->first_sample_idx;
    left->last_sample_idx = j;
    right->first_sample_idx = j + 1;
    right->last_sample_idx = node->last_sample_idx;

    return SPLIT_BOTH;
}



void decision_tree_print(FILE *fh, DecisionTree tree) {
    Stack S;

    stack_create(&S);
    stack_push(S, tree.root);
    while (!stack_is_empty(S)) {
        const Node node = *((Node *) stack_pop(S));
        node_print(fh, node);
        if (node.right) {
            stack_push(S, node.right);
        }
        if (node.left) {
            stack_push(S, node.left);
        }
    }
    stack_delete(&S);
}



void node_print(FILE *fh, Node node) {
     unsigned int i;

     for (i = 0; i < node_depth(node); ++i) {
         fprintf(fh, "  ");
     }

     if (node_is_leaf(node)) {
         fprintf(fh, "LEAF ");
     }
     else {
         fprintf(fh, "SPLIT x_%u <= %.2g ", node.feature, node.threshold);
     }

     fprintf(fh, "[%u", node.frequencies[0]);
     for (i = 1; i < node.tree->dataset.n_labels; ++i) {
         fprintf(fh, ", %u", node.frequencies[i]);
     }
     fprintf(fh, "]\n");
}
