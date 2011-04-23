#include "eaw.h"
#include "set.h"
#include "map.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>

static eaw_t
eaw_decode_fast(unsigned char c1, unsigned char c2)
{
    if (c2 == 0) {
        switch (c1) {
        case 'N': return EAW_N;
        case 'A': return EAW_A;
        case 'H': return EAW_H;
        case 'W': return EAW_W;
        case 'F': return EAW_F;
        }
    } else {
        if (c1 == 'N' && c2 == 'a')
            return EAW_Na;
    }
    return -1;
}

eaw_t
eaw_decode(unsigned char const *name)
{
    return *name ? eaw_decode_fast(name[0], name[1]) : (eaw_t)-1;
}

unsigned int
eaw_makeset(unsigned int n, unsigned char **names)
{
    unsigned int s = 0, i, m;
    eaw_t c;
    for (i = 0; i < n; ++i) {
        c = eaw_decode(names[i]);
        if (c == (eaw_t)-1) {
            fprintf(stderr, "Invalid east Asian width: %s",
                    (char *)names[i]);
            exit(1);
        }
        m = 1U << (unsigned)c;
        if (s & m) {
            fprintf(stderr, "Warning: duplicate east Asian width %s",
                    (char *)names[i]);
        }
        s |= m;
    }
    return s;
}

struct map *
eaw_getmap(void)
{
    struct mapbuilder m;
    struct datafile *fl;
    struct field f[MAX_FIELDS];
    unsigned int r, first, last;
    eaw_t eaw;
    mapbuilder_init(&m, EAW_N);
    fl = datafile_open("EastAsianWidth.txt");
    while ((r = datafile_read(fl, &first, &last, f))) {
        if (f[1].ptr + 1 == f[1].end) {
            eaw = eaw_decode_fast(f[1].ptr[0], '\0');
            if (eaw == (eaw_t)-1)
                goto err;
        } else if (f[1].ptr + 2 == f[1].end &&
                   f[1].ptr[0] == 'N' && f[1].ptr[1] == 'a') {
            eaw = EAW_Na;
        } else
            goto err;
        mapbuilder_insert(&m, first, last, eaw);
    }
    return mapbuilder_finish(&m);
err:
    fputs("Invalid east Asian width in data file: '", stderr);
    fwrite(f[1].ptr, f[1].end - f[1].ptr, 1, stderr);
    fputs("'\n", stderr);
    exit(1);
    return 0;
}

struct set *
eaw_getchars(unsigned int eawset)
{
    struct map *map = eaw_getmap();
    return map_preimage(map, eawset);
}

struct set *
eaw_getchars2(unsigned int n, unsigned char **names)
{
    unsigned int eawset = eaw_makeset(n, names);
    return eaw_getchars(eawset);
}
