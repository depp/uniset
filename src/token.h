/* The tokenizer is very simple.

   A parenthesis ( ) is a token.

   All other tokens are "atoms", which are simply contiguous ranges of
   non-space non-parenthesis characters.  Non-ASCII characters are not
   allowed.  Errors will cause the tokenizer to abort the program.  */

enum {
    TOKEN_EOF,
    TOKEN_ATOM,
    TOKEN_PAROPEN,
    TOKEN_PARCLOSE,

    TOKEN_MAX
};

struct token {
    unsigned int type;
    unsigned char *datap;
    unsigned char *datae;
};

struct tokenizer {
    unsigned char **queue;
    unsigned int qpos, qend, qalloc;
    unsigned char *ptr;
    struct token tok;
};

void
tokenizer_init(struct tokenizer *restrict tt);

void
tokenizer_destroy(struct tokenizer *restrict tt);

void
tokenizer_addtext(struct tokenizer *restrict tt, char *text[]);

void
tokenizer_lex(struct tokenizer *restrict tt);
