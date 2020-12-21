#include "distribution.h"

#include <stdlib.h>

/***********************************************************************
 * Internal functions.
 **********************************************************************/
static int double_cmp(const void *a, const void *b) {
    const double diff = *((double *) a) - *((double *) b);
    return diff < 0.0
         ? -1
         : (diff > 0.0 ? +1 : 0);
}





/***********************************************************************
 * Public function.
 **********************************************************************/
void distribution_create(Distribution *distribution, const unsigned int size) {
    distribution->values = (double *) malloc(size * sizeof(double));
    distribution->size = size;
    distribution->is_sorted = 0;
}



void distribution_delete(Distribution distribution) {
    free(distribution.values);
}



double distribution_min(Distribution *distribution) {
    distribution_sort(distribution);

    return distribution->values[0];
}



double distribution_max(Distribution *distribution) {
    distribution_sort(distribution);

    return distribution->values[distribution->size - 1];
}


double distribution_median(Distribution *distribution) {
    return distribution_nth_percentile(distribution, 50);
}



double distribution_nth_percentile(Distribution *distribution, const unsigned int percentile) {
    const unsigned int index = percentile * distribution->size / 100;

    distribution_sort(distribution);

    return distribution->values[index];
}



double distribution_mean(Distribution *distribution) {
    unsigned int i;
    double sum = 0.0;

    for (i = 0; i < distribution->size; ++i) {
        sum += distribution->values[i];
    }

    return sum / distribution->size;
}



double distribution_variance(Distribution *distribution) {
    const double mean = distribution_mean(distribution);
    unsigned int i;
    double sum = 0.0;

    for (i = 0; i < distribution->size; ++i) {
        sum += (distribution->values[i] - mean) * (distribution->values[i] - mean);
    }

    return sum / distribution->size;
}



void distribution_sort(Distribution *distribution) {
    if (distribution->is_sorted) {
        return;
    }

    qsort(distribution->values, distribution->size, sizeof(double), double_cmp);
    distribution->is_sorted = 1;
}



void distribution_print(FILE *stream, const Distribution distribution) {
    unsigned int i;

    fprintf(stream, "Distribution (%u elements): [", distribution.size);
    for (i = 0; i < distribution.size; ++i) {
        fprintf(stream, " %g, ", distribution.values[i]);
    }
    fprintf(stream, "\n");
}
