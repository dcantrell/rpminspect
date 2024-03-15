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

static void append_macros(string_list_t **macros, const char *s)
{
    string_list_t *new_macros = NULL;
    string_entry_t *entry = NULL;

    if (s == NULL) {
        return;
    }

    new_macros = get_macros(s);

    if (new_macros == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(new_macros)) {
        /* take the first new macro */
        entry = TAILQ_FIRST(new_macros);
        TAILQ_REMOVE(new_macros, entry, items);

        /* add the macro to the list if not found */
        if (list_contains(*macros, entry->data)) {
            free(entry->data);
            free(entry);
        } else {
            TAILQ_INSERT_TAIL(*macros, entry, items);
        }
    }

    free(new_macros);
    return;
}

static bool check_release_macros(const int macrocount, const pair_list_t *macros, const char *release, const char *disttag)
{
    bool ret = false;
    string_list_t *tag_macros = NULL;
    string_entry_t *macro = NULL;
    pair_entry_t *pair = NULL;
    int found = 0;
    int valid = 0;

    if (macrocount == 0 || release == NULL || disttag == NULL) {
        return false;
    }

    /* get an initial list of macros from the Release string */
    tag_macros = get_macros(release);

    if (tag_macros == NULL) {
        return false;
    } else if (TAILQ_EMPTY(tag_macros)) {
        list_free(tag_macros, free);
        return false;
    }

    /* iterate over macros until we have no more */
    while (!TAILQ_EMPTY(tag_macros)) {
        /* take the first one from the list */
        macro = TAILQ_FIRST(tag_macros);
        TAILQ_REMOVE(tag_macros, macro, items);

        /* check this macro value for the dist tag */
        found = valid = 0;

        TAILQ_FOREACH(pair, macros, items) {
            if (!strcmp(pair->key, macro->data)) {
                found++;

                /* collect any new macros in this macro value */
                append_macros(&tag_macros, pair->value);

                if (strstr(pair->value, SPEC_DISTTAG)) {
                    valid++;
                }
            }
        }

        /* clean up the macro we removed */
        free(macro->data);
        free(macro);

        /* end early if we have found the dist tag */
        if (found == valid) {
            ret = true;
            break;
        }
    }

    /* clean up */
    list_free(tag_macros, free);

    return ret;
}

static bool disttag_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *buf = NULL;
    char *release = NULL;
    char *expanded_release = NULL;
    int macrocount = 0;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Read in spec file macros */
    macrocount = get_specfile_macros(ri, file->fullpath);

    /* Check for the %{?dist} macro in the Release value */
    contents = read_file(file->fullpath);

    if (contents == NULL) {
        return true;
    }

    TAILQ_FOREACH(entry, contents, items) {
        buf = entry->data;

        /* trim line endings */
        buf[strcspn(buf, "\r\n")] = 0;

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
    if (release && !strcmp(release, SPEC_AUTORELEASE)) {
        list_free(contents, free);
        return true;
    }

    /* Expand macros in the release value */
    expanded_release = rpmExpand(release, NULL);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_DISTTAG;
    params.remedy = get_remedy(REMEDY_DISTTAG);
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
    } else if (strstr(release, SPEC_DISTTAG) || strstr(expanded_release, DIST_TAG_MARKER)) {
        result = true;
    } else if (!check_release_macros(macrocount, ri->macros, release, SPEC_DISTTAG)) {
        xasprintf(&params.msg, _("The %s tag value is missing the dist tag in the proper form. The dist tag should be of the form '%s' in the %s tag or in a macro used in the %s tag. After RPM macro expansion, no dist tag was found in this %s tag value."), SPEC_TAG_RELEASE, SPEC_DISTTAG, SPEC_TAG_RELEASE, SPEC_TAG_RELEASE, SPEC_TAG_RELEASE);
        params.verb = VERB_FAILED;
        params.noun = _("${FILE} does not use '%%{?dist}' in Release");
        add_result(ri, &params);
        result = false;
    }

    free(expanded_release);
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
