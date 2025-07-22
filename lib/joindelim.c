/*
 * Given a list of path components as separate strings, join them in to a
 * correct (but unverified) Unix path.  Extra slashes are removed.  Spaces
 * and other special characters are not escaped.  This function allocates
 * memory for the returned value.  The caller must free this memory when
 * done.
 *
 * Usage: path = join(a, b, c, ..., NULL);
 * (where a, b, and c are char *)
 */

/* {{{ Apache License version 2.0
 */
/*
 * Copyright 2018 David Cantrell <david.l.cantrell@gmail.com>
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
/* }}} */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include "rpminspect.h"

/*
 * Join all of the strings starting at s with the character delim.
 * This function preserves a single delim when joining strings, so if
 * strings passed in start or end with delim, those will be reduced
 * down to a single delim joining the string.  The typical use case is
 * for joining path strings.
 */
char *joindelim(const char delim, const char *s, ...)
{
    va_list ap;
    int i = 0;
    bool needsep = false;
    char *element = NULL;
    char *tail = NULL;
    char *built = NULL;
    char *tmp = NULL;
    char *near = NULL;
    char *far = NULL;

    assert(s != NULL);

    /* Allocate a large buffer to use for building the string. */
    built = xalloc(PATH_MAX + 1);

    /* begin our joined path */
    tail = stpcpy(built, s);

    /* the remaining elements come in this way */
    va_start(ap, s);

    while ((element = va_arg(ap, char *)) != NULL) {
        /* for the trailing NUL */
        needsep = false;

        /* trim any extra trailing delimiters */
        while (*tail == delim) {
            *tail = '\0';
            tail--;
        }

        /*
         * This loop trims multiple leading delims down to just one.
         * If 'i' is 1, it will preserve 1 leading delim.  Actually, set
         * this to the number of delims to preserve.
         */
        if (*element == delim && *tail != delim) {
            i = 1;
        } else {
            i = 0;
        }

        while (*(element + i) == delim) element++;

        /* make sure we have at least one delim in case there are none */
        if (*element != delim && *tail != delim) {
            needsep = true;
        }

        /* perform the concatenations */
        if (needsep) {
            xasprintf(&tmp, "%c", delim);
            assert(tmp != NULL);
            tail = stpcpy(tail, tmp);
            free(tmp);
        }

        tail = stpcpy(tail, element);
    }

    va_end(ap);

    /* it's possible there are repeating delims, eliminate them */
    near = built;
    far = built;

    while ((*near = *far)) {
        if (*near == delim) {
            while (*far == delim) {
                far++;
            }

            near++;
        } else {
            near++;
            far++;
        }
    }

    /* shrink memory allocation */
    tmp = xrealloc(built, strlen(built) + 1);
    built = tmp;

    return built;
}
