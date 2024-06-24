/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <assert.h>
#include <rpm/header.h>

#include "rpminspect.h"

/*
 * Main driver for the 'subpackages' inspection.
 */
bool inspect_subpackages(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    string_list_t *before_pkgs = NULL;
    string_list_t *after_pkgs = NULL;
    string_entry_t *entry = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    char *arch = NULL;
    struct result_params params;

    assert(ri != NULL);

    before_pkgs = (string_list_t *) xalloc(sizeof(*before_pkgs));
    TAILQ_INIT(before_pkgs);

    after_pkgs = (string_list_t *) xalloc(sizeof(*after_pkgs));
    TAILQ_INIT(after_pkgs);

    /* Gather up all the package names */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->before_hdr) {
            entry = (string_entry_t *) xalloc(sizeof(*entry));
            xasprintf(&entry->data, "%s %s", headerGetString(peer->before_hdr, RPMTAG_NAME), get_rpm_header_arch(peer->before_hdr));
            TAILQ_INSERT_TAIL(before_pkgs, entry, items);
        }

        if (peer->after_hdr) {
            entry = (string_entry_t *) xalloc(sizeof(*entry));
            xasprintf(&entry->data, "%s %s", headerGetString(peer->after_hdr, RPMTAG_NAME), get_rpm_header_arch(peer->after_hdr));
            TAILQ_INSERT_TAIL(after_pkgs, entry, items);
        }
    }

    /* Compute what was lost and gained */
    lost = list_difference(before_pkgs, after_pkgs);
    gain = list_difference(after_pkgs, before_pkgs);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_SUBPACKAGES;

    /* Report results */
    if (lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            arch = strstr(entry->data, " ");
            *arch = '\0';
            arch++;

            if (allowed_arch(ri, arch)) {
                xasprintf(&params.msg, _("Subpackage '%s' has disappeared on '%s'"), entry->data, arch);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = get_remedy(REMEDY_SUBPACKAGES_LOST);
                params.arch = arch;
                params.file = entry->data;
                params.verb = VERB_REMOVED;
                params.noun = _("subpackage ${FILE}");
                add_result(ri, &params);
                free(params.msg);
            }
        }

        result = false;
    }


    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            arch = strstr(entry->data, " ");
            *arch = '\0';
            arch++;

            if (allowed_arch(ri, arch)) {
                xasprintf(&params.msg, _("Subpackage '%s' has appeared on '%s'"), entry->data, arch);
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.remedy = get_remedy(REMEDY_SUBPACKAGES_GAIN);
                params.arch = arch;
                params.file = entry->data;
                params.verb = VERB_ADDED;
                params.noun = _("subpackage ${FILE}");
                add_result(ri, &params);
                free(params.msg);
            }
        }

        result = false;
    }

    list_free(lost, free);
    list_free(gain, free);
    list_free(before_pkgs, free);
    list_free(after_pkgs, free);

    /* Sound the everything-is-ok alarm if everything is, in fact, ok */
    if (result) {
        init_result_params(&params);
        params.header = NAME_SUBPACKAGES;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
