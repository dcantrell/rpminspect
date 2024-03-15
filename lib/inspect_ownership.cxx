/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

static bool ownership_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    return check_ownership(ri, file, NAME_OWNERSHIP, &reported, false);
}

/*
 * Main driver for the 'ownership' inspection.
 */
bool inspect_ownership(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);
    result = foreach_peer_file(ri, NAME_OWNERSHIP, ownership_driver);

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_OWNERSHIP;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
