#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <stdio.h>


typedef struct distribution Distribution;

struct distribution {
    double *values;
    unsigned int size;
    unsigned int is_sorted;
};

void distribution_create(Distribution *distribution, const unsigned int size);

void distribution_delete(Distribution distribution);

double distribution_min(Distribution *distribution);

double distribution_max(Distribution *distribution);

double distribution_median(Distribution *distribution);

double distribution_nth_percentile(Distribution *distribution, const unsigned int percentile);

double distribution_mean(Distribution *distribution);

double distribution_variance(Distribution *distribution);

void distribution_sort(Distribution *distribution);

void distribution_print(FILE *stream, const Distribution distribution);

#endif
