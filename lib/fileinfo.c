/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fnmatch.h>
#include <rpm/header.h>
#include "rpminspect.h"

/**
 * @brief Check for the given path on the fileinfo list.  If found,
 * check the st_mode value and report accordingly.
 *
 * @param ri The main struct rpminspect for the program.
 * @param file The file to find on the fileinfo list.
 * @param header The header string to use for results reporting if the
 *               file is found.
 * @param remedy The remedy string to use for results reporting if the
 *               file is found.
 * @return True if the file is on the fileinfo list, false otherwise.
 */
bool match_fileinfo_mode(struct rpminspect *ri, const rpmfile_entry_t *file, const char *header, const char *remedy)
{
    fileinfo_entry_t *fientry = NULL;
    mode_t interesting = S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO;
    mode_t perms = 0;
    const char *pkg = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    perms = file->st.st_mode & interesting;
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    if (remedy) {
        params.remedy = strdup(remedy);
    }

    if (init_fileinfo(ri)) {
        TAILQ_FOREACH(fientry, ri->fileinfo, items) {
            if (!strcmp(file->localpath, fientry->filename)) {
                if (file->st.st_mode == fientry->mode) {
                    xasprintf(&params.msg, _("%s in %s on %s carries expected mode %04o"), file->localpath, pkg, params.arch, perms);
                    params.severity = RESULT_INFO;
                    params.waiverauth = NOT_WAIVABLE;
                    add_result(ri, &params);
                    free(params.msg);
                    free(params.remedy);
                    return true;
                } else {
                    params.severity = get_secrule_result_severity(ri, file, SECRULE_MODES);

                    if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                        params.waiverauth = WAIVABLE_BY_SECURITY;
                        xasprintf(&params.msg, _("%s in %s on %s carries unexpected mode %04o; expected mode %04o; requires inspection by the Security Team"), file->localpath, pkg, params.arch, perms, fientry->mode);
                        add_result(ri, &params);
                        free(params.msg);
                        free(params.remedy);
                        return true;
                    }
                }

                break;
            }
        }
    }

    /* catch anything not on the fileinfo list with setuid/setgid */
    if ((perms & S_ISUID) || (perms & S_ISGID)) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_MODES);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            params.waiverauth = WAIVABLE_BY_SECURITY;
            xasprintf(&params.msg, _("%s in %s on %s carries insecure mode %04o, Security Team review may be required"), file->localpath, pkg, params.arch, perms);
            add_result(ri, &params);
            free(params.msg);
            free(params.remedy);
        }
    }

    return false;
}

/**
 * @brief Check for the given path on the fileinfo list.  If found,
 * check the st_uid value and report accordingly.
 *
 * @param ri The main struct rpminspect for the program.
 * @param file The file to find on the fileinfo list.
 * @param owner The file owner from the RPM header.
 * @param header The header string to use for results reporting if the
 *               file is found.
 * @param remedy The remedy string to use for results reporting if the
 *               file is found.  Must contain %s for fname.
 * @param fname The filename of the data package file to update.
 * @return True if the file is on the fileinfo list, false otherwise.
 */
bool match_fileinfo_owner(struct rpminspect *ri, const rpmfile_entry_t *file, const char *owner, const char *header, const char *remedy, const char *fname)
{
    fileinfo_entry_t *fientry = NULL;
    const char *pkg = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);
    assert(owner != NULL);

    if (remedy) {
        assert(fname != NULL);
    }

    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    if (remedy) {
        xasprintf(&params.remedy, remedy, fname);
    }

    if (init_fileinfo(ri)) {
        TAILQ_FOREACH(fientry, ri->fileinfo, items) {
            if (!strcmp(file->localpath, fientry->filename)) {
                if (!strcmp(owner, fientry->owner)) {
                    xasprintf(&params.msg, _("%s in %s on %s carries expected owner '%s'"), file->localpath, pkg, params.arch, fientry->owner);
                    params.severity = RESULT_INFO;
                    params.waiverauth = NOT_WAIVABLE;
                    add_result(ri, &params);
                    free(params.msg);
                    free(params.remedy);
                    return true;
                } else {
                    params.severity = get_secrule_result_severity(ri, file, SECRULE_MODES);

                    if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                        params.waiverauth = WAIVABLE_BY_SECURITY;
                        xasprintf(&params.msg, _("%s in %s on %s carries unexpected owner '%s'; expected owner '%s'; requires inspection by the Security Team"), file->localpath, pkg, params.arch, owner, fientry->owner);
                        add_result(ri, &params);
                        free(params.msg);
                        free(params.remedy);
                        return true;
                    }
                }

                break;
            }
        }
    }

    free(params.remedy);
    return false;
}

/**
 * @brief Check for the given path on the fileinfo list.  If found,
 * check the st_gid value and report accordingly.
 *
 * @param ri The main struct rpminspect for the program.
 * @param file The file to find on the fileinfo list.
 * @param group The file group from the RPM header.
 * @param header The header string to use for results reporting if the
 *               file is found.
 * @param remedy The remedy string to use for results reporting if the
 *               file is found.  Must contain %s for fname.
 * @param fname The filename of the data package file to update.
 * @return True if the file is on the fileinfo list, false otherwise.
 */
bool match_fileinfo_group(struct rpminspect *ri, const rpmfile_entry_t *file, const char *group, const char *header, const char *remedy, const char *fname)
{
    fileinfo_entry_t *fientry = NULL;
    const char *pkg = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    if (remedy) {
        assert(fname != NULL);
    }

    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);

    init_result_params(&params);
    params.header = header;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    if (remedy) {
        xasprintf(&params.remedy, remedy, fname);
    }

    if (init_fileinfo(ri)) {
        TAILQ_FOREACH(fientry, ri->fileinfo, items) {
            if (!strcmp(file->localpath, fientry->filename)) {
                if (!strcmp(group, fientry->group)) {
                    xasprintf(&params.msg, _("%s in %s on %s carries expected group '%s'"), file->localpath, pkg, params.arch, fientry->group);
                    params.severity = RESULT_INFO;
                    params.waiverauth = NOT_WAIVABLE;
                    add_result(ri, &params);
                    free(params.msg);
                    free(params.remedy);
                    return true;
                } else {
                    params.severity = get_secrule_result_severity(ri, file, SECRULE_MODES);

                    if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                        params.waiverauth = WAIVABLE_BY_SECURITY;
                        xasprintf(&params.msg, _("%s in %s on %s carries group unexpected '%s'; expected group '%s'; requires inspection by the Security Team"), file->localpath, pkg, params.arch, group, fientry->group);
                        add_result(ri, &params);
                        free(params.msg);
                        free(params.remedy);
                        return true;
                    }
                }

                break;
            }
        }
    }

    free(params.remedy);
    return false;
}

/*
 * Return the caps list entry that matches the package and filepath.
 * If it doesn't exist on the list, return NULL.  This function will
 * take care of initializing the caps list if necessary.
 */
caps_filelist_entry_t *get_caps_entry(struct rpminspect *ri, const char *pkg, const char *filepath)
{
    bool found = false;
    int flags = FNM_NOESCAPE | FNM_PATHNAME;
    caps_entry_t *entry = NULL;
    caps_filelist_entry_t *flentry = NULL;

    assert(ri != NULL);
    assert(pkg != NULL);
    assert(filepath != NULL);

    if (init_caps(ri)) {
        /* Look for the package in the caps list */
        TAILQ_FOREACH(entry, ri->caps, items) {
            if (!strcmp(entry->pkg, pkg) || !fnmatch(entry->pkg, pkg, flags)) {
                found = true;
                break;
            }
        }

        if (!found) {
            return NULL;
        }

        found = false;

        /* Look for this file's entry for that package */
        TAILQ_FOREACH(flentry, entry->files, items) {
            if (!strcmp(flentry->path, filepath) || !fnmatch(flentry->path, filepath, flags)) {
                found = true;
                break;
            }
        }

        /* No entry, make sure to return NULL */
        if (!found) {
            return NULL;
        }
    }

    return flentry;
}
