#include "data.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

struct datafile {
    void *p;
    size_t len;
    char *name;
    unsigned char *ptr, *end;
    int is_unicode_data;
};

struct datafile *
datafile_open(char const *restrict name)
{
    static char *data_dir = 0;
    char path[PATH_MAX];
    struct datafile *restrict f;
    struct stat s;
    int fd, r;
    size_t sz;
    void *p;
    if (!data_dir) {
        data_dir = getenv("UNICODE_DIR");
        if (!data_dir) {
            fputs("No UNICODE_DIR environment variable set.\n", stderr);
            exit(1);
        }
    }
    r = snprintf(path, sizeof(path), "%s/%s", data_dir, name);
    if (r < 0 || (size_t)r > sizeof(path)) {
        fputs("Bad things happened.  Sorry.\n", stderr);
        exit(1);
    }
    fd = open(path, O_RDONLY);
    if (fd < 0)
        err(1, "open: %s", name);
    r = fstat(fd, &s);
    if (r < 0)
        err(1, "stat: %s", name);
    if (!S_ISREG(s.st_mode))
        errx(1, "not a regular file: %s", name);
    if ((uint64_t)s.st_size > SIZE_MAX)
        errx(1, "file too large: %s", name);
    sz = s.st_size;
    p = mmap(0, sz, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0);
    if (p == (void *)-1)
        errx(1, "mmap: %s", name);
    close(fd);
    f = malloc(sizeof(*f));
    if (!f)
        errx(1, "malloc");
    f->p = p;
    f->len = sz;
    f->name = strdup(name);
    if (!f->name)
        errx(1, "malloc");
    f->ptr = p;
    f->end = f->ptr + sz;
    f->is_unicode_data = !strcmp(name, "UnicodeData.txt");
    return f;
}

void
datafile_close(struct datafile *restrict f)
{
    munmap(f->p, f->len);
    free(f->name);
    free(f);
}

static unsigned int
datafile_read_line(unsigned char *ptr, unsigned char *end,
                   struct field fields[restrict MAX_FIELDS])
{
    unsigned char *p = ptr, *fs, *fe;
    unsigned int nf = 0, i;
    while (p != end && *p != '#')
        p++;
    end = p;
    p = ptr;
    while (p != end) {
        while (p != end && *p == ' ')
            p++;
        fs = p;
        while (p != end && *p != ';')
            p++;
        fe = p;
        if (p != end)
            p++;
        while (fe != fs && fe[-1] == ' ')
            fe--;
        if (nf == MAX_FIELDS) {
            fputs("Too many fields in one line\n", stderr);
            exit(1);
        }
        fields[nf].ptr = fs;
        fields[nf].end = fe;
        nf += 1;
    }
    if (fields[0].ptr == fields[0].end)
        nf = 0;
    for (i = nf; i < MAX_FIELDS; ++i) {
        fields[i].ptr = 0;
        fields[i].end = 0;
    }
    return nf;
}

static unsigned int
datafile_read_raw(struct datafile *restrict f,
                  struct field fields[restrict MAX_FIELDS])
{
    unsigned char *ptr = f->ptr, *end = f->end;
    unsigned char *ls, *le;
    unsigned int r;
    do {
        ls = ptr;
        while (ptr != end && *ptr != '\n')
            ptr++;
        le = ptr;
        if (ptr != end)
            ptr++;
        r = datafile_read_line(ls, le, fields);
    } while (r <= 1 && ptr != end);
    f->ptr = ptr;
    return r;
}

static unsigned int
hex_digit(unsigned int x)
{
    if (x >= '0' && x <= '9')
        return x - '0';
    if (x >= 'A' && x <= 'F')
        return x - 'A' + 10;
    if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
    return 16;
}

static void
datafile_decode_range(unsigned int *first, unsigned int *last,
                      unsigned char *start, unsigned char *end)
{
    unsigned char *ptr = start, *p2;
    unsigned int v1, v2, d;
    v1 = 0;
    while (ptr != end) {
        d = hex_digit(*ptr);
        if (d == 16)
            break;
        v1 = (v1 << 4) | d;
        ptr++;
    }
    if (ptr == start || v1 > 0x10ffff)
        goto err;
    if (ptr == end) {
        *first = v1;
        *last = v1;
        return;
    }
    if (*ptr != '.')
        goto err;
    ptr++;
    if (ptr == end && *ptr == '.')
        goto err;
    ptr++;
    p2 = ptr;
    v2 = 0;
    while (ptr != end) {
        d = hex_digit(*ptr);
        if (d == 16)
            break;
        v2 = (v2 << 4) | d;
        ptr++;
    }
    if (ptr != end || ptr == p2 || v2 <= v1 || v2 > 0x10ffff)
        goto err;
    *first = v1;
    *last = v2;
    return;
err:
    fputs("Invalid unicode range in data file: '", stderr);
    fwrite(start, end - start, 1, stderr);
    fputs("'\n", stderr);
    exit(1);
}

unsigned int
datafile_read(struct datafile *restrict f,
              unsigned int *first, unsigned int *last,
              struct field fields[restrict MAX_FIELDS])
{
    unsigned char *p, *e, *p2, *e2;
    size_t sz, sz2;
    unsigned int r, r2, fr1, fr2, la1, la2;
    struct field f2[MAX_FIELDS];
    r = datafile_read_raw(f, fields);
    if (!r)
        return 0;
    if (r && f->is_unicode_data) {
        p = fields[1].ptr;
        e = fields[1].end;
        sz = e - p;
        if (sz > 9 && *p == '<' && !memcmp(e - 8, ", First>", 8)) {
            r2 = datafile_read_raw(f, f2);
            if (r2 != r)
                goto err;
            p2 = f2[1].ptr;
            e2 = f2[1].end;
            sz2 = e2 - p2;
            if (!(sz2 == sz - 1 && *p2 == '<' &&
                  !memcmp(e2 - 7, ", Last>", 7) &&
                  !memcmp(p + 1, p2 + 1, sz - 9)))
                goto err;
            fields[1].ptr += 1;
            fields[1].end -= 8;
            datafile_decode_range(&fr1, &la1, fields[0].ptr, fields[0].end);
            datafile_decode_range(&fr2, &la2, f2[0].ptr, f2[0].end);
            if (fr1 != la1 || fr2 != la2 || fr1 >= la2)
                goto err;
            *first = fr1;
            *last = la2;
            return r;
        }
    }
    datafile_decode_range(first, last, fields[0].ptr, fields[0].end);
    return r;
err:
    fputs("Invalid format of unicode data file\n", stderr);
    exit(1);
}
