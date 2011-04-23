#include "category.h"
#include "data.h"
#include "set.h"
#include "map.h"
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

struct map *
category_getmap(void)
{
    struct mapbuilder m;
    struct datafile *fl;
    struct field f[MAX_FIELDS];
    unsigned int r, first, last;
    category_t cat;
    mapbuilder_init(&m, CAT_Cn);
    fl = datafile_open("UnicodeData.txt");
    while ((r = datafile_read(fl, &first, &last, f))) {
        if (f[2].ptr + 2 != f[2].end)
            goto err;
        cat = category_decode_fast(f[2].ptr[0], f[2].ptr[1]);
        if (cat == (category_t)-1)
            goto err;
        mapbuilder_insert(&m, first, last, cat);
    }
    return mapbuilder_finish(&m);
err:
    fputs("Invalid category in data file\n", stderr);
    exit(1);
    return 0;
}

struct set *
category_getchars(unsigned int catset)
{
    struct map *map = category_getmap();
    return map_preimage(map, catset);
}

struct set *
category_getchars2(unsigned int n, unsigned char **names)
{
    unsigned int catset = category_makeset(n, names);
    return category_getchars(catset);
}
