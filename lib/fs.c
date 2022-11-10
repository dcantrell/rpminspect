/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <sys/statvfs.h>

#include "rpminspect.h"

unsigned long get_available_space(const char *path)
{
    unsigned long r = 0;
    struct statvfs svb;

    assert(path != NULL);

    if (statvfs(path, &svb) == -1) {
        warn("statvfs");
        return 0;
    }

    r = svb.f_bsize * svb.f_bavail;

    return r;
}
