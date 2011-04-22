#include "set.h"
#include <stdlib.h>
#include <err.h>
#include <assert.h>

struct set *
set_create(unsigned int length)
{
    struct set *s = malloc(sizeof(struct set) +
                           sizeof(struct range) * length);
    if (!s)
        err(1, "malloc");
    return s;
}

struct set *
set_range(unsigned int first, unsigned int last)
{
    struct set *s = set_create(1);
    assert(first <= last);
    assert(last <= 0x10ffff);
    s->r[0].first = first;
    s->r[0].last = last;
    s->length = 1;
    return s;
}

struct set *
set_union(struct set *x, struct set *y)
{
    struct set *z;
    struct range *restrict xp = x->r, *xe = x->r + x->length;
    struct range *restrict yp = y->r, *ye = y->r + y->length;
    struct range *tp, *te, *restrict zp;
    unsigned int afirst, alast, bfirst, blast;
    if (x->length == 0) {
        free(x);
        return y;
    } else if (y->length == 0) {
        free(y);
        return x;
    }
    z = set_create(x->length + y->length);
    zp = z->r;
    while (1) {
        if (xp->first < yp->first) {
            tp = xp; te = xe;
            xp = yp; xe = ye;
            yp = tp; ye = te;
        }
        afirst = yp->first;
        alast = yp->last;
        ++yp;
        while (1) {
            bfirst = xp->first;
            blast = xp->last;
            if (bfirst > alast + 1)
                break;
            ++xp;
            if (blast > alast) {
                alast = blast;
                tp = xp; te = xe;
                xp = yp; xe = ye;
                yp = tp; ye = te;
            }
            if (xp == xe) {
                zp->first = afirst;
                zp->last = alast;
                ++zp;
                while (yp != ye)
                    *zp++ = *yp++;
                goto done;
            }
        }
        zp->first = afirst;
        zp->last = alast;
        ++zp;
        if (yp == ye) {
            while (xp != xe)
                *zp++ =  *xp++;
            goto done;
        }
    }
done:
    z->length = zp - z->r;
    free(x);
    free(y);
    return z;
}

struct set *
set_intersect(struct set *x, struct set *y)
{
    struct set *z;
    struct range *restrict xp = x->r, *xe = x->r + x->length;
    struct range *restrict yp = y->r, *ye = y->r + y->length;
    struct range *tp, *te, *restrict zp;
    unsigned int limit;
    if (x->length == 0) {
        free(y);
        return x;
    } else if (y->length == 0) {
        free(x);
        return y;
    }
    z = set_create(x->length + y->length);
    zp = z->r;
    while (xp != xe && yp != ye) {
        if (xp->first > yp->first) {
            tp = xp; te = xe;
            xp = yp; xe = ye;
            yp = tp; ye = te;
        }
        limit = xp->last;
        ++xp;
        while (yp != ye && yp->first <= limit) {
            zp->first = yp->first;
            if (yp->last <= limit) {
                zp->last = yp->last;
                ++zp;
                ++yp;
            } else {
                zp->last = limit;
                ++zp;
                limit = yp->last;
                ++yp;
                tp = xp; te = xe;
                xp = yp; xe = ye;
                yp = tp; ye = te;
            }
        }
    }
    z->length = zp - z->r;
    free(x);
    free(y);
    return z;
}

struct set *
set_complement(struct set *x)
{
    struct set *z = set_create(x->length + 1);
    struct range *restrict xp = x->r, *xe = x->r + x->length;
    struct range *restrict zp = z->r;
    if (xp == xe) {
        zp->first = 0;
        zp->last = 0x10ffff;
        ++zp;
    } else {
        if (xp->first != 0) {
            zp->first = 0;
            zp->last = xp->first - 1;
            ++zp;
        }
        zp->first = xp->last + 1;
        ++xp;
        while (xp != xe) {
            zp->last = xp->first - 1;
            ++zp;
            zp->first = xp->last + 1;
            ++xp;
        }
        if (zp->first <= 0x10ffff) {
            zp->last = 0x10ffff;
            ++zp;
        }
    }
    z->length = zp - z->r;
    free(x);
    return z;
}
