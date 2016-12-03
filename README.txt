Uniset: Compute sets of unicode code points
===========================================

Uniset is a simple command-line tool for computing sets of Unicode
code points.  Its main goal is to support the development of fast
Unicode-aware parsers.

Warning!
--------

This software is currently "early-beta".  It may crash or generate
incorrect results.  It has very few features.  It was written in a
single day and only tested manually.

Usage
-----

uniset [OPT..] [EXPR]

Prints the set of Unicode characters specified by EXPR.  The
UNICODE_DIR environment variable must point to a directory containing
Unicode data tables.

In examples below, set expressions are quoted or left unquoted
somewhat arbitrarily.  In most situations, the quoting is irrelevant.
However, remember that '*' is a special character in the shell.
Single quote marks <'> are preferred over double quote marks <">,
because a single quote marks <'> disable shell expansion.

These three invocations are equivalent:

    uniset cat:Zs + FEFF - 0..7F	# ok
    uniset "cat:Zs + FEFF - 0..7F"      # ok
    uniset 'cat:Zs + FEFF - 0..7F'      # ok

However, these three invocations are not:

    uniset cat:Ll,Lu * 0..FF		# not ok
    uniset "cat:Ll,Lu * 0..FF"		# not preferred
    uniset 'cat:Ll,Lu * 0..FF'  	# ok

Set operations
--------------

Sets may be combined using simple set operations.  Note that the
operators (+, -, *, !) must be separated from other tokens (besides
parenthesis) by at least one space.

    SET ::= SET + SET   (union)
          | SET - SET   (difference)
          | SET * SET   (intersection)
          | ! SET       (complement)
          | ( SET )     (grouping)

Operator precedence and parentheses work as in ordinary algebra, and
the complement (!) operator has the highest precedence.  For example,

    ! a + b * (c + d) + ! e * f

is the same as

    (! a) + (b * (c + d)) + ((! e) * f)

The --verbose flag will cause uniset to print the expression to the
standard error stream.

Note that unlike ordinary algebra, the following are not equivalent:

    a + b - c    vs.    a - c + b

Basic sets
----------

Individual characters and ranges of characters can be specified in
hexadecimal.  Hexadecimal was chosen because Unicode characters are
organized naturally in hexadecimal and because the Unicode
specification refers to code points using hexadecimal.  Decimal is not
supported.

Examples:

    Line feed:  a, 0a, or 000A
    All ASCII:  0..7F

General category:

    cat:CAT1,CAT2,...

East asian width:

    eaw:W1,W2,...

ECMAScript example
------------------

The 5th edition of ECMAScript specifies that source files are
Unicode.  Identifiers may start with letters, '$', '_', and escape
sequences.  The set of Unicode letters, according to the ECMAScript
standard, is given by the following command:

    uniset 'cat:Lu,Ll,Lt,Lm,Lo,Nl'

The remainder of an identifier may contain the same characters, as
well as combining marks (categories Mn, Mc), digits (category Nd),
connector punctuation (category Pc), and the characters ZWNJ (U+200C)
and ZWJ (U+200D).  The additional characters are given by the command:

    uniset 'cat:Mn,Mc,Nd,Pc + 200C + 200D'

Whitespace in ECMAScript consists of the Unicode category Zs, as well
as the byte-order mark U+FEFF.

    uniset 'cat:Zs + FEFF'

Suppose that your ECMAScript parser uses a table or switch statement
to handle ASCII characters.  For efficiency, you can omit all ASCII
characters from a set by subtracting them at the end.  For example,

    uniset 'cat:Lu,Ll,Lt,Lm,Lo,Nl - 0..7f'

Output formats
--------------

By default, uniset outputs a sorted list of non-overlapping ranges of
characters in the set, in hexadecimal.  For example,

    $ uniset cat:Zs
    20
    a0
    1680
    180e
    2000..200a
    202f
    205f
    3000

The '--16' option specifies a C-style array of pairs of 16-bit
unsigned integers.  The first 17 entries correspond to the 17 Unicode
planes, and each entry specifies a pair of offsets into the remainder
of the table.

    $ uniset --16 cat:Zs
    { /* plane */ 0, 8 },
    { /* plane */ 0, 0 },
    <15 repeated entries removed>
    { 32, 32 },
    { 160, 160 },
    { 5760, 5760 },
    { 6158, 6158 },
    { 8192, 8202 },
    { 8239, 8239 },
    { 8287, 8287 },
    { 12288, 12288 }

Note that the category 'Zs' only contains characters in the first
plane, so the other 16 planes have zero entries.  The entry for plane
0, { 0, 8 }, indicates that the first entry is at 17 + 0 and the entry
on past the end is at 17 + 8.

But if you don't like reading English, here is the C code to test if a
character is a member of a set:

    bool uniset_test(uint16_t const set[][2], uint32_t c)
    {
        unsigned int p = c >> 16;
        if (p > 16)
            return false;
        unsigned int l = set[p][0] + 17, r = set[p][1] + 17;
	c &= 0xffff;
        while (l < r) {
            unsigned int m = (l + r) / 2;
            if (c < set[m][0])
                r = m;
            else if (c > set[m][1])
                l = m + 1;
            else
                return true;
        }
        return false;
    }

The '--32' option specifies a C-style array of 32-bit unsigned
integers.  Each entry is a range of characters.

    $ uniset --32 cat:Zs
    { /* plane */ 0, 7 },
    { /* plane */ 0, 0 },
    <15 repeated entries removed>
    { 0x0020, 0x0020 },
    { 0x00A0, 0x00A0 },
    { 0x1680, 0x1680 },
    { 0x2000, 0x200A },
    { 0x202F, 0x202F },
    { 0x205F, 0x205F },
    { 0x3000, 0x3000 }

Here is the C code for checking membership, where 'n' is the array
size:

    bool uniset_test(uint32_t n, uint32_t const set[][2], uint32_t c)
    {
        unsigned int l = 0, r = n;
        while (l < r) {
            unsigned int m = (l + r) / 2;
            if (c < set[m][0])
                r = m;
            else if (c > set[m][1])
                l = m + 1;
            else
                return true;
        }
        return false;
    }

The typical way to use the '--16' or '--32' options is as an include
file.  For example,

    const uint16_t UNICODE_LETTER[][2] = {
    #include "unicode_letter.def"
    };
