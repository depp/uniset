#include "node.h"
#include "set.h"
#include "category.h"
#include "eaw.h"
#include <stdlib.h>

struct set *
node_eval(struct node *n)
{
    struct set *x, *y;
    switch (n->type) {
    case NODE_RANGE:
        return set_range(n->range.first, n->range.last);
    case NODE_CATEGORY:
        return category_getchars2(n->strlist.length, n->strlist.list);
    case NODE_EAW:
        return eaw_getchars2(n->strlist.length, n->strlist.list);
    case NODE_NOT:
        x = node_eval(n->oper.x);
        return set_complement(x);
    case NODE_UNION:
        x = node_eval(n->oper.x);
        y = node_eval(n->oper.y);
        return set_union(x, y);
    case NODE_DIFFERENCE:
        x = node_eval(n->oper.x);
        y = node_eval(n->oper.y);
        y = set_complement(y);
        return set_intersect(x, y);
    case NODE_INTERSECT:
        x = node_eval(n->oper.x);
        y = node_eval(n->oper.y);
        return set_intersect(x, y);
    default:
        fputs("Unsupported\n", stderr);
        exit(1);
        return 0;
    }
}
