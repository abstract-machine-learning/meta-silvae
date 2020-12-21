#ifndef GEOMETRY_POINT_H
#define GEOMETRY_POINT_H

#include <stdio.h>
#include <stdlib.h>


typedef struct point Point;

struct point {
    double *data;
    unsigned int space_size;
};


#define point_create(point, size) \
    (point).data = (double *) malloc((size) * sizeof(double)); \
    (point).space_size = (size);


#define point_delete(point) \
    free((point).data); \
    (point).data = NULL; \
    (point).space_size = 0;


#define point_random(point, min, max) { \
    unsigned int i; \
    for (i = 0; i < (point).space_size; ++i) { \
        (point).data[i] = ((double) rand() / RAND_MAX) * (max - min) + min; \
    } \
}


#define point_print(fh, point) { \
    if ((point).space_size == 0) { \
        fprintf(fh, "NULL point"); \
    } \
    else { \
        unsigned int i; \
        fprintf(fh, "(%.2f", (point).data[0]); \
        for (i = 1; i < (point).space_size; ++i) { \
            fprintf(fh, ", %.2f", (point).data[i]); \
        } \
        fprintf(fh, ")"); \
    } \
}
    

#endif
