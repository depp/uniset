struct set;

struct maprange {
    unsigned int first, last;
    unsigned int value;
};

struct map {
    unsigned int length;
    struct maprange r[];
};

struct mapbuilder {
    unsigned int alloc;
    unsigned int size;
    unsigned int defaultv;
    unsigned int cfirst, clast, cvalue;
    struct map *map;
};

struct map *
map_resize(struct map *m, unsigned int size);

void
mapbuilder_init(struct mapbuilder *restrict m, unsigned int defaultv);

void
mapbuilder_destroy(struct mapbuilder *restrict m);

struct map *
mapbuilder_finish(struct mapbuilder *restrict m);

static inline void
mapbuilder_insert(struct mapbuilder *restrict m,
                  unsigned int first, unsigned int last,
                  unsigned int value)
{
    if (value == m->cvalue &&
        (m->clast == first - 1 || value == m->defaultv)) {
        m->clast = last;
    } else {
        if (m->cvalue != (unsigned int)-1 || m->clast != first - 1) {
            if (m->cvalue == m->defaultv)
                m->clast = first - 1;
            struct maprange *restrict r;
            if (m->size + 2 > m->alloc) {
                unsigned int nalloc = m->alloc ? m->alloc * 2 : 16;
                m->map = map_resize(m->map, nalloc);
                m->alloc = nalloc;
            }
            r = m->map->r + m->size;
            if (m->cvalue != (unsigned int)-1) {
                r->first = m->cfirst;
                r->last = m->clast;
                r->value = m->cvalue;
                r++;
                m->size++;
            }
            if (m->clast != first - 1) {
                r->first = m->clast + 1;
                r->last = first - 1;
                r->value = m->defaultv;
                r++;
                m->size++;
            }
        }
        m->cfirst = first;
        m->clast = last;
        m->cvalue = value;
    }
}

struct set *
map_preimage(struct map *restrict m, unsigned int bitset);
