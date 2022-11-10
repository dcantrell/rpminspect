/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/capability.h>

#include "rpminspect.h"

static bool capabilities_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    cap_t aftercap = NULL;
    cap_t beforecap = NULL;
    cap_t expected = NULL;
    char *after = NULL;
    char *before = NULL;
    const char *pkg = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    caps_filelist_entry_t *flcaps = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip debuginfo and debugsource packages */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
        return true;
    }

    /* Skip any file in a build-id path */
    if (strstr(file->localpath, BUILD_ID_DIR)) {
        return true;
    }

    /* Get the cap values */
    after = get_rpm_header_value(file, RPMTAG_FILECAPS);

    if (after && strcmp(after, "")) {
        aftercap = cap_from_text(after);
    } else {
        free(after);
        after = NULL;
    }

    if (file->peer_file) {
        before = get_rpm_header_value(file->peer_file, RPMTAG_FILECAPS);

        if (before && strcmp(before, "")) {
            beforecap = cap_from_text(before);
        } else {
            free(before);
            before = NULL;
        }
    }

    /* The architecture is used in reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_CAPABILITIES;
    params.arch = arch;
    params.file = file->localpath;

    /* Report if the caps are different */
    if (beforecap && aftercap) {
        if (cap_compare(beforecap, aftercap)) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

            if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                xasprintf(&params.msg, _("File capabilities for %s changed from '%s' to '%s' on %s\n"), file->localpath, before, after, arch);
                xasprintf(&params.remedy, REMEDY_CAPABILITIES, ri->caps_filename);
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_CHANGED;
                params.noun = _("${FILE} capabilities on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);
                free(params.remedy);
                result = false;
            }
        } else if (!cap_compare(beforecap, aftercap) && (ri->tests & INSPECT_CAPABILITIES)) {
            xasprintf(&params.msg, _("File capabilities found for %s: '%s' on %s\n"), file->localpath, after, arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
            add_result(ri, &params);
            free(params.msg);
        }
    }

    /* If we have after caps, check it against the caps list and report */
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);
    flcaps = get_caps_entry(ri, pkg, file->localpath);

    if (!aftercap && !flcaps) {
        free(after);
        free(before);
        return true;
    }

    if (flcaps && flcaps->caps) {
        expected = cap_from_text(flcaps->caps);
    }

    if (aftercap && expected) {
        if (!cap_compare(aftercap, expected) && (ri->tests & INSPECT_CAPABILITIES)) {
            xasprintf(&params.msg, _("File capabilities list entry found for %s: '%s' on %s, matches package\n"), file->localpath, flcaps->caps, arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
            add_result(ri, &params);
            free(params.msg);
        } else if (cap_compare(aftercap, expected) && (ri->tests & INSPECT_CAPABILITIES)) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

            if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                xasprintf(&params.msg, _("File capabilities list mismatch for %s: expected '%s', got '%s'\n"), file->localpath, flcaps->caps, after);
                xasprintf(&params.remedy, REMEDY_CAPABILITIES, ri->caps_filename);
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_FAILED;
                params.noun = _("${FILE} capabilities list on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);
                free(params.remedy);
                result = false;
            }
        }
    } else if (aftercap && !expected) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            xasprintf(&params.msg, _("File capabilities for %s (%s) not found on the capabilities list on %s\n"), file->localpath, after, arch);
            xasprintf(&params.remedy, REMEDY_CAPABILITIES, ri->caps_filename);
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.verb = VERB_REMOVED;
            params.noun = _("${FILE} capabilities list on ${ARCH}");
            add_result(ri, &params);
            free(params.msg);
            free(params.remedy);
            result = false;
        }
    } else if (!aftercap && expected) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            xasprintf(&params.msg, _("File capabilities expected for %s but not found on %s: expected '%s'\n"), file->localpath, arch, flcaps->caps);
            xasprintf(&params.remedy, REMEDY_CAPABILITIES, ri->caps_filename);
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} capabilities list on ${ARCH}");
            add_result(ri, &params);
            free(params.msg);
            free(params.remedy);
            result = false;
        }
    }

    free(before);
    free(after);

    return result;
}

/*
 * Main driver for the 'capabilities' inspection.
 */
bool inspect_capabilities(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    assert(ri != NULL);

    /* run the capabilities inspection across all RPM files */
    result = foreach_peer_file(ri, NAME_CAPABILITIES, capabilities_driver);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_CAPABILITIES;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
