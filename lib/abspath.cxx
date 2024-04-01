/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "rpminspect.h"

/*
 * Canonicalize a path with relative references.  Does not rely on
 * filesystem existence.  Caller must free the returned string.
 */
char *abspath(const char *path)
{
    char *r = NULL;
    char *p = NULL;
    const char *delim = "/";
    string_list_t *tokens = NULL;
    string_list_t *newpath = NULL;
    string_entry_t *token = NULL;
    string_entry_t *element = NULL;

    if (path == NULL) {
        return NULL;
    }

    if (!strcmp(path, "") || !strcmp(path, delim)) {
        return strdup(path);
    }

    /* split path in to tokens */
    tokens = strsplit(path, delim);
    assert(tokens != NULL);

    /* our new path elements */
    newpath = (string_list_t *) calloc(1, sizeof(*newpath));
    assert(newpath != NULL);
    TAILQ_INIT(newpath);

    /* handle each part of the path */
    TAILQ_FOREACH(token, tokens, items) {
        if (!strcmp(token->data, "") || !strcmp(token->data, ".") || (!strcmp(token->data, "..") && TAILQ_EMPTY(newpath))) {
            /* no need to add this token */
            continue;
        } else if (!strcmp(token->data, "..") && !TAILQ_EMPTY(newpath)) {
            /* back up a path element */
            element = TAILQ_LAST(newpath, string_entry_s);
            TAILQ_REMOVE(newpath, element, items);
            free(element->data);
            free(element);
        } else {
            /* take this path element */
            newpath = list_add(newpath, token->data);
        }
    }

    /* generate the final path string */
    p = list_to_string(newpath, "/");

    if (p) {
        xasprintf(&r, "/%s", p);
    } else {
        r = strdup("/");
    }

    /* clean up */
    list_free(tokens, free);
    list_free(newpath, free);
    free(p);

    return r;
}
