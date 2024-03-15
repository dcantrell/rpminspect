/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <string.h>
#include <assert.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * Read the specified source package spec file and return the Release
 * value with the %{?dist} macro trimmed.  For example:
 *        Release:    47%{?dist}
 * Will return this string:
 *        47
 * Release values are strings, so it has to be a string.  This function
 * allocates memory, so the caller is responsible for clearing it, but
 * it's likely going in to struct ri in which case that cleanup will
 * handle it.
 */
char *read_release(const rpmfile_t *files)
{
    char *rel = NULL;
    char *val = NULL;
    char *tail = NULL;
    rpmfile_entry_t *entry = NULL;
    string_list_t *spec = NULL;
    string_entry_t *specline = NULL;

    assert(files != NULL);

    /* find the spec file in this package */
    TAILQ_FOREACH(entry, files, items) {
        if (strsuffix(entry->localpath, SPEC_FILENAME_EXTENSION)) {
            break;
        }
    }

    /* open and read the Release line */
    spec = read_file(entry->fullpath);
    assert(spec != NULL);

    TAILQ_FOREACH(specline, spec, items) {
        if (strprefix(specline->data, "Release:")) {
            /* advance past 'Release:' */
            val = strchr(specline->data, ':');
            while (*val++ == ':');

            /* advance to the value */
            while (*val == ' ' || *val == '\t') {
                val++;
            }

            /* now find the beginning of the macros and terminate it */
            tail = strchr(val, '%');
            if (tail && *tail == '%') {
                *tail = '\0';
            }

            /* copy the release value */
            rel = strdup(val);
            break;
        }
    }

    list_free(spec, free);

    return rel;
}

/*
 * Return the before build Release value from the spec file with the
 * dist tag trimmed.  Tries to return the cached value first,
 * otherwise does a lookup and caches it.
 */
const char *get_before_rel(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;

    assert(ri != NULL);

    if (ri->before_rel == NULL) {
        TAILQ_FOREACH(peer, ri->peers, items) {
            if (peer->before_hdr && headerIsSource(peer->before_hdr)) {
                ri->before_rel = read_release(peer->before_files);
                break;
            }
        }
    }

    return ri->before_rel;
}

/*
 * Return the after build Release value from the spec file with the
 * dist tag trimmed.  Tries to return the cached value first,
 * otherwise does a lookup and caches it.
 */
const char *get_after_rel(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;

    assert(ri != NULL);

    if (ri->after_rel == NULL) {
        TAILQ_FOREACH(peer, ri->peers, items) {
            if (peer->after_hdr && headerIsSource(peer->after_hdr)) {
                ri->after_rel = read_release(peer->after_files);
                break;
            }
        }
    }

    return ri->after_rel;
}
