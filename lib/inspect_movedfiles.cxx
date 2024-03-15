/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include "rpminspect.h"

static bool movedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool rebase = false;
    struct result_params params;
    const char *arch = NULL;
    char *noun = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that has not moved */
    if (!file->moved_path && !file->peer_file->moved_path) {
        return true;
    }

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    /* package architecture */
    arch = get_rpm_header_arch(file->rpm_header);

    init_result_params(&params);
    params.header = NAME_MOVEDFILES;
    params.file = file->localpath;
    params.arch = arch;

    if (rebase) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_OK;
        params.remedy = NULL;
    } else {
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.remedy = get_remedy(REMEDY_MOVEDFILES);
    }

    xasprintf(&noun, _("%s moved to ${FILE} on ${ARCH}"), file->peer_file->localpath);
    params.noun = noun;

    xasprintf(&params.msg, _("%s probably moved to %s on %s\n"), file->peer_file->localpath, file->localpath, arch);
    add_result(ri, &params);

    free(params.msg);
    free(noun);
    return false;
}

/*
 * Main driver for the 'movedfiles' inspection.
 */
bool inspect_movedfiles(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    result = foreach_peer_file(ri, NAME_MOVEDFILES, movedfiles_driver);

    if (result) {
        init_result_params(&params);
        params.header = NAME_MOVEDFILES;
        params.verb = VERB_OK;
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
