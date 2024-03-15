/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <err.h>
#include <sys/capability.h>

#include "rpminspect.h"

static bool reported = false;

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

    if (is_debuginfo_rpm(file->rpm_header) || is_debugsource_rpm(file->rpm_header)) {
        return true;
    }

    /* Skip any file in a build-id path */
    if (strstr(file->localpath, BUILD_ID_DIR)) {
        return true;
    }

    /* Get the cap values */
    after = get_rpm_header_string_array_value(file, RPMTAG_FILECAPS);

    if (after && strcmp(after, "")) {
        aftercap = cap_from_text(after);
    } else {
        free(after);
        after = NULL;
    }

    if (file->peer_file) {
        before = get_rpm_header_string_array_value(file->peer_file, RPMTAG_FILECAPS);

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

    /* Check file mode and ownership*/
    result &= check_permissions(ri, file, params.header, &reported, ri->tests & INSPECT_CAPABILITIES);
    result &= check_ownership(ri, file, params.header, &reported, ri->tests & INSPECT_CAPABILITIES);

    /* Report if the caps are different */
    if (beforecap && aftercap) {
        if (cap_compare(beforecap, aftercap)) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

            if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                xasprintf(&params.msg, _("File capabilities for %s changed from '%s' to '%s' in %s on %s\n"), file->localpath, before, after, name, arch);
                params.remedy = get_remedy(REMEDY_CAPABILITIES);
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_CHANGED;
                params.noun = _("${FILE} capabilities on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);

                if (params.severity >= RESULT_VERIFY) {
                    result = false;
                }

                reported = true;
            }
        } else if (!cap_compare(beforecap, aftercap) && (ri->tests & INSPECT_CAPABILITIES)) {
            xasprintf(&params.msg, _("File capabilities found for %s: '%s' in %s on %s\n"), file->localpath, after, name, arch);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
            add_result(ri, &params);
            free(params.msg);
            reported = true;
        }
    }

    /* If we have after caps, check it against the caps list and report */
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);
    flcaps = get_caps_entry(ri, pkg, file->localpath);

    if (!aftercap && !flcaps) {
        free(after);
        free(before);

        if (cap_free(aftercap) == -1) {
            warn("*** cap_free");
        }

        if (cap_free(beforecap) == -1) {
            warn("*** cap_free");
        }

        return true;
    }

    if (flcaps && flcaps->caps) {
        expected = cap_from_text(flcaps->caps);
    }

    if (!beforecap) {
        if (aftercap && expected) {
            if (!cap_compare(aftercap, expected) && (ri->tests & INSPECT_CAPABILITIES)) {
                xasprintf(&params.msg, _("File capabilities list entry found for %s: '%s' in %s on %s, matches package\n"), file->localpath, flcaps->caps, name, arch);
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
                add_result(ri, &params);
                free(params.msg);
                reported = true;
            } else if (cap_compare(aftercap, expected) && (ri->tests & INSPECT_CAPABILITIES)) {
                params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

                if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                    xasprintf(&params.msg, _("File capabilities list mismatch for %s: expected '%s', got '%s' in %s on %s\n"), file->localpath, flcaps->caps, after, name, arch);
                    params.remedy = get_remedy(REMEDY_CAPABILITIES);
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    params.verb = VERB_FAILED;
                    params.noun = _("${FILE} capabilities list on ${ARCH}");
                    add_result(ri, &params);
                    free(params.msg);

                    if (params.severity >= RESULT_VERIFY) {
                        result = false;
                    }

                    reported = true;
                }
            }
        } else if (aftercap && !expected) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

            if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                xasprintf(&params.msg, _("File capabilities for %s (%s) not found on the capabilities list in %s on %s\n"), file->localpath, after, name, arch);
                params.remedy = get_remedy(REMEDY_CAPABILITIES);
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_REMOVED;
                params.noun = _("${FILE} capabilities list on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);

                if (params.severity >= RESULT_VERIFY) {
                    result = false;
                }

                reported = true;
            }
        } else if (!aftercap && expected) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_CAPS);

            if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                xasprintf(&params.msg, _("File capabilities expected for %s in %s but not found on %s: expected '%s'\n"), file->localpath, name, arch, flcaps->caps);
                params.remedy = get_remedy(REMEDY_CAPABILITIES);
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_FAILED;
                params.noun = _("${FILE} capabilities list on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);

                if (params.severity >= RESULT_VERIFY) {
                    result = false;
                }

                reported = true;
            }
        }
    }

    free(before);
    free(after);

    if (cap_free(aftercap) == -1) {
        warn("*** cap_free");
    }

    if (cap_free(beforecap) == -1) {
        warn("*** cap_free");
    }

    if (cap_free(expected) == -1) {
        warn("*** cap_free");
    }

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
    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_CAPABILITIES;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
