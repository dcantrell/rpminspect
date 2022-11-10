/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <math.h>

#include "rpminspect.h"

char *human_size(const unsigned long bytes)
{
    char *r = NULL;
    double s = 0;
    char *units = "BKMGTPEZY";
    char *begin = units;

    s = (double) bytes;
    units = begin;

    while ((s > 1024) && (*(units + 1) != '\0')) {
        s = round(s / 1024);
        units++;
    }

    xasprintf(&r, "%lu %c%s", lround(s), *units, (*units == 'B') ? "" : "iB");
    return r;
}
