#include "node.h"

static void
node_print_plus(FILE *restrict f, struct node *restrict n, int m);

static void
node_print_range(FILE *restrict f, struct node_range *restrict n)
{
    if (n->first == 0 && n->last == 0x10ffff)
        fputs("all", f);
    else if (n->first == n->last)
        fprintf(f, "U+%04X", n->first);
    else
        fprintf(f, "U+%04X..U+%04X", n->first, n->last);
}

static void
node_print_strlist(FILE *restrict f, char const *what,
                   struct node *restrict n)
{
    unsigned int i;
    fputs(what, f);
    for (i = 0; i < n->strlist.length; ++i) {
        if (i)
            putc(',', f);
        fputs((char *)n->strlist.list[i], f);
    }
}

static void
node_print_atom(FILE *restrict f, struct node *restrict n)
{
    switch (n->type) {
    case NODE_RANGE:
        node_print_range(f, &n->range);
        break;
    case NODE_CATEGORY:
        node_print_strlist(f, "cat:", n);
        break;
    case NODE_EAW:
        node_print_strlist(f, "eaw:", n);
        break;
    case NODE_NOT:
    case NODE_UNION:
    case NODE_DIFFERENCE:
    case NODE_INTERSECT:
        putc('(', f);
        node_print_plus(f, n, 1);
        putc(')', f);
        break;
    default:
        fputs("<unknown>", f);
    }
}

static void
node_print_not(FILE *restrict f, struct node *restrict n)
{
    switch (n->type) {
    case NODE_NOT:
        fputs("! ", f);
        node_print_not(f, n->oper.x);
        break;
    default:
        node_print_atom(f, n);
        break;
    }
}

static void
node_print_times(FILE *restrict f, struct node *restrict n)
{
    switch (n->type) {
    case NODE_INTERSECT:
        node_print_times(f, n->oper.x);
        fputs(" * ", f);
        node_print_times(f, n->oper.y);
        break;
    default:
        node_print_not(f, n);
        break;
    }
}

static void
node_print_plus(FILE *restrict f, struct node *restrict n, int m)
{
    switch (n->type) {
    case NODE_UNION:
        node_print_plus(f, n->oper.x, m);
        fputs(" + ", f);
        node_print_plus(f, n->oper.y, 0);
        break;
    case NODE_DIFFERENCE:
        if (!m)
            putc('(', f);
        node_print_plus(f, n->oper.x, 1);
        fputs(" - ", f);
        node_print_times(f, n->oper.y);
        if (!m)
            putc(')', f);
        break;
    default:
        node_print_times(f, n);
        break;
    }
}

void
node_print(FILE *restrict f, struct node *restrict n)
{
    node_print_plus(f, n, 1);
}
