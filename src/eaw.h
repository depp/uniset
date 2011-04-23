struct set;
struct map;

typedef enum {
    EAW_N,
    EAW_A,
    EAW_H,
    EAW_W,
    EAW_F,
    EAW_Na
} eaw_t;

eaw_t
eaw_decode(unsigned char const *name);

unsigned int
eaw_makeset(unsigned int n, unsigned char **names);

struct map *
eaw_getmap(void);

struct set *
eaw_getchars(unsigned int eawset);

struct set *
eaw_getchars2(unsigned int n, unsigned char **names);
