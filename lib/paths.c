/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <assert.h>
#include <err.h>
#include <glob.h>
#include <fnmatch.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * Helper for the debuginfo path functions.
 */
static const char *_get_debuginfo_path_helper(struct rpminspect *ri, const rpmfile_entry_t *file, const char *binarch, int build)
{
    char *r = NULL;
    rpmpeer_entry_t *peer = NULL;
    rpmpeer_entry_t *safety = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    char *root = NULL;
    char *base = NULL;
    char *check = NULL;
    unsigned int count = 0;
    struct stat sb;

    assert(ri != NULL);
    assert(file != NULL);
    assert(binarch != NULL);
    assert(build == BEFORE_BUILD || build == AFTER_BUILD);

    /* debuginfo base file */
    xasprintf(&base, "%s-%s-%s.%s%s", file->localpath,
                                      headerGetString(file->rpm_header, RPMTAG_VERSION),
                                      headerGetString(file->rpm_header, RPMTAG_RELEASE),
                                      get_rpm_header_arch(file->rpm_header),
                                      DEBUG_FILE_SUFFIX);
    assert(base != NULL);

    /* try to find a debuginfo package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if ((build == BEFORE_BUILD && peer->before_hdr == NULL) || (build == AFTER_BUILD && peer->after_hdr == NULL)) {
            continue;
        }

        if (build == BEFORE_BUILD) {
            arch = get_rpm_header_arch(peer->before_hdr);
            root = peer->before_root;
        } else {
            arch = get_rpm_header_arch(peer->after_hdr);
            root = peer->after_root;
        }

        assert(arch != NULL);

        if (strcmp(arch, binarch)) {
            /* not the same architecture */
            continue;
        }

        if (build == BEFORE_BUILD) {
            name = headerGetString(peer->before_hdr, RPMTAG_NAME);
        } else {
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);
        }

        assert(name != NULL);

        /* found debuginfo package */
        if (strsuffix(name, DEBUGINFO_SUFFIX)) {
            count++;

            /* used for older systems that generate single debuginfo packages */
            if (safety == NULL) {
                safety = peer;
            }

            /* look for the debuginfo package here */
            check = joinpath(root, DEBUG_PATH, base, NULL);

            if (stat(check, &sb) == -1 || !S_ISREG(sb.st_mode)) {
                /* file not in this package */
                free(check);
                continue;
            }

            free(check);

            /* just copy the pointer, no need to dupe it */
            if (build == BEFORE_BUILD) {
                r = peer->before_root;
            } else {
                r = peer->after_root;
            }

            if (S_ISREG(sb.st_mode)) {
                break;
            }
        }
    }

    /* older systems used to generate a single debuginfo package */
    if (count == 1 && r == NULL) {
        if (build == BEFORE_BUILD) {
            r = safety->before_root;
        } else {
            r = safety->after_root;
        }
    }

    free(base);
    return r;
}

/**
 * @brief Return the before build debuginfo package path where the
 * package was extracted for rpminspect.  The path must match the
 * architecture provided.
 *
 * IMPORTANT: Do not free the returned string.
 *
 * @param ri The struct rpminspect for the program.
 * @param file The file we are looking for debuginfo for.
 * @param binarch The required debuginfo architecture.
 * @return Full path to the extract before build debuginfo package, or
 * NULL if not found.
 */
const char *get_before_debuginfo_path(struct rpminspect *ri, const rpmfile_entry_t *file, const char *binarch)
{
    return _get_debuginfo_path_helper(ri, file, binarch, BEFORE_BUILD);
}

/**
 * @brief Return the after build debuginfo package path where the
 * package was extracted for rpminspect.  The path must match the
 * architecture provided.
 *
 * IMPORTANT: Do not free the returned string.
 *
 * @param ri The struct rpminspect for the program.
 * @param file The file we are looking for debuginfo for.
 * @param binarch The required debuginfo architecture.
 * @return Full path to the extract after build debuginfo package for
 * the name subpackage, or NULL if not found.
 */
const char *get_after_debuginfo_path(struct rpminspect *ri, const rpmfile_entry_t *file, const char *binarch)
{
    return _get_debuginfo_path_helper(ri, file, binarch, AFTER_BUILD);
}

/*
 * Checks to see if the given path is a readable directory.
 */
bool usable_path(const char *path)
{
    struct stat sb;

    if (path == NULL) {
        return false;
    }

    if (access(path, R_OK) == -1) {
        return false;
    }

    memset(&sb, 0, sizeof(sb));

    if (lstat(path, &sb) == -1) {
        warn("lstat");
        return false;
    }

    if (!S_ISDIR(sb.st_mode)) {
        return false;
    }

    return true;
}

/*
 * Helper function for glob(7) matching given a path string and
 * (optional) root directory.
 */
bool match_path(const char *pattern, const char *root, const char *path)
{
    bool match = false;
    int r = 0;
    int gflags = GLOB_NOSORT | GLOB_PERIOD;
    char globpath[PATH_MAX + 1];
    char *globsub = NULL;
    char *gp = globpath;
    glob_t found;
    size_t len = 0;
    size_t i = 0;
    char *n = NULL;

    assert(pattern != NULL);
    assert(path != NULL);

    /* Simple check first */
    if (!strcmp(pattern, path)) {
        return true;
    }

    /* A pattern ending with '/' will match a path prefix */
    if (strsuffix(pattern, "/") && strprefix(path, pattern)) {
        return true;
    }

    /*
     * Also handle the incredibly common case of the trailing '/'
     * where users specify an asterisk after the slash to mean
     * everything below this directory.
     */
    if (strsuffix(pattern, "/*")) {
        globsub = strdup(pattern);
        assert(globsub != NULL);
        globsub[strlen(globsub) - 1] = '\0';

        if (globsub && strprefix(path, globsub)) {
            free(globsub);
            return true;
        }

        free(globsub);
    }

    /* Try a match on the leading subdirectory */
    if ((strsuffix(pattern, "*") || strsuffix(pattern, "?")) && !fnmatch(pattern, path, FNM_LEADING_DIR)) {
        return true;
    }

    /* Fall through to glob(3) matching */
#ifdef GLOB_BRACE
    /* this is a GNU extension, see glob(3) */
    gflags |= GLOB_BRACE;
#endif

    n = strdup(path);
    assert(n != NULL);

    if (root != NULL) {
        len = strlen(root);
    }

    memset(globpath, 0, sizeof(globpath));

    if (root != NULL) {
        gp = stpcpy(gp, root);
    }

    if (!strprefix(pattern, "/")) {
        gp = stpcpy(gp, dirname(n));
    }

    free(n);

    if (!strsuffix(globpath, "/") && !strprefix(pattern, "/")) {
        gp = stpcpy(gp, "/");
    }

    gp = stpcpy(gp, pattern);
    r = glob(globpath, gflags, NULL, &found);

    if (r == GLOB_NOSPACE || r == GLOB_ABORTED) {
        warn("glob");
    }

    if (r != 0) {
        return false;
    }

    for (i = 0; i < found.gl_pathc; i++) {
        globsub = found.gl_pathv[i] + len;

        if (!strcmp(globsub, path)) {
            match = true;
            break;
        }
    }

    globfree(&found);

    return match;
}

/**
 * @brief Given a path and struct rpminspect, determine if the path
 * should be ignored or not.
 *
 * @param ri The struct rpminspect for the program.
 * @param inspection The name of the inspection currently running.
 * @param path The relative path to check (i.e., localpath).
 * @param root The root directory, optional (pass NULL to use '/').
 * @return True if path should be ignored, false otherwise.
 */
bool ignore_path(const struct rpminspect *ri, const char *inspection, const char *path, const char *root)
{
    bool match = false;
    string_entry_t *entry = NULL;
    string_list_map_t *mapentry = NULL;

    assert(ri != NULL);

    if (path == NULL) {
        return true;
    }

    /* first, handle the global ignores */
    if (ri->ignores != NULL && !TAILQ_EMPTY(ri->ignores)) {
        TAILQ_FOREACH(entry, ri->ignores, items) {
            match = match_path(entry->data, root, path);

            if (match) {
                return match;
            }
        }
    }

    /* second, handle the per-inspection ignores */
    if (ri->inspection_ignores != NULL) {
        HASH_FIND_STR(ri->inspection_ignores, inspection, mapentry);

        if (mapentry != NULL && mapentry->value != NULL && !TAILQ_EMPTY(mapentry->value)) {
            TAILQ_FOREACH(entry, mapentry->value, items) {
                match = match_path(entry->data, root, path);

                if (match) {
                    return match;
                }
            }
        }
    }

    return match;
}
