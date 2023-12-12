/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

#include "rpminspect.h"

static char **read_mo_strings(FILE *f, uint32_t numstrings, uint32_t offset)
{
    uint32_t i = 0;
    size_t s = 0;
    long pos = 0;
    mo_entry_t entry;
    char **array = NULL;

    assert(f != NULL);

    if (numstrings == 0) {
        return NULL;
    }

    /* set up the string array */
    array = calloc(numstrings, sizeof(*array));
    assert(array != NULL);

    /* position file at the beginning of the string offset table */
    if (fseek(f, offset, SEEK_SET) == -1) {
        err(EXIT_FAILURE, "fseek");
    }

    for (i = 0; i < numstrings; i++) {
        /* read the entry information */
        s = fread(&entry, sizeof(entry.length), 2, f);

        if (s != 2) {
            err(EXIT_FAILURE, "fread");
        }

        /* save current position of the file */
        pos = ftell(f);

        if (pos == -1) {
            err(EXIT_FAILURE, "ftell");
        }

        /* allocate memory and read the string from the offset */
        array[i] = calloc(1, entry.length + 1);
        assert(array[i] != NULL);

        if (fseek(f, entry.offset, SEEK_SET) == -1) {
            err(EXIT_FAILURE, "fseek");
        }

        s = fread(array[i], sizeof(char), entry.length, f);

        if (s != entry.length) {
            err(EXIT_FAILURE, "fread");
        }

        /* move back to the position where the entry data is stored */
        if (fseek(f, pos, SEEK_SET) == -1) {
            err(EXIT_FAILURE, "fseek");
        }
    }

    return array;
}

mo_file_t *read_mofile(const char *filename)
{
    uint32_t i = 0;
    long pos = 24;
    size_t s = 0;
    FILE *f = NULL;
    uint8_t b;
    mo_file_t *mf = NULL;

    assert(filename != NULL);

    /* set things up */
    mf = calloc(1, sizeof(*mf));
    assert(mf != NULL);

    /* open the alleged .mo file */
    f = fopen(filename, "rb");

    if (f == NULL) {
        err(EXIT_FAILURE, "fopen");
    }

    /* read the magic number */
    /* (read this way to preserve file byte order mark) */
    pos = 24;

    for (i = 0; i < sizeof(mf->magic); i++) {
        s = fread(&b, 1, 1, f);

        if (s != 1) {
            err(EXIT_FAILURE, "fread");
        }

        mf->magic += b << pos;
        pos -= 8;
    }

    /* read the other fields */
    s = fread(&(mf->ver), sizeof(mf->ver), 6, f);

    if (s != 6) {
        err(EXIT_FAILURE, "fread");
    }

    /* read the strings */
    mf->original = read_mo_strings(f, mf->numstrings, mf->origoffset);
    mf->translated = read_mo_strings(f, mf->numstrings, mf->transoffset);

    for (i = 0; i < mf->numstrings; i++) {
DEBUG_PRINT("O: |%s|\n", mf->original[i]);
DEBUG_PRINT("T: |%s|\n\n", mf->translated[i]);
    }

    /* clean up */
    if (fclose(f) != 0) {
        err(EXIT_FAILURE, "fclose");
    }

    return mf;
}
