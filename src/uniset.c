#include "node.h"
#include "token.h"
#include "set.h"
#include <string.h>

static char const HELP[] =
"uniset: generate sets of Unicode characters.\n"
"Usage: uniset [OPTS..] [--] [SET]\n"
"\n"
"Options:\n"
"  --help         : show this help screen\n"
"  --16           : use planar output format\n"
"\n"
"Output format:\n"
"  Sorted list of non-overlapping ranges of characters in the set.\n"
"  Each range is of the format 'CHAR' or 'CHAR..CHAR'.  Characters\n"
"  are given in hexadecimal.  Each range appears on a separate line.\n"
"\n"
"Set operations:\n"
"  [SET] + [SET]  : union\n"
"  [SET] - [SET]  : difference\n"
"  [SET] * [SET]  : intersection\n"
"  ! [SET]        : complement\n"
"  ( [SET] )      : parentheses\n"
"\n"
"Basic sets:\n"
"  U+XXXX             : Individual character\n"
"  U+XXXX..U+XXXX     : Character range\n"
"  all                : All characters, U+0000..U+10FFFF\n"
"  cat:CAT1,CAT2,...  : General category (Lu,Ll,...)\n"
"  eaw:W1,W2,...      : East Asian width (F,H,W,Na,A,N)\n"
;

int main(int argc, char *argv[])
{
    struct tokenizer t;
    struct node *n;
    struct set *s;
    int i;
    char *opt;
    int format = 0, verbose = 0;

    for (i = 1; i < argc; ++i) {
        if (memcmp(argv[i], "--", 2))
            break;
        opt = argv[i] + 2;
        if (!strcmp(opt, "16")) {
            format = 1;
        } else if (!strcmp(opt, "help")) {
            fputs(HELP, stderr);
            return 1;
        } else if (!strcmp(opt, "verbose")) {
            verbose = 1;
        } else {
            fprintf(stderr, "Invalid option: '%s'\n", argv[i]);
            return 1;
        }
    }
    if (i == argc) {
        fputs(HELP, stderr);
        return 1;
    }

    tokenizer_init(&t);
    tokenizer_addtext(&t, argv + i);
    n = node_read(&t);
    tokenizer_destroy(&t);

    if (verbose) {
        fputs("Expression:", stderr);
        node_print(stderr, n);
        fputs("\n\n", stderr);
    }

    s = node_eval(n);
    if (!s->length)
        fputs("Warning: Set is empty\n", stderr);
    switch (format) {
    case 0:
        set_print(stdout, s);
        break;
    case 1:
        set_print16(stdout, s);
        break;
    }

    return 0;
}
