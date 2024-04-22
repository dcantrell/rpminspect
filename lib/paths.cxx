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

/**
 * @brief Return the selected build debuginfo package path where the
 * package was extracted for rpminspect.  The path must match the
 * architecture provided.
 *
 * IMPORTANT: Do not free the returned string.
 *
 * @param ri The struct rpminspect for the program.
 * @param file The file we are looking for debuginfo for.
 * @param binarch The required debuginfo architecture.
 * @return Full path to the extracted debuginfo package, or
 *         NULL if not found.
 */
const char *get_debuginfo_path(struct rpminspect *ri, const rpmfile_entry_t *file, const char *binarch, int build)
{
    char *r = NULL;
    rpmpeer_entry_t *peer = NULL;
    rpmpeer_entry_t *safety = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    char *root = NULL;
    char *pattern = NULL;
    char *check = NULL;
    char *tmp = NULL;
    rpmfile_t *files = NULL;
    rpmfile_entry_t *pfile = NULL;
    unsigned int count = 0;
    struct stat sb;
    Header peer_rpm_header = NULL;

    assert(ri != NULL);
    assert(file != NULL);
    assert(binarch != NULL);
    assert(build == BEFORE_BUILD || build == AFTER_BUILD);

    /* debuginfo base pattern */
    arch = get_rpm_header_arch(file->rpm_header);

    if (!fnmatch(RPM_X86_ARCH_PATTERN, arch, 0)) {
        arch = RPM_X86_ARCH_PATTERN;
    }

    xasprintf(&pattern, "%s-%s-%s.%s%s", file->localpath,
                                         headerGetString(file->rpm_header, RPMTAG_VERSION),
                                         headerGetString(file->rpm_header, RPMTAG_RELEASE),
                                         arch,
                                         DEBUG_FILE_SUFFIX);
    assert(pattern != NULL);

    /* try to find a debuginfo package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if ((build == BEFORE_BUILD && peer->before_hdr == NULL) || (build == AFTER_BUILD && peer->after_hdr == NULL)) {
            continue;
        }

        if (build == BEFORE_BUILD) {
            arch = get_rpm_header_arch(peer->before_hdr);
            root = peer->before_root;
            files = peer->before_files;
        } else {
            arch = get_rpm_header_arch(peer->after_hdr);
            root = peer->after_root;
            files = peer->after_files;
        }

        assert(arch != NULL);

        if (strcmp(arch, binarch)) {
            /* not the same architecture */
            continue;
        }

        if (build == BEFORE_BUILD) {
            name = headerGetString(peer->before_hdr, RPMTAG_NAME);
            peer_rpm_header = peer->before_hdr;
        } else {
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);
            peer_rpm_header = peer->after_hdr;
        }

        assert(name != NULL);

        /* found debuginfo package */
        if (is_debuginfo_rpm(peer_rpm_header)) {
            count++;

            /* used for older systems that generate single debuginfo packages */
            if (safety == NULL) {
                safety = peer;
            }

            /* create a full pattern for matching */
            tmp = pattern;

            if (strsuffix(DEBUG_PATH, "/")) {
                while (*tmp == '/' && *tmp != '\0') {
                    tmp++;
                }
            }

            xasprintf(&check, "%s%s%s", root, DEBUG_PATH, tmp);
            assert(check != NULL);

            TAILQ_FOREACH(pfile, files, items) {
                if (fnmatch(check, pfile->fullpath, 0)) {
                    continue;
                } else {
                    if (stat(pfile->fullpath, &sb) == 0 && S_ISREG(sb.st_mode)) {
                        /* just copy the pointer, no need to dupe it */
                        if (build == BEFORE_BUILD) {
                            r = peer->before_root;
                        } else {
                            r = peer->after_root;
                        }

                        break;
                    }
                }
            }

            free(check);

            if (r) {
                break;
            }
        }

        if (r) {
            break;
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

    free(pattern);
    return r;
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
        warn("*** lstat");
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
    int flags = FNM_NOESCAPE | FNM_PATHNAME;
    int gflags = GLOB_NOSORT;
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

    /* Try a simple glob match */
    if (!fnmatch(pattern, path, flags)) {
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

#ifdef GLOB_PERIOD
    /* this is a GNU extension, see glob(3) */
    gflags |= GLOB_PERIOD;
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

    free(n);

    if (!strsuffix(globpath, "/") && !strprefix(pattern, "/")) {
        gp = stpcpy(gp, "/");
        len++;
    }

    (void) stpcpy(gp, pattern);
    r = glob(globpath, gflags, NULL, &found);

    if (r == GLOB_NOSPACE || r == GLOB_ABORTED) {
        warn("*** glob");
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
    assert(inspection != NULL);

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

/**
 * Given an rpmfile_entry_t and an inspection name, determine if it
 * should be ignored based on the current configuration rules.
 *
 * @param ri The struct rpminspect for the program.
 * @param inspection The name of the inspection currently running.
 * @param file The rpmfile_entry_t to check.
 * @return True if path should be ignored, false otherwise.
 */
bool ignore_rpmfile_entry(const struct rpminspect *ri, const char *inspection, const rpmfile_entry_t *file)
{
    bool ignore = false;
    char *head = NULL;
    char save;

    assert(ri != NULL);
    assert(inspection != NULL);
    assert(file != NULL);

    head = file->fullpath;
    head += strlen(head) - strlen(file->localpath);
    save = *head;
    *head = '\0';

    ignore = ignore_path(ri, inspection, file->localpath, file->fullpath);

    *head = save;
    head = NULL;

    return ignore;
}
