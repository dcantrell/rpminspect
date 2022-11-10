/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool specgood = false;
static bool seen = false;

static bool specname_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    char *specfile = NULL;
    char *dot = NULL;
    char *desc = NULL;
    char *primary = NULL;
    struct result_params params;

    /* Skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* We only want to look at the spec file */
    if (!strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
        return true;
    }

    /* Spec files are all named in a standard way */
    if (ri->specprimary == PRIMARY_NAME) {
        primary = strdup(headerGetString(file->rpm_header, RPMTAG_NAME));
    } else if (ri->specprimary == PRIMARY_FILENAME) {
        primary = strdup(file->localpath);
        dot = rindex(primary, '.');
        assert(dot != NULL);
        *dot = '\0';
    }

    xasprintf(&specfile, "%s%s", primary, SPEC_FILENAME_EXTENSION);

    /* Match spec file name per conf file rule */
    if (ri->specmatch == MATCH_FULL && !strcmp(file->localpath, specfile)) {
        specgood = true;
    } else if (ri->specmatch == MATCH_PREFIX && strprefix(file->localpath, primary)) {
        specgood = true;
    } else if (ri->specmatch == MATCH_SUFFIX && strsuffix(file->localpath, specfile)) {
        specgood = true;
    }

    /*
     * Emit a failure if we're looking at what we think is a spec file
     * but it's not named in the expected way.
     */
    if (!specgood) {
        /* Set up result parameters */
        init_result_params(&params);
        params.severity = RESULT_BAD;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_SPECNAME;
        params.remedy = REMEDY_SPECNAME;
        params.file = file->localpath;
        params.verb = VERB_FAILED;
        params.noun = _("unexpected spec filename");

        if (ri->specmatch == MATCH_FULL) {
            desc = _("exactly match");
        } else if (ri->specmatch == MATCH_PREFIX) {
            desc = _("begin with");
        } else if (ri->specmatch == MATCH_SUFFIX) {
            desc = _("end with");
        }

        xasprintf(&params.msg, _("Spec filename does not %s the primary name %s; got '%s'"), desc, primary, file->localpath);
        add_result(ri, &params);
        free(params.msg);
    }

    free(primary);
    free(specfile);
    seen = true;
    return specgood;
}

/*
 * Main driver for the 'specname' inspection.
 */
bool inspect_specname(struct rpminspect *ri)
{
    struct result_params params;

    assert(ri != NULL);
    foreach_peer_file(ri, NAME_SPECNAME, specname_driver);

    init_result_params(&params);
    params.header = NAME_SPECNAME;
    params.verb = VERB_OK;

    if (specgood) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!seen) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, _("The specname inspection is only for source packages, skipping."));
        add_result(ri, &params);
        free(params.msg);

        /*
         * There's no reason to fail this test for an informational message.
         */
        specgood = true;
    }

    return specgood;
}
