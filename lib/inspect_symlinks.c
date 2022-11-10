/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_symlinks.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief Symbolic link inspection and helper functions.
 * @copyright LGPL-3.0-or-later
 */

#include <string.h>
#include <assert.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <err.h>
#include "rpminspect.h"

/**
 * @brief Called by the main symlinks inspection driver.
 *
 * If the given file is a symbolic link, perform the following checks:
 * - If there is a peer file, see if it is also a symlink.  If it is a
 *   directory, raise a BAD warning.  If it is anything else other
 *   than a symlink, raise a VERIFY warning.
 * - Try to read the symlink destination.  On error, report the error
 *   as BAD and return false.
 *
 * - Try to find the symlink destination in each peer root.  If none
 *   found, raise a BAD error and return false.
 *
 * @param ri The struct rpminspect pointer for the run of the program
 * @param file The file the function is asked to examine
 * @return True if the file passes, false otherwise
 */
static bool symlinks_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    ssize_t len = 0;
    char localpath[PATH_MAX + 1];
    char linktarget[PATH_MAX + 1];
    char reltarget[PATH_MAX + 1];
    char *target = NULL;
    int linkerr = 0;
    char cwd[PATH_MAX + 1];
    bool found = false;
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    char *tmp = NULL;
    char *tail = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip debug paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* only applies to symbolic links */
    if (!S_ISLNK(file->st.st_mode)) {
        return true;
    }

    /* name and architecture is used in reporting */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    arch = get_rpm_header_arch(file->rpm_header);

    /* initialize the result parameters */
    init_result_params(&params);
    params.header = NAME_SYMLINKS;
    params.remedy = REMEDY_SYMLINKS;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    /* first, check for things becoming symlinks to guard RPM */
    if (file->peer_file && !S_ISLNK(file->peer_file->st.st_mode)) {
        if (S_ISDIR(file->peer_file->st.st_mode)) {
            /* Some RPM versions cannot handle this on an upgrade */
            params.remedy = REMEDY_SYMLINKS_DIRECTORY;
            xasprintf(&params.msg, _("Directory %s became a symbolic link (to %s) in %s on %s; this is not allowed!"), file->peer_file->localpath, file->localpath, name, arch);
        } else {
            xasprintf(&params.msg, _("%s %s became a symbolic link (to %s) in %s on %s"), strtype(file->peer_file->st.st_mode), file->peer_file->localpath, file->localpath, name, arch);
        }

        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_CHANGED;
        params.noun = _("${FILE} became a symlink on ${ARCH}");
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    /* get the target */
    len = readlink(file->fullpath, linktarget, sizeof(linktarget) - 1);
    linktarget[len] = '\0';
    target = linktarget;

    if (len == -1) {
        /* a read error on the link here prevents further analysis */
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.details = strerror(errno);
        params.verb = VERB_FAILED;
        params.noun = _("unable to read symlink ${FILE} on ${ARCH}");
        xasprintf(&params.msg, _("An error occurred reading symbolic link %s in %s on %s."), file->localpath, name, arch);
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /* save current directory */
    memset(cwd, '\0', sizeof(cwd));

    if (getcwd(cwd, PATH_MAX) == NULL) {
        err(RI_PROGRAM_ERROR, "getcwd");
    }

    memset(reltarget, '\0', sizeof(reltarget));

    /*
     * try to find the link destination
     * Handle absolute symlinks differently so we can check each subpackage.
     * Relative symlinks will be canonicalized in each subpackage.
     */
    if (*target == '/') {
        /* link is absolute */
        /*
         * Trim leading slashes so readlink() can work from the subpackage
         * root directory.
         */
        while (*target == '/' && *target != '\0') {
            target++;
        }

        tail = stpcpy(reltarget, target);
    } else {
        /* link is relative */
        /*
         * Canonicalize the link without looking at the filesystem.  Trim
         * the target of leading slashes.
         */
        tmp = strncpy(localpath, file->localpath, PATH_MAX);
        assert(tmp != NULL);
        tail = stpcpy(reltarget, dirname(localpath));
        assert(reltarget != NULL);
        DEBUG_PRINT("    reltarget=|%s|, target=|%s|\n", reltarget, target);

        while (target && *target != '\0') {
            if (*target == '/') {
                /* ignore the leading slash */
                target++;
            } else if (strprefix(target, "./")) {
                /* skip current directory references */
                target += 2;
            } else if (strprefix(target, "../") && !strcmp(reltarget, "")) {
                /* relative symlink cannot be resolved */
                xasprintf(&params.msg, _("%s %s has too many levels of redirects and cannot be resolved in %s on %s"), strtype(file->st.st_mode), file->localpath, name, arch);
                xasprintf(&params.details, "%s -> %s", file->localpath, linktarget);
                params.severity = RESULT_VERIFY;
                params.verb = VERB_FAILED;
                params.noun = _("too many redirects for ${FILE} on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);
                free(params.details);

                if (chdir(cwd) == -1) {
                    warn("%s: chdir to %s", __func__, cwd);
                }

                return false;
            } else if (strprefix(target, "../")) {
                /* back up a directory level */
                tmp = rindex(reltarget, '/');
                assert(tmp != NULL);
                *tmp = '\0';
                tail = tmp;
                target += 3;
                DEBUG_PRINT("    reltarget=|%s|, target=|%s|\n", reltarget, target);
            } else {
                /* copy the path segment we are currently looking at */
                /*
                 * for example if target is "usr/lib64/libexcitement.so.47" then
                 * we want to copy just "usr/" to reltarget and advance target
                 * so it looks like "lib64/libexcitement.so.47"
                 */

                /*
                 * Find the directory separator and make it end-of-string.
                 * Now 'target' points to the first directory element.
                 * Take care to account for the final path element
                 * where there is no '/' so index() will give us NULL.
                 */
                tmp = index(target, '/');

                if (tmp != NULL) {
                    *tmp = '\0';
                }

                /* make sure a slash is present if appending final element */
                if (!strsuffix(reltarget, "/") && !strprefix(target, "/")) {
                    tail = stpcpy(tail, "/");
                    assert(tail != NULL);
                }

                /* append the path element to reltarget */
                tail = stpcpy(tail, target);
                assert(tail != NULL);
                DEBUG_PRINT("    reltarget=|%s|\n", reltarget);

                /* advance target past the first directory element and '/' */
                target += strlen(target);

                if (tmp != NULL) {
                    target++;

                    /* add a slash to the end */
                    tail = stpcpy(tail, "/");
                    assert(tail != NULL);
                }

                DEBUG_PRINT("    tmp=|%s|, reltarget=|%s|, target=|%s|\n", tmp, reltarget, target);
            }
        }
    }

    target = reltarget;

    /* drop the leading slashes */
    while (*target == '/' && *target != '\0') {
        target++;
    }

    DEBUG_PRINT("final target=|%s|\n", target);

    /* look for the symlink in each root */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* move to the subpackage root */
        if (chdir(peer->after_root) == -1) {
            continue;
        }

        /* try to access the target */
        if (access(target, R_OK) == 0) {
            found = true;
            break;
        }

        if (errno == ELOOP || errno == ENAMETOOLONG) {
            /* save interesting symlink errors */
            linkerr = errno;
        }
    }

    /* not found?  report */
    if (!found) {
        xasprintf(&params.msg, _("%s %s %s a dangling symbolic link in %s on %s"), strtype(file->st.st_mode), file->localpath, (file->peer_file) ? "became" : "is", name, arch);

        if (linkerr == ELOOP || linkerr == ENAMETOOLONG) {
            params.severity = RESULT_BAD;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.details = strerror(linkerr);
            params.verb = VERB_FAILED;
            params.noun = _("dangling symlink ${FILE} on ${ARCH}");
            result = false;
        } else {
            /* XXX - try to find a way to find link destinations in
               Require'd packages (#145); report as INFO for now */
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
            result = true;
        }

        add_result(ri, &params);
        free(params.msg);
    }

    /* return to D-station */
    if (chdir(cwd) == -1) {
        warn("chdir");
    }

    return result;
}

/**
 * @brief Main driver for the 'symlinks' inspection.
 *
 * Check for dangling symlinks as well as symlinks that present issues
 * like ELOOP and ENAMETOOLONG.  Guard against directories becoming
 * symlinks, which is a limitation of RPM.
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_symlinks(struct rpminspect *ri)
{
    bool result = true;
    struct result_params params;

    assert(ri != NULL);
    result = foreach_peer_file(ri, NAME_SYMLINKS, symlinks_driver);

    if (result) {
        init_result_params(&params);
        params.header = NAME_SYMLINKS;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
