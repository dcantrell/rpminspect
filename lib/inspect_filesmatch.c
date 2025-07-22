/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <rpm/rpmmacro.h>

#include "rpminspect.h"

/* Globals */
static bool result = true;
static string_hash_t *files_globs = NULL;
static string_hash_t *files_dirs = NULL;
static string_hash_t *files_excludes = NULL;

enum {
    FILES_GLOBS = 0,
    FILES_DIRS = 1,
    FILES_EXCLUDES = 2
};

/* Helper function to save strings in a specified hash table */
static void save_pathspec(const char *s, const int type)
{
    string_hash_t *hash = NULL;
    string_hash_t *entry = NULL;

    assert(s != NULL);

    /* sometimes we end up with empty strings, ignore */
    if (strlen(s) == 0) {
        return;
    }

    /* determine which hash table to use */
    if (type == FILES_GLOBS) {
        hash = files_globs;
    } else if (type == FILES_DIRS) {
        hash = files_dirs;
    } else if (type == FILES_EXCLUDES) {
        hash = files_excludes;
    }

    /* look for the string first */
    HASH_FIND_STR(hash, s, entry);

    /* if we don't have the string yet, add it to the hash table */
    if (entry == NULL) {
        entry = xalloc(sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(s);
DEBUG_PRINT("pathspec: |%s|\n", entry->data);
        HASH_ADD_KEYPTR(hh, hash, entry->data, strlen(entry->data), entry);
    }

    return;
}

/*
 * Read all of the %files section in spec files and gather the
 * macro-expanded entries.
 */
static void gather_files_entries(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_list_t *speclines = NULL;
    string_entry_t *line = NULL;
    string_list_t *filenames = NULL;
    string_entry_t *filename = NULL;
    bool found = false;
    const char *name = NULL;
    char *pathspec = NULL;
    char *expanded = NULL;
    char *entry = NULL;
    char *prefix = NULL;
    int type = FILES_GLOBS;

    assert(ri != NULL);

    /* Build the file glob list first */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        name = headerGetString(peer->after_hdr, RPMTAG_NAME);
DEBUG_PRINT("package: %s-%s-%s\n", name, headerGetString(peer->after_hdr, RPMTAG_VERSION), headerGetString(peer->after_hdr, RPMTAG_RELEASE));

        /*
         * Iterate over the file list of the SRPM and read the spec file.
         * There should only be one spec file, but if there is a future
         * with multiple spec files in a single SRPM then I sure hope to
         * be a USCG licensed captain by then.
         */
        TAILQ_FOREACH(file, peer->after_files, items) {
            /* skip all source files except the spec file */
            if (!strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                continue;
            }

            /* read in the spec file */
            speclines = read_file(file->fullpath);

            if (speclines == NULL) {
                continue;
            }

            /* read in all of the glob(7) lines from %files sections */
            TAILQ_FOREACH(line, speclines, items) {
                line->data = strtrim(line->data);
                assert(line->data != NULL);
                type = FILES_GLOBS;

                /* skip empty lines */
                if (!strcmp(line->data, "")) {
                    continue;
                }

                /* when found is true, we are reading %files entries */
                if (found) {
                    /* look for %files sections and skip other sections */
                    if (strprefix(line->data, SPEC_SECTION_FILES)) {
                        /* a subpackage %files section, keep reading */
                        continue;
                    } else if (strprefix(line->data, SPEC_MACRO_IF) ||
                               strprefix(line->data, SPEC_MACRO_ELSE) ||
                               strprefix(line->data, SPEC_MACRO_ENDIF)) {
                        /* do not read conditional macros as file globs */
                        continue;
                    } else if (strprefix(line->data, SPEC_SECTION_DESCRIPTION) ||
                               strprefix(line->data, SPEC_SECTION_PACKAGE) ||
                               strprefix(line->data, SPEC_SECTION_PREP) ||
                               strprefix(line->data, SPEC_SECTION_BUILD) ||
                               strprefix(line->data, SPEC_SECTION_INSTALL) ||
                               strprefix(line->data, SPEC_SECTION_CHECK) ||
                               strprefix(line->data, SPEC_SECTION_PRE) ||
                               strprefix(line->data, SPEC_SECTION_PREUN) ||
                               strprefix(line->data, SPEC_SECTION_POST) ||
                               strprefix(line->data, SPEC_SECTION_POSTUN) ||
                               strprefix(line->data, SPEC_SECTION_TRIGGERUN) ||
                               strprefix(line->data, SPEC_SECTION_CHANGELOG)) {
                        /* we reached the end of the %files section(s) */
                        found = false;
                        break;
                    } else if (strprefix(line->data, SPEC_SECTION_FILES)) {
                        /* a subpackage %files section, keep reading */
                        continue;
                    } else if (strprefix(line->data, SPEC_MACRO_IF) ||
                               strprefix(line->data, SPEC_MACRO_ELSE) ||
                               strprefix(line->data, SPEC_MACRO_ENDIF)) {
                        /* do not read conditional macros as file globs */
                        continue;
                    }

                    /* we have a glob(7) specification */
                    pathspec = line->data;

                    /* take care of any macros leading the %files entry line */
                    if (strprefix(pathspec, SPEC_FILES_DOC)) {
                        /* handle %doc */
                        pathspec = strchr(pathspec, ' ');
                        xasprintf(&prefix, "%%{_defaultdocdir}/%s", name);
                        assert(prefix != NULL);
                    } else if (strprefix(pathspec, SPEC_FILES_LICENSE)) {
                        /* handle %licensedir */
                        pathspec = strchr(pathspec, ' ');
                        xasprintf(&prefix, "%%{_defaultlicensedir}/%s", name);
                        assert(prefix != NULL);
                    } else if (strstr(pathspec, SPEC_FILES_GHOST)) {
                        /* skip any entries with %ghost */
                        continue;
                    } else if (*pathspec == '#') {
                        /* skip comments */
                        continue;
                    } else if (strprefix(pathspec, SPEC_FILES_DIR)) {
                        /*
                         * this is a %dir specification so we
                         * record it in a different hash table
                         * because checking actual files in the
                         * binary RPMs will be checking path
                         * prefixes against these vs glob matching
                         */
                        type = FILES_DIRS;
                        pathspec = strchr(pathspec, ' ');
                    } else if (strprefix(pathspec, SPEC_FILES_EXCLUDE)) {
                        /*
                         * this is an %exclude specification which
                         * is a known glob that *SHOULD NOT* be
                         * present in built RPMs
                         */
                        type = FILES_EXCLUDES;
                        pathspec = strchr(pathspec, ' ');
                    } else {
                        /* skip past %attr, %config, %verify */
                        while (pathspec != NULL &&
                               (strprefix(pathspec, SPEC_FILES_ATTR) ||
                                strprefix(pathspec, SPEC_FILES_CONFIG) ||
                                strprefix(pathspec, SPEC_FILES_VERIFY))) {
                            pathspec = strchr(pathspec, ' ');

                            /* advance the string up to the actual path spec */
                            while (pathspec != NULL && isspace(*pathspec)) {
                                pathspec++;
                            }
                        }
                    }

                    if (prefix) {
                        filenames = strsplit(pathspec, " \t");

                        if (filenames != NULL) {
                            TAILQ_FOREACH(filename, filenames, items) {
                                /* build this entry's path spec */
                                entry = joinpath(prefix, filename->data, NULL);
                                assert(entry != NULL);

                                /* expand macros */
                                expanded = rpmExpand(entry, NULL);

                                /* save this glob in the right hash table */
                                save_pathspec(expanded, type);

                                /* cleanup */
                                free(expanded);
                                free(entry);
                            }

                            list_free(filenames, free);
                        }

                        free(prefix);
                        prefix = NULL;
                    } else {
                        /* trim and leading or trailing whitespace */
                        pathspec = strtrim(pathspec);

                        if (pathspec == NULL) {
                            /* no reason to continue if there's nothing here */
                            continue;
                        }

                        /* expand macros */
                        expanded = rpmExpand(pathspec, NULL);

                        /* save this glob in the right hash table */
                        save_pathspec(expanded, type);

                        /* cleanup */
                        free(expanded);
                    }
                } else if (!found && strprefix(line->data, SPEC_SECTION_FILES)) {
                    /* we found %files, start reading lines */
                    found = true;
                    continue;
                }
            }

            list_free(speclines, free);
        }
    }

    return;
}

/*
 * Main driver for the 'filesmatch' inspection.
 */
bool inspect_filesmatch(struct rpminspect *ri)
{
    struct result_params params;

    assert(ri != NULL);

    /* Read in all of the %files entries */
    gather_files_entries(ri);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_FILESMATCH;
    params.verb = VERB_OK;

    /* Report a single OK message if everything was fine */
    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    /* cleanup */
    free_string_hash(files_dirs);
    free_string_hash(files_globs);
    free_string_hash(files_excludes);

    return result;
}
