/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

static bool permissions_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    return check_permissions(ri, file, NAME_PERMISSIONS, &reported, false);
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
