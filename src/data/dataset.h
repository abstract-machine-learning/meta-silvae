#ifndef DATA_DATASET_H
#define DATA_DATASET_H

#include <stdio.h>
#include <stdlib.h>

typedef struct dataset Dataset;

struct dataset {
    unsigned int space_size;
    unsigned int size;
    double *points;
    char **labels;
    unsigned int n_labels;
    unsigned int *label_lookup;
    double **projections;
    unsigned int *n_projections;
};


void dataset_create(Dataset *dataset, const unsigned int space_size, const unsigned int size);

void dataset_delete(Dataset dataset);

void dataset_load(FILE *fh, Dataset *dataset);

unsigned int dataset_label_lookup(const Dataset dataset, const double point[]);

void dataset_print(FILE *fh, const Dataset dataset);

#endif
