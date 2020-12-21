#ifndef GEOMETRY_HYPERRECTANGLE_H
#define GEOMETRY_HYPERRECTANGLE_H

typedef struct hyperrectangle Hyperrectangle;

struct hyperrectangle {
    unsigned int space_size;
    double *upperbounds;
    double *lowerbounds;
};



#define hyperrectangle_create(hyperrectangle, size) { \
    (hyperrectangle).space_size = (size); \
    (hyperrectangle).upperbounds = (double *) malloc((size) * sizeof(double)); \
    (hyperrectangle).lowerbounds = (double *) malloc((size) * sizeof(double)); \
}


#define hyperrectangle_delete(hyperrectangle) { \
    free((hyperrectangle).lowerbounds); \
    free((hyperrectangle).upperbounds); \
}


#define build_hypercube(hyperrectangle, center, radius) { \
    unsigned int i; \
    for (i = 0; i < (hyperrectangle).space_size; ++i) { \
        (hyperrectangle).lowerbounds[i] = (center)[i] - (radius); \
        (hyperrectangle).upperbounds[i] = (center)[i] + (radius); \
    } \
}


#define hyperrectangle_print(fh, hyperrectangle) { \
    unsigned int i; \
    fprintf((fh), "Hyperrectangle in R^%u:", (hyperrectangle).space_size); \
    for (i = 0; i < (hyperrectangle).space_size; ++i) { \
        fprintf((fh), " [%.2g, %.2g]", (hyperrectangle).lowerbounds[i], (hyperrectangle).upperbounds[i]); \
    } \
}

#endif
