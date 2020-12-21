#include "performance.h"
#include "../../geometry/hyperrectangle.h"
#include "../../data_structures/stack.h"


/***********************************************************************
 * Internal functions.
 **********************************************************************/

static unsigned int decision_tree_is_stable(
    const DecisionTree tree,
    double *sample,
    const double epsilon,
    const Bitmask labels,
    Stack S
) {
    unsigned int is_stable = 1;

    stack_clear(S);
    stack_push(S, tree.root);
    while (!stack_is_empty(S)) {
        Node *node = stack_pop(S);

        if (node_is_leaf(*node)) {
            if (labels != node->labels) {
                is_stable = 0;
                break;
            }
        }

        else {
            const unsigned int idx = node->feature;
            const double k = node->threshold;

            if (sample[idx] - epsilon <= k) {
                stack_push(S, node->left);
            }
            if (sample[idx] + epsilon > k) {
                stack_push(S, node->right);
            }
        }
    }

    return is_stable;
}





/***********************************************************************
 * Public functions.
 **********************************************************************/

void decision_tree_performance(Performance *performance, const DecisionTree tree, const Dataset dataset, const double epsilon) {
    unsigned int i;
    Stack S;

    performance->samples = dataset.size;
    performance->correct = 0;
    performance->wrong = 0;
    performance->stable = 0;
    performance->unstable = 0;
    performance->robust = 0;
    performance->vulnerable = 0;
    performance->fragile = 0;
    performance->broken = 0;

    stack_create(&S);
    for (i = 0; i < dataset.size; ++i) {
        double *sample = dataset.points + i * dataset.space_size;
        unsigned int label = dataset.label_lookup[i];
        unsigned int is_correct, is_stable;
        Bitmask labels = decision_tree_classify(tree, sample);
        unsigned int n_labels;

        bitmask_cardinality(labels, n_labels);

        is_correct = n_labels == 1 && bitmask_is_set(labels, label);
        is_stable = decision_tree_is_stable(tree, sample, epsilon, labels, S);

        performance->correct += is_correct;
        performance->wrong += 1 - is_correct;
        performance->stable += is_stable;
        performance->unstable += 1 - is_stable;
        performance->robust += is_correct && is_stable;
        performance->vulnerable += (1 - is_correct) && is_stable;
        performance->fragile += is_correct && (1 - is_stable);
        performance->broken += (1 - is_correct) && (1 - is_stable);
    }
    stack_delete(&S);
}



void decision_tree_performance_print(FILE *fh, const Performance performance) {
    fprintf(fh, "size: %u     correct: %u    stable: %u\n", performance.samples, performance.correct, performance.stable);
}
