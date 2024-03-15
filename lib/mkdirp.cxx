/*
 * Perform a 'mkdir -p' operation on a given path.
 */

/*
 * Copyright 2018-2021 David Cantrell <david.l.cantrell@gmail.com>
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "rpminspect.h"

int mkdirp(const char *path, mode_t mode)
{
    int r = 0;
    char *p = NULL;
    char *start = NULL;
    char *cwd = NULL;
    struct stat sb;

    assert(path != NULL);

    /* exit if path exists */
    r = stat(path, &sb);

    if (r == 0 && S_ISDIR(sb.st_mode)) {
        return 0;
    }

    /* handle relative vs. explicit paths, we want explicit */
    if (*path == '/') {
        start = p = strdup(path);
    } else {
        if ((cwd = getcwd(NULL, 0)) == NULL) {
            warn("*** getcwd");
            return -1;
        }

        xasprintf(&start, "%s/%s", cwd, path);
        free(cwd);
        p = start;
    }

    p++;

    /* handle all the parent directories */
    while (*p != '\0') {
        if (*p == '/') {
            *p = '\0';
            r = stat(start, &sb);

            if (r == 0 && S_ISDIR(sb.st_mode)) {
                *p = '/';
                p++;
                continue;
            } else if (mkdir(start, mode) == -1) {
                warn(_("*** unable to mkdir %s"), start);
                free(start);
                return -1;
            }

            *p = '/';
        }

        p++;
    }

    /* final directory */
    if ((stat(start, &sb) != 0) && (mkdir(start, mode) == -1)) {
        warn(_("*** unable to mkdir %s"), start);
        free(start);
        return -1;
    }

    free(start);

    return 0;
}
