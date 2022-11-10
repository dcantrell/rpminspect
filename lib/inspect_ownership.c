/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <err.h>

#ifdef _WITH_LIBCAP
#include <sys/capability.h>
#endif

#include "rpminspect.h"

/* Main driver for the 'ownership' inspection */
static bool ownership_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    char *owner = NULL;
    char *group = NULL;
    char *before_owner = NULL;
    char *before_group = NULL;
    string_entry_t *entry = NULL;
    bool bin = false;
    char *before_val = NULL;
    char *after_val = NULL;
    char *what = NULL;
    char *captext = NULL;
    cap_t cap = NULL;
    cap_flag_value_t have_setuid = CAP_CLEAR;
    struct result_params params;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Get the arch, we'll use that */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Get the owner and group of the file */
    owner = get_rpm_header_value(file, RPMTAG_FILEUSERNAME);
    group = get_rpm_header_value(file, RPMTAG_FILEGROUPNAME);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_OWNERSHIP;
    params.arch = arch;
    params.file = file->localpath;

    /*
     * AFTER ONLY
     */

    /* Report forbidden file owners */
    if (ri->forbidden_owners && list_contains(ri->forbidden_owners, owner) && (ri->tests & INSPECT_OWNERSHIP)) {
        xasprintf(&params.msg, _("File %s has forbidden owner `%s` on %s"), file->localpath, owner, arch);
        xasprintf(&params.remedy, REMEDY_OWNERSHIP_DEFATTR, ri->fileinfo_filename);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("forbidden owner for ${FILE} on ${ARCH}");
        add_result(ri, &params);
        free(params.msg);
        free(params.remedy);
        result = false;
    }

    /* Report forbidden file groups */
    if (ri->forbidden_groups && list_contains(ri->forbidden_groups, group) && (ri->tests & INSPECT_OWNERSHIP)) {
        xasprintf(&params.msg, _("File %s has forbidden group `%s` on %s"), file->localpath, owner, arch);
        xasprintf(&params.remedy, REMEDY_OWNERSHIP_DEFATTR, ri->fileinfo_filename);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("forbidden group for ${FILE} on ${ARCH}");
        add_result(ri, &params);
        free(params.msg);
        free(params.remedy);
        result = false;
    }

    /* Report files in bin paths not under the bin owner or group */
    TAILQ_FOREACH(entry, ri->bin_paths, items) {
        if (strprefix(file->localpath, entry->data)) {
            bin = true;

            /* Check the owner */
            if (strcmp(owner, ri->bin_owner) && !match_fileinfo_owner(ri, file, owner, NAME_OWNERSHIP, NULL, NULL) && (ri->tests & INSPECT_OWNERSHIP)) {
                xasprintf(&params.msg, _("File %s has owner `%s` on %s, but should be `%s`"), file->localpath, owner, arch, ri->bin_owner);
                xasprintf(&params.remedy, REMEDY_OWNERSHIP_BIN_OWNER, ri->fileinfo_filename);
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.verb = VERB_FAILED;
                params.noun = _("invalid owner for ${FILE} on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);
                free(params.remedy);
                result = false;
            }

            /* Check the group - special handling */
            if (strcmp(group, ri->bin_group)) {
                /* Gather capabilities(7) for the file we need */
                captext = get_rpm_header_value(file, RPMTAG_FILECAPS);

                if (captext && strcmp(captext, "")) {
                    cap = cap_from_text(captext);
                }

                free(captext);
                captext = NULL;

                if (cap) {
                    if (cap_get_flag(cap, CAP_SETUID, CAP_EFFECTIVE, &have_setuid) == -1) {
                        warnx("cap_get_flag");
                        have_setuid = CAP_CLEAR;
                    }
                }

                /* Handle if CAP_SETUID is present or not */
                if (have_setuid == CAP_SET) {
                    if ((file->st.st_mode & S_IXOTH) && (ri->tests & INSPECT_OWNERSHIP)) {
                        params.severity = get_secrule_result_severity(ri, file, SECRULE_SETUID);

                        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                            xasprintf(&params.msg, _("File %s on %s has CAP_SETUID capability but group `%s` and is world executable"), file->localpath, arch, group);
                            xasprintf(&params.remedy, REMEDY_OWNERSHIP_IXOTH, ri->fileinfo_filename);
                            params.waiverauth = WAIVABLE_BY_SECURITY;
                            params.verb = VERB_FAILED;
                            params.noun = _("CAP_SETUID and o+x for ${FILE} on ${ARCH}");
                            add_result(ri, &params);
                            free(params.msg);
                            free(params.remedy);
                            result = false;
                        }
                    }

                    if (file->st.st_mode & S_IWGRP) {
                        params.severity = get_secrule_result_severity(ri, file, SECRULE_SETUID);

                        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                            xasprintf(&params.msg, _("File %s on %s has CAP_SETUID capability but group `%s` and is group writable"), file->localpath, arch, group);
                            xasprintf(&params.remedy, REMEDY_OWNERSHIP_IWGRP, ri->fileinfo_filename);
                            params.waiverauth = WAIVABLE_BY_SECURITY;
                            params.verb = VERB_FAILED;
                            params.noun = _("CAP_SETUID and g+w for ${FILE} on ${ARCH}");
                            add_result(ri, &params);
                            free(params.msg);
                            free(params.remedy);
                            result = false;
                        }
                    }
                } else if (!match_fileinfo_group(ri, file, group, NAME_OWNERSHIP, NULL, NULL) && (ri->tests & INSPECT_OWNERSHIP)) {
                    xasprintf(&params.msg, _("File %s has group `%s` on %s, but should be `%s`"), file->localpath, group, arch, ri->bin_group);
                    xasprintf(&params.remedy, REMEDY_OWNERSHIP_BIN_GROUP, ri->fileinfo_filename);
                    params.severity = RESULT_BAD;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    params.verb = VERB_FAILED;
                    params.noun = _("invalid group for ${FILE} on ${ARCH}");
                    add_result(ri, &params);
                    free(params.msg);
                    free(params.remedy);
                    result = false;
                }
            }

            break;
        }
    }

    /*
     * BEFORE AND AFTER
     */
    if (file->peer_file && (ri->tests & INSPECT_OWNERSHIP)) {
        /* Get the before file values */
        before_owner = get_rpm_header_value(file->peer_file, RPMTAG_FILEUSERNAME);
        before_group = get_rpm_header_value(file->peer_file, RPMTAG_FILEGROUPNAME);

        /* Determine if anything changed */
        if (strcmp(before_owner, owner)) {
            what = "owner";
            before_val = strdup(before_owner);
            after_val = strdup(owner);
        }

        if (strcmp(before_group, group)) {
            if (what) {
                what = "owner:group";
                free(before_val);
                free(after_val);
                xasprintf(&before_val, "%s:%s", before_owner, before_group);
                xasprintf(&after_val, "%s:%s", owner, group);
            } else {
                what = "group";
                before_val = strdup(before_group);
                after_val = strdup(group);
            }
        }

        /* Report out changes, if necessary */
        if (what) {
            /* Change the severity depending on what happened */
            xasprintf(&params.msg, "%s:%s", owner, group);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.verb = VERB_FAILED;

            if (bin &&
                ((!strcmp(owner, ri->bin_owner) && !strcmp(what, "owner")) ||
                 (!strcmp(group, ri->bin_group) && !strcmp(what, "group")) ||
                 (!strcmp(params.msg, after_val) && !strcmp(what, "owner:group")))) {
                /*
                 * If ownership changed to bin_owner/bin_group for bin files,
                 * just make it an informational message.
                 */
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
            }

            free(params.msg);
            xasprintf(&params.msg, _("File %s changed %s from `%s` to `%s` on %s"), file->localpath, what, before_val, after_val, arch);
            xasprintf(&params.remedy, REMEDY_OWNERSHIP_CHANGED, ri->fileinfo_filename);
            params.noun = _("${FILE} changed owner on ${ARCH}");
            add_result(ri, &params);
            free(params.msg);
            free(params.remedy);
            result = false;
        }

        free(before_owner);
        free(before_group);
        free(before_val);
        free(after_val);
    }

    free(owner);
    free(group);
    return result;
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

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_OWNERSHIP;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
