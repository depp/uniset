#include <stdio.h>

struct range {
    unsigned int first, last;
};

struct set {
    unsigned int length;
    struct range r[];
};

struct set *
set_create(unsigned int length);

struct set *
set_range(unsigned int first, unsigned int last);

/* Compute the union.  Both arguments are freed.  */
struct set *
set_union(struct set *x, struct set *y);

/* Compute the intersection.  Both arguments are freed.  */
struct set *
set_intersect(struct set *x, struct set *y);

/* Compute the complement.  Argument is freed.  */
struct set *
set_complement(struct set *x);

void
set_print(FILE *f, struct set *x);

void
set_print16(FILE *f, struct set *x);
