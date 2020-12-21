#ifndef DECISION_TREE_PERFORMANCE_PERFORMANCE_H
#define DECISION_TREE_PERFORMANCE_PERFORMANCE_H

#include <stdio.h>

#include "../decision_tree.h"
#include "../../data/dataset.h"

typedef struct performance Performance;

struct performance {
    unsigned int samples;
    unsigned int correct;
    unsigned int wrong;
    unsigned int stable;
    unsigned int unstable;
    unsigned int robust;
    unsigned int vulnerable;
    unsigned int fragile;
    unsigned int broken;
    double time;
};


void decision_tree_performance(Performance *performance, const DecisionTree tree, const Dataset dataset, const double epsilon);

void decision_tree_performance_print(FILE *fh, const Performance performance);

#endif
