/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rpminspect.h"

/**
 * Given a byte array of a specified length, convert it to a NUL
 * terminated string and return a pointer to the string to the caller.
 * The caller is responsible for freeing the memory associated with
 * this string.
 *
 * @param array The byte array to convert.
 * @param len Number of elements in the byte array.
 * @return Newly allocated string representation of the byte array.
 */
char *bytes_to_str(unsigned char *array, size_t len)
{
    size_t i;
    char *ret = NULL;
    char *walk = NULL;

    if (array == NULL) {
        return NULL;
    }

    ret = calloc(1, sizeof(char) * (len + 1));
    assert(ret != NULL);
    walk = ret;

    for (i = 0; i < len; i++) {
        snprintf(walk, 2, "%c", array[i]);
        walk++;
    }

    return ret;
}
