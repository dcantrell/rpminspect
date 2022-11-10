/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <rpm/header.h>
#include "queue.h"
#include "uthash.h"
#include "rpminspect.h"

/* All of the macros that can appear in a %files section */
static const char *files_macros[] = { "%artifact",
                                      "%attr",
                                      "%config",
                                      "%defattr",
                                      "%defverify",
                                      "%dev",
                                      "%dir",
                                      "%doc",
                                      "%docdir",
                                      "%exclude",
                                      "%ghost",
                                      "%lang",
                                      "%license",
                                      "%missingok",
                                      "%pubkey",
                                      "%readme",
                                      "%verify",
                                      NULL };

static bool files_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    bool in_files = false;
    bool valid_macro = false;
    long line = 1;
    int i = 0;
    struct result_params params;
    string_list_t *spec = NULL;
    string_entry_t *specline = NULL;
    string_entry_t *path = NULL;
    string_list_map_t *mapentry = NULL;
    bool ignore = false;
    char *noun = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* if there are no forbidden paths defined, we pass */
    if (ri->forbidden_paths == NULL || TAILQ_EMPTY(ri->forbidden_paths)) {
        return true;
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_FILES;
    params.remedy = REMEDY_FILES;
    params.file = file->localpath;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.verb = VERB_FAILED;

    /* read in the spec file */
    spec = read_file(file->fullpath);
    assert(spec != NULL);

    /* scan all %files sections for any forbidden path references */
    TAILQ_FOREACH(specline, spec, items) {
        /* determine when we enter and leave %files sections */
        if (*specline->data == '%') {
            if (strprefix(specline->data, SPEC_SECTION_FILES)) {
                in_files = true;
            } else if (in_files) {
                /* look for valid macros in the %files section */
                for (i = 0; files_macros[i] != NULL; i++) {
                    if (strprefix(specline->data, files_macros[i])) {
                        valid_macro = true;
                        break;
                    }
                }

                if (!valid_macro && !strprefix(specline->data, "%{")) {
                    in_files = false;
                }
            }
        }

        /* check for forbidden references */
        if (in_files && *specline->data != '#') {
            /* skip if this path is something we should ignore */
            if (ri->inspection_ignores != NULL) {
                HASH_FIND_STR(ri->inspection_ignores, NAME_FILES, mapentry);

                if (mapentry != NULL && mapentry->value != NULL && !TAILQ_EMPTY(mapentry->value)) {
                    TAILQ_FOREACH(path, mapentry->value, items) {
                        if (strprefix(specline->data, path->data)) {
                            ignore = true;
                            break;
                        }
                    }
                }
            }

            if (!ignore) {
                TAILQ_FOREACH(path, ri->forbidden_paths, items) {
                    if (strprefix(specline->data, path->data)) {
                        xasprintf(&params.msg, _("Forbidden path reference (%s) on line %ld of %s"), path->data, line, file->localpath);
                        params.details = specline->data;
                        xasprintf(&noun, _("invalid spec line: %s"), specline->data);
                        params.noun = noun;
                        add_result(ri, &params);
                        free(params.msg);
                        free(noun);
                        result = false;
                    }
                }
            }
        }

        /* increment spec file line counter */
        line++;

        /* reset for next loop iteration--only valid for current line */
        valid_macro = false;
    }

    list_free(spec, free);
    return result;
}

/*
 * Main driver for the 'files' inspection.
 */
bool inspect_files(struct rpminspect *ri)
{
    bool result = false;
    bool src = false;
    struct result_params params;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;

    assert(ri != NULL);

    /* find the after build SRPM */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (headerIsSource(peer->after_hdr) && peer->after_files) {
            /* find the after build spec file */
            TAILQ_FOREACH(file, peer->after_files, items) {
                if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                    src = true;
                    result = files_driver(ri, file);
                    break;
                }
            }
        }
    }

    init_result_params(&params);
    params.header = NAME_FILES;
    params.verb = VERB_OK;

    if (result && src) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!src) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, _("The files inspection is only for source packages, skipping."));
        add_result(ri, &params);
        free(params.msg);
    }

    return result;
}
