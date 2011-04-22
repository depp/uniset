#include "category.h"
#include "data.h"
#include "set.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

category_t
category_decode_fast(unsigned char c1, unsigned char c2)
{
    switch (c1) {
    case 'L':
        switch (c2) {
        case 'u': return CAT_Lu;
        case 'l': return CAT_Ll;
        case 't': return CAT_Lt;
        case 'm': return CAT_Lm;
        case 'o': return CAT_Lo;
        }
        break;
    case 'M':
        switch (c2) {
        case 'n': return CAT_Mn;
        case 'c': return CAT_Mc;
        case 'e': return CAT_Me;
        }
        break;
    case 'N':
        switch (c2) {
        case 'd': return CAT_Nd;
        case 'l': return CAT_Nl;
        case 'o': return CAT_No;
        }
        break;
    case 'P':
        switch (c2) {
        case 'c': return CAT_Pc;
        case 'd': return CAT_Pd;
        case 's': return CAT_Ps;
        case 'e': return CAT_Pe;
        case 'i': return CAT_Pi;
        case 'f': return CAT_Pf;
        case 'o': return CAT_Po;
        }
        break;
    case 'S':
        switch (c2) {
        case 'm': return CAT_Sm;
        case 'c': return CAT_Sc;
        case 'k': return CAT_Sk;
        case 'o': return CAT_So;
        }
        break;
    case 'Z':
        switch (c2) {
        case 's': return CAT_Zs;
        case 'l': return CAT_Zl;
        case 'p': return CAT_Zp;
        }
        break;
    case 'C':
        switch (c2) {
        case 'c': return CAT_Cc;
        case 'f': return CAT_Cf;
        case 's': return CAT_Cs;
        case 'o': return CAT_Co;
        case 'n': return CAT_Cn;
        }
        break;
    default:
        break;
    }
    return -1;
}

category_t
category_decode(unsigned char const *name)
{
    if (!name[0] || !name[1] || name[2])
        return -1;
    return category_decode_fast(name[0], name[1]);
}

unsigned int
category_makeset(unsigned int n, unsigned char **names)
{
    unsigned int s = 0, i, m;
    category_t c;
    for (i = 0; i < n; ++i) {
        c = category_decode(names[i]);
        if (c == (category_t)-1) {
            fprintf(stderr, "Invalid category: %s", (char *)names[i]);
            exit(1);
        }
        m = 1U << (unsigned)c;
        if (s & m) {
            fprintf(stderr, "Warning: duplicate category %s",
                    (char *)names[i]);
        }
        s |= m;
    }
    return s;
}

struct category_enumerator {
    struct datafile *fl;
    unsigned int lastc;
    category_t cat;
    unsigned int first, last;
};

static void
category_enumerator_init(struct category_enumerator *restrict e)
{
    e->fl = datafile_open("UnicodeData.txt");
    e->lastc = -1;
    e->cat = -1;
    e->first = 0;
    e->last = 0;
}

static void
category_enumerator_destroy(struct category_enumerator *restrict e)
{
    if (e->fl)
        datafile_close(e->fl);
}

static int
category_enumerator_next(struct category_enumerator *restrict e,
                         unsigned int *first, unsigned int *last,
                         category_t *category)
{
    unsigned int fr, la, r;
    category_t c;
    struct field f[MAX_FIELDS];
    if (e->cat != (category_t)-1) {
        *category = e->cat;
        *first = e->first;
        *last = e->last;
        e->cat = -1;
        return 1;
    }
    if (!e->fl)
        return 0;
    r = datafile_read(e->fl, &fr, &la, f);
    if (!r) {
        datafile_close(e->fl);
        e->fl = 0;
        if (e->lastc < 0x10ffff) {
            *first = e->lastc + 1;
            *last = 0x10ffff;
            *category = CAT_Cn;
            return 1;
        }
        return 0;
    }
    if (f[2].ptr + 2 != f[2].end)
        goto err;
    c = category_decode_fast(f[2].ptr[0], f[2].ptr[1]);
    if (c == (category_t)-1)
        goto err;
    if (fr != e->lastc + 1) {
        if (fr < e->lastc + 1)
            goto err;
        e->cat = c;
        e->first = fr;
        e->last = la;
        *category = CAT_Cn;
        *first = e->lastc + 1;
        *last = fr - 1;
    } else {
        *category = c;
        *first = fr;
        *last = la;
    }
    e->lastc = la;
    return 1;
err:
    fputs("Invalid category in data file\n", stderr);
    exit(1);
    return 0;
}

struct set *
category_getchars(unsigned int catset)
{
    unsigned int alloc = 32, count = 0;
    struct set *restrict s = set_create(32), *s2;
    unsigned int cfirst, clast, afirst = -1, alast = -1, r;
    category_t ccat;
    struct category_enumerator e;
    category_enumerator_init(&e);
    do {
        r = category_enumerator_next(&e, &cfirst, &clast, &ccat);
        if (r && catset & (1U << (unsigned)ccat)) {
            if (afirst == (unsigned int)-1)
                afirst = cfirst;
            alast = clast;
        } else {
            if (afirst != (unsigned int)-1) {
                if (count >= alloc) {
                    s2 = set_create(alloc * 2);
                    alloc = alloc * 2;
                    memcpy(s2->r, s->r, count * sizeof(*s->r));
                    free(s);
                    s = s2;
                }
                s->r[count].first = afirst;
                s->r[count].last = alast;
                count += 1;
                afirst = -1;
            }
        }
    } while (r);
    category_enumerator_destroy(&e);
    s->length = count;
    return s;
}


struct set *
category_getchars2(unsigned int n, unsigned char **names)
{
    unsigned int catset = category_makeset(n, names);
    return category_getchars(catset);
}
