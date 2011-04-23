#include "map.h"
#include "set.h"
#include <stdlib.h>
#include <err.h>
#include <assert.h>

struct map *
map_resize(struct map *m, unsigned int size)
{
    size_t sz = sizeof(struct map) + sizeof(struct maprange) * size;
    struct map *nm = realloc(m, sz);
    if (!nm) err(1, "malloc");
    return nm;
}

void
mapbuilder_init(struct mapbuilder *restrict m, unsigned int defaultv)
{
    m->alloc = 0;
    m->size = 0;
    m->defaultv = defaultv;
    m->cfirst = 0;
    m->clast = -1;
    m->cvalue = -1;
    m->map = 0;
}

void
mapbuilder_destroy(struct mapbuilder *restrict m)
{
    free(m->map);
}

struct map *
mapbuilder_finish(struct mapbuilder *restrict m)
{
    struct map *mm;
    mapbuilder_insert(m, 0x11000, 0, -2);
    mm = m->map;
    m->map = 0;
    mm->length = m->size;
    return mm;
}

struct set *
map_preimage(struct map *restrict m, unsigned int bitset)
{
    struct setbuilder s;
    unsigned int i;
    setbuilder_init(&s);
    for (i = 0; i < m->length; ++i)
        if (bitset & (1U << m->r[i].value))
            setbuilder_insert(&s, m->r[i].first, m->r[i].last);
    return setbuilder_finish(&s);
}
