#include "node.h"
#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <err.h>

enum {
    TOKEN_PLUS = TOKEN_MAX,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_NOT
};

static struct node *
node_new(void)
{
    struct node *n = malloc(sizeof(*n));
    if (!n)
        err(1, "malloc");
    return n;
}

static int
node_tokcmp(struct token *restrict t, char const *s)
{
    size_t l = strlen(s);
    return t->datap + l == t->datae && !memcmp(t->datap, s, l);
}

int
node_tokpfx(struct token *restrict t, char const *s)
{
    size_t l = strlen(s);
    return (size_t)(t->datae - t->datap) >= l && !memcmp(t->datap, s, l);
}

static void
node_read_token(struct tokenizer *restrict t)
{
    tokenizer_lex(t);
    if (t->tok.type == TOKEN_ATOM) {
        if (t->tok.datap + 1 == t->tok.datae) {
            switch (*t->tok.datap) {
            case '+': t->tok.type = TOKEN_PLUS; break;
            case '-': t->tok.type = TOKEN_MINUS; break;
            case '*': t->tok.type = TOKEN_TIMES; break;
            case '!': t->tok.type = TOKEN_NOT; break;
            default: break;
            }
        }
    }
}

static struct node *
node_read_plus(struct tokenizer *restrict t);

static void
node_read_strlist(struct node_strlist *restrict n,
                  unsigned char *start, unsigned char *end)
{
    unsigned int alloc = 0, nalloc;
    unsigned char *p = start, *s, **nlist, *str;
    size_t len;
    n->length = 0;
    n->list = 0;
    while (p != end) {
        s = p;
        for (; p != end && *p != ','; ++p);
        if (s != p) {
            if (n->length >= alloc) {
                nalloc = alloc ? alloc * 2 : 4;
                nlist = realloc(n->list, sizeof(*nlist) * nalloc);
                if (!nlist)
                    err(1, "malloc");
                n->list = nlist;
                alloc = nalloc;
            }
            len = p - s;
            str = malloc(len + 1);
            if (!str)
                err(1, "malloc");
            memcpy(str, s, len);
            str[len] = '\0';
            n->list[n->length++] = str;
        }
        if (p != end)
            p++;
    }
}

static void
node_read_all(struct node_range *restrict n)
{
    n->first = 0;
    n->last = 0x10ffff;
}

static unsigned int
node_read_char(unsigned char *start, unsigned char *end)
{
    // Longest is 8 chars -- U+10FFFF
    size_t len = end - start;
    unsigned char *p = start;
    unsigned int c, d;
    if (len > 8 || !len)
        goto err;
    if (len >= 2 && (p[0] == 'U' || p[0] == 'u') && p[1] == '+') {
        if (len == 2)
            goto err;
        p += 2;
        len -= 2;
    }
    c = 0;
    for (; p != end; ++p) {
        if (*p >= '0' && *p <= '9')
            d = *p - '0';
        else if (*p >= 'a' && *p <= 'f')
            d = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F')
            d = *p - 'A' + 10;
        else
            goto err;
        c = (c << 4) | d;
    }
    if (c >= 0x10ffff)
        goto err;
    return c;
err:
    fputs("Invalid character reference\n", stderr);
    exit(1);
}

static void
node_read_range(struct node_range *restrict n,
                unsigned char *start, unsigned char *end)
{
    unsigned char *p;
    for (p = start; p != end && *p != '.'; ++p);
    if (p == end) {
        n->last = n->first = node_read_char(start, end);
    } else {
        n->first = node_read_char(start, p);
        p++;
        if (p == end || *p != '.')
            goto err;
        p++;
        n->last = node_read_char(p, end);
        if (n->last < n->first)
            goto err;
    }
    return;
err:
    fputs("Invalid character range\n", stderr);
    exit(1);
}

static struct node *
node_read_atom(struct tokenizer *restrict t)
{
    struct node *n;
    char *type;
    unsigned char *p;
    switch (t->tok.type) {
    case TOKEN_PAROPEN:
        node_read_token(t);
        n = node_read_plus(t);
        if (t->tok.type != TOKEN_PARCLOSE) {
            fputs("Missing )\n", stderr);
            exit(1);
        }
        node_read_token(t);
        return n;
    case TOKEN_ATOM:
        n = node_new();
        for (p = t->tok.datap; p != t->tok.datae && *p != ':'; ++p);
        if (p != t->tok.datae) {
            *p = '\0';
            p++;
            type = (char *)t->tok.datap;
            if (!strcmp(type, "cat"))
                n->type = NODE_CATEGORY;
            else if (!strcmp(type, "eaw"))
                n->type = NODE_EAW;
            else {
                fprintf(stderr, "Unknown type: %s\n", type);
                exit(1);
            }
            node_read_strlist(&n->strlist, p, t->tok.datae);
        } else {
            if (node_tokcmp(&t->tok, "all"))
                node_read_all(&n->range);
            else
                node_read_range(&n->range, t->tok.datap, t->tok.datae);
        }
        node_read_token(t);
        return n;
    default:
        fputs("Invalid syntax\n", stderr);
        exit(1);
        return 0;
    }
}

static struct node *
node_read_not(struct tokenizer *restrict t)
{
    struct node *x, *n;
    switch (t->tok.type) {
    case TOKEN_NOT:
        node_read_token(t);
        x = node_read_not(t);
        n = node_new();
        n->type = NODE_NOT;
        n->oper.x = x;
        return n;
    default:
        return node_read_atom(t);
    }
}

static struct node *
node_read_times(struct tokenizer *restrict t)
{
    struct node *x, *y, *n;
    nodetype_t nt;
    n = node_read_not(t);
    while (1) {
        switch (t->tok.type) {
        case TOKEN_TIMES:
            nt = NODE_INTERSECT;
            break;
        default:
            return n;
        }
        node_read_token(t);
        x = n;
        y = node_read_not(t);
        n = node_new();
        n->type = nt;
        n->oper.x = x;
        n->oper.y = y;
    }
}

static struct node *
node_read_plus(struct tokenizer *restrict t)
{
    struct node *x, *y, *n;
    nodetype_t nt;
    n = node_read_times(t);
    while (1) {
        switch (t->tok.type) {
        case TOKEN_PLUS:
            nt = NODE_UNION;
            break;
        case TOKEN_MINUS:
            nt = NODE_DIFFERENCE;
            break;
        default:
            return n;
        }
        node_read_token(t);
        x = n;
        y = node_read_times(t);
        n = node_new();
        n->type = nt;
        n->oper.x = x;
        n->oper.y = y;
    }
}

struct node *
node_read(struct tokenizer *restrict t)
{
    struct node *n;
    node_read_token(t);
    n = node_read_plus(t);
    if (t->tok.type != TOKEN_EOF) {
        fputs("Expected EOF\n", stderr);
        exit(1);
    }
    return n;
}
