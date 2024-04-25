/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <math.h>

#include "rpminspect.h"

char *human_size(const unsigned long int bytes)
{
    char *r = NULL;
    double s = 0;
    const char units[] = "BKMGTPEZY";
    int i = 0;

    s = (double) bytes;

    while ((s > 1024) && (units[i] != '\0')) {
        s = round(s / 1024);
        i++;
    }

    xasprintf(&r, "%lu %c%s", lround(s), units[i], (units[i] == 'B') ? "" : "iB");
    return r;
}
