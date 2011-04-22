#define MAX_FIELDS 16

struct datafile;

struct field {
    unsigned char *ptr, *end;
};

struct datafile *
datafile_open(char const *restrict name);

void
datafile_close(struct datafile *restrict f);

/* Read the fields from a data file.  The range of characters is
   returned in "first" and "last", and the fields are returned in
   "fields".  Fields will be set to NULL if not present.  Returns the
   number of fields read.  */
unsigned int
datafile_read(struct datafile *restrict f,
              unsigned int *first, unsigned int *last,
              struct field fields[restrict MAX_FIELDS]);
