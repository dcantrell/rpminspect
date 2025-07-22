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

static bool reported = false;
static bool rebase = false;

/*
 * Check the given file to see if the path has any forbidden directory in it.
 */
static bool have_forbidden_directory(const rpmfile_entry_t *file, const char *forbidden)
{
    bool r = false;
    char *local = NULL;
    char *full = NULL;
    char *tmp = NULL;
    struct stat sb;

    assert(file != NULL);
    assert(forbidden != NULL);

    /* first see if the actual rpm file is the forbidden directory */
    if (stat(file->fullpath, &sb) == 0 && S_ISDIR(sb.st_mode) && strsuffix(file->localpath, forbidden)) {
        return true;
    }

    /* copy paths for the loop below */
    local = strdup(file->localpath);
    assert(local != NULL);
    full = strdup(file->fullpath);
    assert(full != NULL);

    /* walk the path backwards checking for the forbidden directory */
    while (full && local) {
        /* path component is a directory and is a forbidden one */
        if (stat(full, &sb) == 0 && S_ISDIR(sb.st_mode) && strsuffix(full, forbidden)) {
            r = true;
            break;
        }

        /* back up the path */
        tmp = strrchr(local, PATH_SEP);

        if (tmp == local) {
            break;
        } else {
            *tmp = '\0';
            tmp = strrchr(full, PATH_SEP);
            *tmp = '\0';
        }
    }

    free(local);
    free(full);

    return r;
}

/*
 * Performs all of the tests associated with the addedfiles inspection.
 */
static bool addedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    bool ignore = false;
    const char *name = NULL;
    char *subpath = NULL;
    char *localpath = NULL;
    const char *arch = NULL;
    bool peer_new = false;
    string_entry_t *entry = NULL;
    struct result_params params;

    /* Ignore source RPMs */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip moved files */
    if (file->moved_path && file->peer_file && file->peer_file->moved_path) {
        return true;
    }

    /* Skip debuginfo and debugsource packages */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);

    if (is_debuginfo_rpm(file->rpm_header) || is_debugsource_rpm(file->rpm_header)) {
        return true;
    }

    /*
     * Ignore certain file additions:
     * - Anything in a .build-id/ subdirectory
     * - Any Python egg file ending with .egg-info
     */
    if (strstr(file->localpath, BUILD_ID_DIR) || strsuffix(file->localpath, EGGINFO_FILENAME_EXTENSION)) {
        return true;
    }

    /* We will skip checks for ignored files */
    ignore = ignore_rpmfile_entry(ri, NAME_ADDEDFILES, file);

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up the result parameters */
    init_result_params(&params);
    params.header = NAME_ADDEDFILES;
    params.arch = arch;
    params.file = file->localpath;
    params.remedy = REMEDY_ADDEDFILES;

    if (rebase) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_OK;
    } else {
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
    }

    if (!ignore) {
        /* Check for any forbidden path prefixes */
        if (ri->forbidden_path_prefixes && (ri->tests & INSPECT_ADDEDFILES)) {
            TAILQ_FOREACH(entry, ri->forbidden_path_prefixes, items) {
                subpath = entry->data;
                localpath = file->localpath;

                /* ensure the paths do not start with PATH_SEP */
                if (*subpath == PATH_SEP) {
                    while (*subpath == PATH_SEP) {
                        subpath++;
                    }
                }

                if (*localpath == PATH_SEP) {
                    while (*localpath == PATH_SEP) {
                        localpath++;
                    }
                }

                if (strprefix(localpath, subpath)) {
                    xasprintf(&params.msg, _("Packages should not contain files or directories starting with `%s` on %s in %s: %s"), entry->data, arch, name, file->localpath);
                    params.noun = _("invalid directory ${FILE} on ${ARCH}");
                    add_result(ri, &params);
                    result = !(params.severity >= RESULT_VERIFY);
                    reported = true;
                    goto done;
                }
            }
        }

        /* Check for any forbidden path suffixes */
        if (ri->forbidden_path_suffixes && (ri->tests & INSPECT_ADDEDFILES)) {
            TAILQ_FOREACH(entry, ri->forbidden_path_suffixes, items) {
                if (strsuffix(file->localpath, entry->data)) {
                    xasprintf(&params.msg, _("Packages should not contain files or directories ending with `%s` on %s in %s: %s"), entry->data, arch, name, file->localpath);
                    params.noun = _("invalid directory ${FILE} on ${ARCH}");
                    add_result(ri, &params);
                    result = !(params.severity >= RESULT_VERIFY);
                    reported = true;
                    goto done;
                }
            }
        }

        /* Check for any forbidden directories */
        if (ri->forbidden_directories && (ri->tests & INSPECT_ADDEDFILES)) {
            TAILQ_FOREACH(entry, ri->forbidden_directories, items) {
                if (have_forbidden_directory(file, entry->data)) {
                    xasprintf(&params.msg, _("Forbidden directory `%s` found on %s in %s: %s"), entry->data, arch, name, file->localpath);
                    params.noun = _("forbidden directory ${FILE} on ${ARCH}");
                    add_result(ri, &params);
                    result = !(params.severity >= RESULT_VERIFY);
                    reported = true;
                    goto done;
                }
            }
        }
    }

    /* security path file -- only applicable for build comparisons */
    if ((file->peer_file != NULL && strcmp(file->localpath, file->peer_file->localpath))
        || (ri->before != NULL && file->peer_file == NULL)) {
        peer_new = true;
    } else {
        peer_new = false;
    }

    if (ri->security_path_prefix && S_ISREG(file->st_mode) && peer_new) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            subpath = entry->data;

            while (*subpath != PATH_SEP) {
                subpath++;
            }

            if (strprefix(file->localpath, subpath)) {
                if (rebase) {
                    params.severity = RESULT_INFO;
                    params.waiverauth = NOT_WAIVABLE;
                } else {
                    params.severity = get_secrule_result_severity(ri, file, SECRULE_SECURITYPATH);
                    params.waiverauth = WAIVABLE_BY_SECURITY;
               }

                if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                    xasprintf(&params.msg, _("New security-related file `%s` added on %s in %s requires inspection by the Security Team"), file->localpath, arch, name);
                    params.verb = VERB_ADDED;
                    params.noun = _("new security-related file ${FILE} on ${ARCH}");
                    add_result(ri, &params);
                    result = !(params.severity >= RESULT_VERIFY);
                    reported = true;
                } else {
                    result = true;
                }

                goto done;
            }
        }
    }

    /*
     * Report that a new file has been added in a build comparison.
     */
    if (!ignore && (ri->tests & INSPECT_ADDEDFILES) && (ri->before != NULL && file->peer_file == NULL)) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_OK;
        xasprintf(&params.msg, _("`%s` added on %s in %s"), file->localpath, arch, name);
        params.noun = _("new file ${FILE} on ${ARCH}");
        add_result(ri, &params);
        reported = true;
    }

done:
    free(params.msg);

    return result;
}

bool inspect_addedfiles(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    rebase = is_rebase(ri);
    result = foreach_peer_file(ri, NAME_ADDEDFILES, addedfiles_driver);

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_ADDEDFILES;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
