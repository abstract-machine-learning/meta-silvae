#ifndef DECISION_TREE_DECISION_TREE_H
#define DECISION_TREE_DECISION_TREE_H

#include <stdio.h>

#include "../data/dataset.h"
#include "../data_structures/bitmask.h"


typedef struct decision_tree DecisionTree;
typedef struct node Node;

struct decision_tree {
    Node *root;
    Dataset dataset;
    double **samples;
};

struct node {
    DecisionTree *tree;
    Node *parent;
    Node *left;
    Node *right;
    unsigned int feature;
    double threshold;
    unsigned int *frequencies;
    double *probabilities;
    Bitmask labels;
    unsigned int first_sample_idx;
    unsigned int last_sample_idx;
};

enum split_result {
    SPLIT_LEFT = 0x1,
    SPLIT_RIGHT = 0x2,
    SPLIT_BOTH = 0x3
};


typedef enum split_result SplitResult;


void decision_tree_create(DecisionTree *tree, const Dataset dataset);

void decision_tree_delete(DecisionTree tree);

unsigned int decision_tree_n_nodes(const DecisionTree tree);

unsigned int decision_tree_n_leaves(const DecisionTree tree);

void decision_tree_leaves(Node *leaves, const DecisionTree tree);

Bitmask decision_tree_classify(const DecisionTree tree, const double *point);

double decision_tree_similarity(const DecisionTree tree_1, const DecisionTree tree_2, const Dataset dataset);


unsigned int node_is_root(const Node node);

unsigned int node_is_leaf(const Node node);

unsigned int node_is_pure(const Node node);

unsigned int node_n_reachable_nodes(const Node node);

unsigned int node_n_reachable_leaves(const Node node);

void node_reachable_leaves(Node *leaves, const Node node);

double node_gini_impurity(const Node node);

double node_entropy(const Node node);

unsigned int node_depth(const Node node);


void node_prune(Node *node);

SplitResult node_split(Node *node, const unsigned int feature, const double threshold);


void decision_tree_print(FILE *fh, DecisionTree tree);

void node_print(FILE *fh, Node node);

#endif
