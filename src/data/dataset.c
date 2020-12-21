#include "dataset.h"

#include <string.h>
#include <ctype.h>

#define LABEL_SIZE 32


/***********************************************************************
 * Internal functions.
 **********************************************************************/

typedef enum {
    DATASET_CSV,
    DATASET_BINARY
} DatasetFormat;



static void clear_fh(FILE *fh) {
    int c;

    do {
        c = fgetc(fh);
    }
    while (isspace(c));
    ungetc(c, fh);
}



static void parse_header(
    DatasetFormat *format,
    unsigned int *n_rows,
    unsigned int *n_cols,
    FILE *fh
) {
    const long initial_position = ftell(fh);
    unsigned int input_1, input_2;
    char next;
    int result;
    
    result = fscanf(fh, "# %u %u", &input_1, &input_2);
    if (result != 2) {
        fprintf(stderr, "[%s: %d] Cannot parse header.\n", __FILE__, __LINE__);
        abort();
    }

    next = fgetc(fh);
    fseek(fh, initial_position, SEEK_SET);

    switch (next) {
        case ' ':
            result = fscanf(fh, "# %u %u %u", format, n_rows, n_cols);
            if (result != 3) {
                fprintf(stderr, "[%s: %d] Cannot parse header.\n", __FILE__, __LINE__);
                abort();
            }
            break;

        case '\n':
            *format = DATASET_CSV;
            result = fscanf(fh, "# %u %u", n_rows, n_cols);
            if (result != 2) {
                fprintf(stderr, "[%s: %d] Cannot parse header.\n", __FILE__, __LINE__);
                abort();
            }
            break;

        default:
            fprintf(stderr, "[%s: %d] Cannot parse header.\n", __FILE__, __LINE__);
            abort();
    }

    clear_fh(fh);
}



static unsigned int label_index_lookup(Dataset *dataset, const char *label) {
    unsigned int i = 0;

    while (i < dataset->n_labels) {
        if (strcmp(label, dataset->labels[i]) == 0) {
            return i;
        }
        ++i;
    }

    dataset->labels = (char **) realloc(dataset->labels, (i + 1) * sizeof(char *));
    dataset->labels[i] = (char *) malloc(strlen(label) + 1);
    strcpy(dataset->labels[i], label);
    ++dataset->n_labels;

    return i;
}



static void dataset_read_csv(FILE *fh, Dataset *dataset) {
    char label_buffer[LABEL_SIZE];
    double *data = dataset->points;
    unsigned int n_cols = dataset->space_size, n_rows = dataset->size, i, j, result;

    for (i = 0; i < n_rows; ++i) {
        double buffer;

        result = fscanf(fh, "\n%[^,],", label_buffer);
        dataset->label_lookup[i] = label_index_lookup(dataset, label_buffer);

        for (j = 0; j < n_cols; ++j) {
            result = fscanf(fh, "%lf,", &buffer);
            data[i * n_cols + j] = buffer;
        }
    }

    (void) result;
}



static void compute_projections(Dataset *dataset) {
    const unsigned int space_size = dataset->space_size,
                       size = dataset->size;
    unsigned int i, j, k;
    unsigned int *n_projections = dataset->n_projections;
    double **projections = dataset->projections;

    for (i = 0; i < space_size; ++i) {
        for (j = 0; j < size; ++j) {
            const double value = dataset->points[j * space_size + i];

            k = 0;
            while (k < n_projections[i] && value > projections[i][k]) {
                ++k;
            }
            if (k < n_projections[i] && value == projections[i][k]) {
                continue;
            }

            projections[i] = (double *) realloc(projections[i], (n_projections[i] + 1) * sizeof(double));
            memmove(projections[i] + k + 1, projections[i] + k, (n_projections[i] - k) * sizeof(double));
            projections[i][k] = value;
            ++n_projections[i];
        }
    }
}





/***********************************************************************
 * Public functions.
 **********************************************************************/

void dataset_create(Dataset *dataset, const unsigned int space_size, const unsigned int size) {
    unsigned int i;

    dataset->space_size = space_size;
    dataset->size = size;

    dataset->points = (double *) malloc(size * space_size * sizeof(double));
    dataset->n_labels = 0;
    dataset->labels = NULL;
    dataset->label_lookup = (unsigned int *) malloc(size * sizeof(unsigned int));
    dataset->projections = (double **) malloc(space_size * sizeof(double *));;
    dataset->n_projections = (unsigned int *) malloc(space_size * sizeof(unsigned int));
    for (i = 0; i < space_size; ++i) {
        dataset->projections[i] = NULL;
        dataset->n_projections[i] = 0;
    }
}



void dataset_delete(Dataset dataset) {
    unsigned int i;

    free(dataset.points);
    for (i = 0; i < dataset.n_labels; ++i) {
        free(dataset.labels[i]);
    }
    free(dataset.labels);
    free(dataset.label_lookup);
    for (i = 0; i < dataset.space_size; ++i) {
        free(dataset.projections[i]);
    }
    free(dataset.projections);
    free(dataset.n_projections);
}



void dataset_load(FILE *fh, Dataset *dataset) {
    unsigned int n_rows, n_cols;
    DatasetFormat format;

    if (!fh) {
        fprintf(stderr, "[%s: %d] Cannot read dataset file.\n", __FILE__, __LINE__);
        abort();
    }

    parse_header(&format, &n_rows, &n_cols, fh);
    dataset_create(dataset, n_cols, n_rows);

    switch (format) {
        case DATASET_CSV:
            dataset_read_csv(fh, dataset);
            break;

        default:
            fprintf(stderr, "Unknown dataset type.\n");
            abort();
    }

    compute_projections(dataset);
}



unsigned int dataset_label_lookup(const Dataset dataset, const double point[]) {
    return dataset.label_lookup[(point - dataset.points) / dataset.space_size];
}



void dataset_print(FILE *fh, const Dataset dataset) {
    unsigned int i, j;

    fprintf(
        fh, "Dataset in R^%u, contains %u samples and %u labels.\n",
        dataset.space_size, dataset.size, dataset.n_labels
    );
    fprintf(fh, "Labels: {");
    for (i = 0; i < dataset.n_labels; ++i) {
        fprintf(fh, "%s ", dataset.labels[i]);
    }
    fprintf(fh, "}\n");

    fprintf(fh, "Points: [\n");
    for (i = 0; i < dataset.size; ++i) {
        fprintf(fh, "  %s ", dataset.labels[dataset.label_lookup[i]]);
        for (j = 0; j < dataset.space_size; ++j) {
            fprintf(fh, "%.2g ", dataset.points[i * dataset.space_size + j]);
        }
        fprintf(fh, "\n");
    }
    fprintf(fh, "]\n");

    fprintf(fh, "Projections: [\n");
    for (i = 0; i < dataset.space_size; ++i) {
        fprintf(fh, "  x_%u (%u): [", i, dataset.n_projections[i]);
        for (j = 0; j < dataset.n_projections[i]; ++j) {
            fprintf(fh, "%.2g  ", dataset.projections[i][j]);
        }
        fprintf(fh, "]\n");
    }
    fprintf(fh, "]");
}
