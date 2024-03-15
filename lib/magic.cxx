/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <magic.h>

#include "rpminspect.h"

static void init_magic_cookie(struct rpminspect *ri)
{
    assert(ri != NULL);

    ri->magic_cookie = magic_open(MAGIC_MIME | MAGIC_CHECK);

    if (ri->magic_cookie == NULL) {
        warnx(_("*** unable to initialize the magic library"));
        return;
    }

    if (magic_load(ri->magic_cookie, NULL) != 0) {
        warnx(_("*** unable to load the magic database: %s"), magic_error(ri->magic_cookie));
        magic_close(ri->magic_cookie);
        return;
    }

    ri->magic_initialized = true;
    return;
}

/*
 * Get the MIME type of a file specified by path rather than
 * rpmfile_entry_t.  It does use the open libmagic handle if it's
 * available.  Caller should not free the returned string.
 */
const char *mime_type(struct rpminspect *ri, const char *file)
{
    const char *tmp = NULL;
    char *type = NULL;
    char *pos = NULL;
    string_hash_t *entry = NULL;

    assert(ri != NULL);

    if (file == NULL) {
        return NULL;
    }

    /* only try to initialize libmagic if it hasn't been yet */
    if (!ri->magic_initialized) {
        init_magic_cookie(ri);
    }

    /* get the type and see if it needs to be saved */
    tmp = magic_file(ri->magic_cookie, file);

    if (tmp) {
        type = strdup(tmp);
        assert(type != NULL);
    } else {
        return NULL;
    }

    if (type != NULL) {
        /*
         * Trim any trailing metadata after the MIME type, such
         * as '; charset=utf-8' and stuff like that.
         */
        pos = strchr(type, ';');

        if (pos != NULL) {
            *pos = '\0';
        }

        /* look for the type first, add if not found */
        HASH_FIND_STR(ri->magic_types, type, entry);

        if (entry == NULL) {
            /* start a new entry for this type */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = strdup(type);
            HASH_ADD_KEYPTR(hh, ri->magic_types, entry->data, strlen(entry->data), entry);
        }
    }

    free(type);
    return entry->data;
}

/*
 * Return the MIME type of the specified file.  The type is cached in the
 * rpmfile_entry_t.  If that is not NULL, this function returns that value.
 * Otherwise it gets the MIME type, caches it, and returns the value.
 * The caller should not free the pointer returned.
 */
const char *get_mime_type(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    assert(ri != NULL);
    assert(file != NULL);

    /* no actual file; no actual MIME type */
    if (file->fullpath == NULL) {
        return NULL;
    }

    /* the type may already be cached */
    if (file->type) {
        return file->type;
    }

    /* look it up */
    return mime_type(ri, file->fullpath);
}

/* Return true if the named file is a text file according to libmagic */
bool is_text_file(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool ret = false;
    const char *type = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    type = get_mime_type(ri, file);

    if (strprefix(type, "text/")) {
        ret = true;
    }

    return ret;
}
