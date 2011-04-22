#include "token.h"
#include <err.h>
#include <stdlib.h>
#include <stdio.h>

void
tokenizer_init(struct tokenizer *restrict t)
{
    t->queue = 0;
    t->qpos = 0;
    t->qend = 0;
    t->qalloc = 0;
    t->ptr = 0;
}

void
tokenizer_destroy(struct tokenizer *restrict t)
{
    free(t->queue);
}

void
tokenizer_addtext(struct tokenizer *restrict t, char *text[])
{
    unsigned int i, nalloc;
    unsigned char **nqueue;
    for (i = 0; text[i]; ++i) {
        if (t->qend == t->qalloc) {
            nalloc = t->qalloc ? t->qalloc * 2 : 4;
            nqueue = realloc(t->queue, sizeof(*nqueue) * nalloc);
            if (!nqueue)
                err(1, "malloc");
            t->queue = nqueue;
            t->qalloc = nalloc;
        }
        t->queue[t->qend++] = (unsigned char *)text[i];
    }
}

static int
is_white(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int
is_atom(unsigned char c)
{
    return c >= 0x21 && c <= 0x7e && c != '(' && c != ')';
}

void
tokenizer_lex(struct tokenizer *restrict t)
{
    unsigned char *restrict p = t->ptr;
    if (!p)
        goto next_chunk;
scan_token:
    while (is_white(*p))
        p++;
    switch (*p) {
    case '\0':
    next_chunk:
        if (t->qpos < t->qend) {
            p = t->queue[t->qpos++];
            goto scan_token;
        }
        t->tok.type = TOKEN_EOF;
        p = 0;
        break;
    case '(':
        t->tok.type = TOKEN_PAROPEN;
        p++;
        break;
    case ')':
        t->tok.type = TOKEN_PARCLOSE;
        p++;
        break;
    default:
        if (is_atom(*p)) {
            t->tok.datap = p;
            do p++; while (is_atom(*p));
            t->tok.datae = p;
            t->tok.type = TOKEN_ATOM;
            t->ptr = p;
            return;
        } else {
            fprintf(stderr, "Invalid character: 0x%02x\n", *p);
            exit(1);
        }
        break;
    }
    t->tok.datap = 0;
    t->tok.datae = 0;
    t->ptr = p;
}
