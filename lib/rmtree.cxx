/*
 * Copyright 2003 David Shea <david@reallylongword.org>
 *                David Cantrell <david.l.cantrell@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <err.h>
#include <ftw.h>

#include "rpminspect.h"

/* Local prototypes */
static int rmtree_entry(const char *, const struct stat *, int, struct FTW *);

static bool contents_only = false;

static int rmtree_entry(const char *fpath, __attribute__((unused)) const struct stat *sb, __attribute__((unused)) int tflag, struct FTW *ftwbuf)
{
    int r = 0;

    assert(fpath != NULL);

    if ((contents_only && ftwbuf->level != 0) || !contents_only) {
        r = remove(fpath);
    }

    return r;
}

/* Remove specified tree, ignoring errors if specified. */
int rmtree(const char *path, const bool ignore_errors, const bool contentsonly)
{
    struct stat sb;
    int flags = FTW_DEPTH | FTW_MOUNT | FTW_PHYS;

    if (path == NULL) {
        return 0;
    }

    contents_only = contentsonly;

    if (stat(path, &sb) == -1) {
        if (ignore_errors) {
            return 0;
        } else {
            warn("*** stat");
            return 1;
        }
    }

    if (!S_ISDIR(sb.st_mode)) {
        if (ignore_errors) {
            return 0;
        } else {
            warnx(_("*** %s is not a directory"), path);
            return 2;
        }
    }

    return nftw(path, rmtree_entry, FOPEN_MAX, flags);
}
