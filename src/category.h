typedef enum {
    /* Letter */
    CAT_Lu,
    CAT_Ll,
    CAT_Lt,
    CAT_Lm,
    CAT_Lo,
    /* Mark */
    CAT_Mn,
    CAT_Mc,
    CAT_Me,
    /* Number */
    CAT_Nd,
    CAT_Nl,
    CAT_No,
    /* Punctuation */
    CAT_Pc,
    CAT_Pd,
    CAT_Ps,
    CAT_Pe,
    CAT_Pi,
    CAT_Pf,
    CAT_Po,
    /* Symbol */
    CAT_Sm,
    CAT_Sc,
    CAT_Sk,
    CAT_So,
    /* Separator */
    CAT_Zs,
    CAT_Zl,
    CAT_Zp,
    /* Other */
    CAT_Cc,
    CAT_Cf,
    CAT_Cs,
    CAT_Co,
    CAT_Cn
} category_t;

category_t
category_decode_fast(unsigned char c1, unsigned char c2);

category_t
category_decode(unsigned char const *name);

unsigned int
category_makeset(unsigned int n, unsigned char **names);

struct set *
category_getchars(unsigned int catset);

struct set *
category_getchars2(unsigned int n, unsigned char **names);
