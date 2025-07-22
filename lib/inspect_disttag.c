/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <rpm/rpmmacro.h>
#include "rpminspect.h"

/*
 * This is used internally by librpminspect to match spec files that
 * used %{?dist} after RPM macro expansion.
 *
 * One potential problem with this substring is someone using it in
 * their own macro definitions.  So this tries to be unique and weird
 * in a way that no one would use it.  But I guess never say never,
 * right?
 */
#define DIST_TAG_MARKER "_._._._._._._.D.I.S.T._._._._._._._"

static bool disttag_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    string_list_t *contents = NULL;
    string_list_t *fields = NULL;
    string_entry_t *line = NULL;
    char *buf = NULL;
    char *release = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* read in the spec file */
    contents = read_spec(file->fullpath);

    if (contents == NULL) {
        return true;
    }

    /* find the Release: line */
    TAILQ_FOREACH(line, contents, items) {
        buf = line->data;

        /* we made it to the changelog, nothing left of value */
        if (strprefix(buf, SPEC_SECTION_CHANGELOG)) {
            break;
        }

        /* found the line to check */
        if (strprefix(buf, SPEC_TAG_RELEASE)) {
            break;
        }
    }

    /* Only look at the value on the Release: line */
    release = buf + strlen(SPEC_TAG_RELEASE);
    assert(release != NULL);

    while (isspace(*release) && *release != '\0') {
        release++;
    }

    /* Allow %autorelease as the Release tag value */
    if (release && strprefix(release, SPEC_AUTORELEASE)) {
        fields = strsplit(release, " \t");

        if (list_contains(fields, SPEC_AUTORELEASE)) {
            list_free(contents, free);
            list_free(fields, free);
            return true;
        }

        list_free(fields, free);
    }

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_DISTTAG;
    params.remedy = REMEDY_DISTTAG;
    params.details = release;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    /* Check the line if we found it */
    if (release == NULL) {
        xasprintf(&params.msg, _("The %s file is missing the %s tag."), file->localpath, SPEC_TAG_RELEASE);
        params.verb = VERB_REMOVED;
        params.noun = _("${FILE} missing Release tag");
        add_result(ri, &params);
        result = false;
    } else {
        if (strstr(release, SPEC_DISTTAG) || strstr(release, DIST_TAG_MARKER)) {
            result = true;
        } else {
            xasprintf(&params.msg, _("The %s tag value is missing the dist tag in the proper form. The dist tag should be of the form '%s' in the %s tag or in a macro used in the %s tag. After RPM macro expansion, no dist tag was found in this %s tag value."), SPEC_TAG_RELEASE, SPEC_DISTTAG, SPEC_TAG_RELEASE, SPEC_TAG_RELEASE, SPEC_TAG_RELEASE);
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} does not use '%%{?dist}' in Release");
            add_result(ri, &params);
            result = false;
        }
    }

    free(params.msg);
    list_free(contents, free);
    return result;
}

/*
 * Main driver for the 'disttag' inspection.
 */
bool inspect_disttag(struct rpminspect *ri)
{
    bool result = true;
    bool src = false;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct result_params params;

    assert(ri != NULL);

    /*
     * Only run over the package headers and mark if
     * we never see an SRPM file.
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        /* We have at least one source package */
        src = true;

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* skip all source files except the spec file */
            if (!strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                continue;
            }

            /* Define the dist macro for rpminspect */
            (void) rpmDefineMacro(NULL, "dist " DIST_TAG_MARKER, 0);

            /* Analyze the spec file */
            result = disttag_driver(ri, file);

            /* Done after looking at the spec file */
            break;
        }
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_DISTTAG;
    params.verb = VERB_OK;

    /* If we never saw an SRPM, tell the user. */
    if (result && src) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!src) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.msg = _("Specified package is not a source RPM, skipping.");
        add_result(ri, &params);
    }

    return result;
}
