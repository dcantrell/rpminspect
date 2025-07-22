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

/*
 * Main driver for the 'filesmatch' inspection.
 */
bool inspect_filesmatch(struct rpminspect *ri)
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
    struct result_params params;

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

                if (found) {
                    if (strprefix(line->data, SPEC_SECTION_CHANGELOG)) {
                        break;
                    } else if (strprefix(line->data, SPEC_SECTION_FILES)) {
                        /* a subpackage %files section, keep reading */
                        continue;
                    } else if (strprefix(line->data, SPEC_MACRO_IF) || strprefix(line->data, SPEC_MACRO_ENDIF)) {
                        /* do not read conditional macros as file globs */
                        continue;
                    } else {
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
                            pathspec = strchr(pathspec, ' ');
                        } else {
                            /* skip past %attr, %config, %verify */
                            while ((strprefix(pathspec, SPEC_FILES_ATTR) ||
                                    strprefix(pathspec, SPEC_FILES_CONFIG) ||
                                    strprefix(pathspec, SPEC_FILES_VERIFY)) &&
                                   pathspec != NULL) {
                                pathspec = strchr(pathspec, ')');

                                while (isspace(*pathspec) && pathspec != NULL) {
                                    pathspec++;
                                }
                            }
                        }

                        if (prefix) {
                            filenames = strsplit(pathspec, " \t");
                            assert(filenames != NULL);

                            TAILQ_FOREACH(filename, filenames, items) {
                                /* build this entry's path spec */
                                entry = joinpath(prefix, filename->data, NULL);
                                assert(entry != NULL);

                                /* expand macros */
                                expanded = rpmExpand(entry, NULL);

DEBUG_PRINT("glob: |%s|\n", expanded);

                                /* cleanup */
                                free(expanded);
                                free(entry);
                            }

                            list_free(filenames, free);
                            free(prefix);
                            prefix = NULL;
                        } else {
                            /* trim and leading or trailing whitespace */
                            pathspec = strtrim(pathspec);
                            assert(pathspec != NULL);

                            /* expand macros */
                            expanded = rpmExpand(pathspec, NULL);

                            /* XXX:
                               trim %ghost
                               anything else?
                             */

                            /* XXX:
                               probably should use a hash table here and avoid adding dupes
                             */
DEBUG_PRINT("glob: |%s|\n", expanded);

                            /* cleanup */
                            free(expanded);
                        }
                    }
                } else if (!found && strprefix(line->data, SPEC_SECTION_FILES)) {
                    found = true;
                    continue;
                }
            }

            list_free(speclines, free);
        }
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_FILESMATCH;
    params.verb = VERB_OK;

    /* Report a single OK message if everything was fine */
    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
