/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "rpminspect.h"

/*
 * Compares two files.  The return value of this function matches what
 * memcmp() returns (mostly).  The function will return 1 if the sizes
 * read of each file are different and it will skip the comparison
 * entirely.
 */
int filecmp(const char *x, const char *y)
{
    int r = 0;
    void *xbuf = NULL;
    void *ybuf = NULL;
    off_t xlen = 0;
    off_t ylen = 0;

    assert(x != NULL);
    assert(y != NULL);

    /* read in the files */
    xbuf = read_file_bytes(x, &xlen);
    ybuf = read_file_bytes(y, &ylen);

    /* they are different if the sizes read are different */
    if (xlen != ylen) {
        free(xbuf);
        free(ybuf);
        return 1;
    }

    r = memcmp(xbuf, ybuf, xlen);
    free(xbuf);
    free(ybuf);

    return r;
}
