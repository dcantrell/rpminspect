/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <rpm/rpmbuild.h>
#include <rpm/rpmspec.h>
#include "rpminspect.h"

/*
 * Given a spec file's full path, read it in and have librpm fully
 * parse it and expand macros and then return the resulting fully
 * parsed spec file text as a string_list_t that we can iterate over
 * line by line.  Caller is responsible for freeing the returned list.
 * Returns NULL on failure.
 */
string_list_t *read_spec(const char *specfile)
{
    string_list_t *r = NULL;
    rpmSpec spec = NULL;
    const char *s = NULL;

    assert(specfile != NULL);

    /* try to read and parse the spec file */
    spec = rpmSpecParse(specfile, RPMBUILD_NOBUILD, NULL);

    if (spec == NULL) {
        warn("rpmSpecParse");
        return NULL;
    }

    /* get the fully expanded spec file */
    s = rpmSpecGetSection(spec, RPMBUILD_NONE);

    if (s == NULL) {
        warn("rpmSpecGetSection");
        return NULL;
    }

    /* split in to lines */
    r = strsplit(s, "\n\r");

    return r;
}
