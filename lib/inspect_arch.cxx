/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <assert.h>
#include <rpm/header.h>

#include "rpminspect.h"

/*
 * Main driver for the 'arch' inspection.
 */
bool inspect_arch(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    const char *before_arch = NULL;
    const char *after_arch = NULL;
    string_list_t *before_arches = NULL;
    string_list_t *after_arches = NULL;
    string_entry_t *entry = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    struct result_params params;

    assert(ri != NULL);

    before_arches = xalloc(sizeof(*before_arches));
    TAILQ_INIT(before_arches);

    after_arches = xalloc(sizeof(*after_arches));
    TAILQ_INIT(after_arches);

    init_result_params(&params);
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_ARCH;

    /* Gather up all the architectures */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->before_hdr == NULL || peer->after_hdr == NULL) {
            /* missing peer packages; handled by other inspections */
            continue;
        }

        before_arch = get_rpm_header_arch(peer->before_hdr);
        after_arch = get_rpm_header_arch(peer->after_hdr);

        /*
         * loss of a noarch package is not something this inspection
         * needs to be concerned with.  this inspection checks for
         * loss of a target machine architecture as provided by the
         * before build
         */
        if (!strcmp(before_arch, "noarch") || !strcmp(after_arch, "noarch")) {
            continue;
        }

        before_arches = list_add(before_arches, before_arch);
        after_arches = list_add(after_arches, after_arch);
    }

    /* Compute what was lost and gained */
    lost = list_difference(before_arches, after_arches);
    gain = list_difference(after_arches, before_arches);

    /* Report results */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            if (allowed_arch(ri, entry->data)) {
                params.severity = RESULT_VERIFY;
                params.remedy = get_remedy(REMEDY_ARCH_LOST);
                params.verb = VERB_REMOVED;
                params.noun = _("lost ${ARCH}");
                params.arch = entry->data;
                xasprintf(&params.msg, _("Architecture '%s' has disappeared"), entry->data);
                add_result(ri, &params);
                free(params.msg);
            }
        }

        result = false;
    }

    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            if (allowed_arch(ri, entry->data)) {
                params.severity = RESULT_INFO;
                params.remedy = get_remedy(REMEDY_ARCH_GAIN);
                params.verb = VERB_ADDED;
                params.noun = _("gained ${ARCH}");
                params.arch = entry->data;
                xasprintf(&params.msg, _("Architecture '%s' has appeared"), entry->data);
                add_result(ri, &params);
                free(params.msg);
            }
        }

        result = false;
    }

    list_free(lost, free);
    list_free(gain, free);
    list_free(before_arches, free);
    list_free(after_arches, free);

    return result;
}
