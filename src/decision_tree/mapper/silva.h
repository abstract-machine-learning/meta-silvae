#ifndef DECISION_TREE_MAPPER_SILVA_H
#define DECISION_TREE_MAPPER_SILVA_H

#include <stdio.h>

#include "../decision_tree.h"

void decision_tree_mapper_silva_load(FILE *fh, DecisionTree *tree, const Dataset dataset);

void decision_tree_mapper_silva_save(FILE *fh, const DecisionTree tree);

#endif
