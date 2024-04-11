/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool rebase = false;
static bool reported = false;

/*
 * Performs all of the tests associated with the removedfiles inspection.
 * NOTE:  This function is called while looping over before_files.
 */
static bool removedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *type = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    char *soname = NULL;
    string_entry_t *entry = NULL;
    security_entry_t *sentry = NULL;
    struct result_params params;

    /* Any entry with a peer has not been removed. */
    if (file->peer_file) {
        return true;
    }

    /* Skip moved files */
    if (file->moved_path && (file->peer_file && file->peer_file->moved_path)) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /*
     * Ignore certain file removals:
     * - Anything in a .build-id/ subdirectory
     * - Any Python egg file ending with .egg-info
     */
    if (strstr(file->localpath, BUILD_ID_DIR) ||
        strsuffix(file->localpath, EGGINFO_FILENAME_EXTENSION)) {
        return true;
    }

    /* Collect the RPM architecture and file MIME type */
    type = get_mime_type(ri, file);
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    arch = get_rpm_header_arch(file->rpm_header);

    /* Get any possible security rule for this path */
    sentry = get_secrule_by_path(ri, file);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_REMOVEDFILES;
    params.arch = arch;
    params.file = file->localpath;
    params.noun = _("library ${FILE} removed on ${ARCH}");

    /* Set the waiver type if this is a file of security concern */
    if (ri->security_path_prefix) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            while (*entry->data != '/') {
                entry->data++;
            }

            if (strprefix(file->localpath, entry->data)) {
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_FAILED;
                break;
            }
        }
    }

    /*
     * File has been removed, report results.
     */
    if (params.waiverauth == WAIVABLE_BY_SECURITY || (ri->tests & INSPECT_REMOVEDFILES)) {
        params.remedy = get_remedy(REMEDY_REMOVEDFILES);

        if (sentry || params.waiverauth == WAIVABLE_BY_SECURITY) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_SECURITYPATH);
        } else {
            if (rebase) {
                params.severity = RESULT_INFO;
            } else {
                params.severity = RESULT_VERIFY;
            }
        }

        if (params.severity <= RESULT_INFO) {
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
        } else {
            if (sentry) {
                params.waiverauth = WAIVABLE_BY_SECURITY;
            } else if ((sentry == NULL) && (params.waiverauth == NULL_WAIVERAUTH)) {
                params.waiverauth = WAIVABLE_BY_ANYONE;
            }

            params.verb = VERB_FAILED;
        }

        if (is_elf_file(file) && !strcmp(type, "application/x-pie-executable")) {
            soname = get_elf_soname(file);

            if (soname) {
                if (params.waiverauth == WAIVABLE_BY_SECURITY) {
                    xasprintf(&params.msg, _("ABI break: Library %s with SONAME '%s' removed from package %s on %s; Removing security policy related files requires inspection by the Security Response Team."), file->localpath, soname, name, arch);

                } else {
                    xasprintf(&params.msg, _("ABI break: Library %s with SONAME '%s' removed from package %s on %s"), file->localpath, soname, name, arch);
                }

                params.noun = _("missing SONAME in ${FILE} on ${ARCH}");
                free(soname);
            } else {
                if (params.waiverauth == WAIVABLE_BY_SECURITY) {
                    xasprintf(&params.msg, _("ABI break: Library %s removed from package %s on %s; Removing security policy related files requires inspection by the Security Response Team."), file->localpath, name, arch);
                } else {
                    xasprintf(&params.msg, _("ABI break: Library %s removed from package %s on %s"), file->localpath, name, arch);
                }
            }
        } else {
            if (params.waiverauth == WAIVABLE_BY_SECURITY) {
                xasprintf(&params.msg, _("%s removed from package %s on %s; Removing security policy related files requires inspection by the Security Response Team."), file->localpath, name, arch);
            } else {
                xasprintf(&params.msg, _("%s removed from package %s on %s"), file->localpath, name, arch);
            }
        }

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            add_result(ri, &params);
            reported = true;
            result = !(params.severity >= RESULT_VERIFY);
        }

        free(params.msg);
    }

    return result;
}

bool inspect_removedfiles(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct result_params params;

    assert(ri != NULL);

    /* is this a rebase comparison? */
    rebase = is_rebase(ri);

    /*
     * This is like our after_files loop helper in inspect.c, but
     * run the loop over the before_files.  This is because we want
     * to check for removed files which is easily detected by a
     * missing peer_file on the before_files list.
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Skip source RPMs */
        if (headerIsSource(peer->before_hdr)) {
            continue;
        }

        /* Skip debuginfo and debugsource packages */
        if (is_debuginfo_rpm(peer->before_hdr) || is_debugsource_rpm(peer->before_hdr)) {
            continue;
        }

        /* If there are no peer files */
        if (peer->before_files == NULL) {
            continue;
        }

        /* Iterate over all files in the before package */
        TAILQ_FOREACH(file, peer->before_files, items) {
            if (!removedfiles_driver(ri, file)) {
                result = false;
            }
        }
    }

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_REMOVEDFILES;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
