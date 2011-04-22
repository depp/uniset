#include <stdio.h>

typedef enum {
    NODE_RANGE,
    NODE_CATEGORY,
    NODE_EAW,

    NODE_NOT,
    NODE_UNION,
    NODE_DIFFERENCE,
    NODE_INTERSECT
} nodetype_t;

struct node_range {
    unsigned int first;
    unsigned int last;
};

struct node_strlist {
    unsigned int length;
    unsigned char **list;
};

struct node_oper {
    struct node *x;
    struct node *y;
};

struct node {
    nodetype_t type;
    union {
        struct node_range range;
        struct node_strlist strlist;
        struct node_oper oper;
    };
};

struct tokenizer;

struct node *
node_read(struct tokenizer *restrict tt);

void
node_print(FILE *restrict f, struct node *restrict n);

struct set;

struct set *
node_eval(struct node *n);
