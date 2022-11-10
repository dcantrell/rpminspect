/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

static bool permissions_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    const char *what = _("changed");
    mode_t before_mode;
    mode_t after_mode;
    mode_t mode_diff;
    bool allowed = false;
    char *noun = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debuginfo and debugsource packages */
    if (strprefix(file->localpath, DEBUG_PATH) || strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_PERMISSIONS;
    params.arch = arch;
    params.file = file->localpath;

    /* Local working copies for display */
    after_mode = file->st.st_mode & 0777;

    /* Compare the modes */
    if (file->peer_file) {
        before_mode = file->peer_file->st.st_mode & 0777;
    } else {
        memset(&before_mode, 0, sizeof(before_mode));
    }

    mode_diff = before_mode ^ after_mode;
    allowed = match_fileinfo_mode(ri, file, NAME_PERMISSIONS, NULL);

    /* if setuid/setgid or new mode is more open */
    if (mode_diff && file->peer_file && !allowed && (ri->tests & INSPECT_PERMISSIONS)) {
        if (!(before_mode & (S_ISUID|S_ISGID)) && (after_mode & (S_ISUID|S_ISGID))) {
            params.severity = RESULT_VERIFY;
            what = _("changed setuid/setgid");
            result = false;
        } else if (S_ISDIR(file->st.st_mode) && !S_ISLNK(file->st.st_mode)) {
            if (((mode_diff & S_ISVTX) && !(after_mode & S_ISVTX)) || ((after_mode & mode_diff) != 0)) {
                params.severity = RESULT_VERIFY;
                what = _("relaxed");
            }
        }

        if (params.severity >= RESULT_VERIFY) {
            params.waiverauth = WAIVABLE_BY_ANYONE;
        }

        xasprintf(&params.msg, _("%s %s permissions from %04o to %04o on %s"), file->localpath, what, before_mode, after_mode, arch);
        params.verb = VERB_CHANGED;
        xasprintf(&noun, _("${FILE} permissions from %04o to %04o on ${ARCH}"), before_mode, after_mode);
        params.noun = noun;
        add_result(ri, &params);
        reported = true;
        free(params.msg);
        free(noun);
    }

    /* check for world-writability */
    if (!allowed && !S_ISLNK(file->st.st_mode) && ((after_mode & (S_IWOTH|S_ISVTX)) || (after_mode & S_IWOTH))) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_WORLDWRITABLE);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            xasprintf(&params.msg, _("%s (%s) is world-writable on %s"), file->localpath, get_mime_type(file), arch);
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} is world-writable on ${ARCH}");
            add_result(ri, &params);
            reported = true;
            free(params.msg);
            result = false;
        }
    }

    return result;
}

/*
 * Main driver for the 'permissions' inspection.
 */
bool inspect_permissions(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    /* run the permissions inspection across all RPM files */
    result = foreach_peer_file(ri, NAME_PERMISSIONS, permissions_driver);

    /* if everything was fine, just say so */
    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_PERMISSIONS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
