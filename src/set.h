#include <stdio.h>

struct range {
    unsigned int first, last;
};

struct set {
    unsigned int length;
    struct range r[];
};

struct setbuilder {
    unsigned int alloc;
    unsigned int size;
    unsigned int cfirst, clast;
    struct set *set;
};

struct set *
set_create(unsigned int length);

struct set *
set_resize(struct set *s, unsigned int size);

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

void
setbuilder_init(struct setbuilder *restrict s);

void
setbuilder_destroy(struct setbuilder *restrict s);

struct set *
setbuilder_finish(struct setbuilder *restrict s);

static inline void
setbuilder_insert(struct setbuilder *restrict s,
                  unsigned int first, unsigned int last)
{
    if (s->clast == first - 1) {
        s->clast = last;
    } else {
        if (s->clast != (unsigned int)-2) {
            if (s->size + 1 > s->alloc) {
                unsigned int nalloc = s->alloc ? s->alloc * 2 : 16;
                s->set = set_resize(s->set, nalloc);
                s->alloc = nalloc;
            }
            s->set->r[s->size].first = s->cfirst;
            s->set->r[s->size].last = s->clast;
            s->size++;
        }
        s->cfirst = first;
        s->clast = last;
    }
}
