#ifndef BITMASK_H
#define BITMASK_H

typedef long unsigned Bitmask;

typedef void (*BitmaskWalk)(const long unsigned int, const long unsigned int, void *);

#define BITMASK_ALL ((long unsigned int) -1)
#define BITMASK_NOTHING 0UL
#define BITMASK_SIZE ((size_t) sizeof(Bitmask) << 3)


#define bitmastk_create(b) \
    (b = BITMASK_NOTHING)

#define bitmask_set(b, i) \
    (b |= 1UL << i)

#define bitmask_unset(b, i) \
    (b &= ~(1UL << i))

#define bitmask_toggle(b, i) \
    (b ^= 1UL << u)

#define bitmask_complement(b) \
    (~b)

#define bitmask_union(a, b) \
    ((a) | (b))

#define bitmask_intersection(a, b) \
    ((a) & (b))

#define bitmask_difference(a, b) \
    ((a) & ~(b))

#define bitmask_symmetric_difference(a, b) \
    ((a) ^ (b))

#define bitmask_fma(a, b, c) \
    (((a) & (b)) | (c))

#define bitmask_equality(a, b) \
    ((a) == (b))

#define bitmask_compare(a, b) \
    (bitmask_is_subset(a, b) ? -1 : bitmask_is_superset(a, b))

#define bitmask_is_subset(a, b) \
    (((a) & (b)) == (a))

#define bitmask_is_superset(a, b) \
    bitmask_is_subset(b, a)

#define bitmask_is_set(b, i) \
    (((b) >> i) & 1UL)


#define bitmask_cardinality(b, cardinality) { \
    Bitmask t = (b); \
    cardinality = 0; \
    while (t) { \
        t &= (t - 1); \
        ++cardinality; \
    } \
}

#define bitmask_walk(b, f, data) { \
    unsigned int i; \
    for (i = 0; i < BITMASK_SIZE; ++i) { \
        (f)(i, bitmask_is_set((b), i), (data)); \
    } \
}


#define bitmask_print(fh, b) { \
    unsigned int i; \
    for (i = BITMASK_SIZE; i > 0; --i) { \
        fprintf(fh, "%lu", ((b) >> (i - 1)) & 1); \
    } \
}
    

#endif
