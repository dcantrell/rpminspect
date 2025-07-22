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

    if (s == NULL) {
DEBUG_PRINT("s is NULL\n");
        return;
    }

    /* sometimes we end up with empty strings, ignore */
    if (strlen(s) == 0) {
        return;
    }

    /* alright, we need at least some path separator in a pathspec */
    if (!strchr(s, PATH_SEP)) {
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
 * Trim any leading modifiers on %files entry lines that are not
 * delimited by spaces from the actual pathspec.  For instance, a line
 * like this:
 *
 *     %config(noreplace)/etc/passwd
 *
 * Would have the %config() part trimmed.  In cases where there is a
 * space between "%config(noreplace)" and "/etc/passwd, the main block
 * below would handle skipping the modifier.
 */
static char *trim_files_modifiers(char *s)
{
    char *r = s;

    if (r == NULL) {
        return NULL;
    }

    if (strprefix(r, SPEC_FILES_ATTR) ||
        strprefix(r, SPEC_FILES_CONFIG) ||
        strprefix(r, SPEC_FILES_VERIFY) ||
        strprefix(r, SPEC_FILES_LANG) ||
        strprefix(r, SPEC_FILES_CAPS)) {
        if (strchr(r, '(') && strchr(r, ')')) {
            r = strchr(r, ')');
        }

        while (*r != PATH_SEP && *r != '\0') {
            *r = ' ';
            r++;
        }
    }

    return r;
}

/*
 * Given a %files entry line, loop over the line expanding macros
 * until no more macros are seen.
 */
static char *expand_files_lines(struct rpminspect *ri, rpmfile_entry_t *file, char *s)
{
    char *r = NULL;
    char *tmp = NULL;
    char *prev = NULL;
    bool expand = true;

    assert(ri != NULL);
    assert(file != NULL);
    assert(s != NULL);

DEBUG_PRINT("s=|%s|\n", s);

    tmp = strdup(s);
    assert(tmp != NULL);

    while (expand) {
        /* no further expansion is occurring */
        if (prev && !strcmp(tmp, prev)) {
            r = tmp;
            expand = false;
            continue;
        }

        /* expand the macros */
        r = expand_macros(ri, file, tmp, "*");
        free(tmp);

        /* if we still see macros, keep expanding, otherwise get out */
        if (strchr(r, '%') == NULL) {
            expand = false;
        } else {
            if (prev) {
                free(prev);
            }

            prev = strdup(r);
            assert(prev != NULL);
            tmp = r;
        }
    }

    free(prev);
    return r;
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
    string_list_t *tokens = NULL;
    string_entry_t *token = NULL;
    bool found = false;
    const char *name = NULL;
    char *pathspec = NULL;
    char *expanded = NULL;
    char *entry = NULL;
    char *prefix = NULL;
    char *macrofunc = NULL;
    char *macro = NULL;
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
            /* find the spec file and read the lines */
            if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                speclines = read_file(file->fullpath);
                assert(speclines != NULL);
                break;
            }
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
                /* we have a glob(7) specification */
                pathspec = line->data;

                /* look for %files sections and skip other sections */
                if (strprefix(pathspec, SPEC_SECTION_FILES)) {
                    /* a subpackage %files section, keep reading */
                    continue;
                } else if (strprefix(pathspec, SPEC_FILES_DEFATTR) ||
                           strprefix(pathspec, SPEC_FILES_GHOST)) {
                    /* ignore %defattr and %ghost lines */
                    continue;
                } else if (*pathspec == '#') {
                    /* skip comments */
                    continue;
                } else if (strprefix(pathspec, SPEC_MACRO_IF) ||
                           strprefix(pathspec, SPEC_MACRO_ELSE) ||
                           strprefix(pathspec, SPEC_MACRO_ENDIF)) {
                    /* do not read conditional macros as file globs */
                    continue;
                } else if (strprefix(pathspec, SPEC_SECTION_DESCRIPTION) ||
                           strprefix(pathspec, SPEC_SECTION_PACKAGE) ||
                           strprefix(pathspec, SPEC_SECTION_PREP) ||
                           strprefix(pathspec, SPEC_SECTION_BUILD) ||
                           strprefix(pathspec, SPEC_SECTION_INSTALL) ||
                           strprefix(pathspec, SPEC_SECTION_CHECK) ||
                           strprefix(pathspec, SPEC_SECTION_PRE) ||
                           strprefix(pathspec, SPEC_SECTION_PREUN) ||
                           strprefix(pathspec, SPEC_SECTION_POST) ||
                           strprefix(pathspec, SPEC_SECTION_POSTUN) ||
                           strprefix(pathspec, SPEC_SECTION_TRIGGERUN) ||
                           strprefix(pathspec, SPEC_SECTION_CHANGELOG)) {
                    /* we reached the end of the %files section(s) */
                    found = false;
                    break;
                }

                /* split the line and check for documentation macros */
                tokens = strsplit(pathspec, " \t");

                if (list_len(tokens) >= 2) {
                    token = TAILQ_FIRST(tokens);
                } else {
                    token = NULL;
                }

                if (token && !strcmp(token->data, SPEC_FILES_DOC)) {
                    /* handle %doc */
                    macrofunc = SPEC_FILES_DOC;
                    macro = "_defaultdocdir";
                } else if (token && !strcmp(token->data, SPEC_FILES_LICENSE)) {
                    /* handle %licensedir */
                    macrofunc = SPEC_FILES_LICENSE;
                    macro = "_defaultlicensedir";
                }

                list_free(tokens, free);

                if (macrofunc && macro) {
                    pathspec += strlen(macrofunc) + 1;

                    while (*pathspec != '\0' && isspace(*pathspec)) {
                        pathspec++;
                    }

                    xasprintf(&prefix, "%%{%s}/%s", macro, name);
                    assert(prefix != NULL);
                }

                macrofunc = NULL;
                macro = NULL;

                /* save docs and licenses if we are on one of those lines */
                if (prefix) {
                    tokens = strsplit(pathspec, " \t");

                    if (tokens != NULL) {
                        TAILQ_FOREACH(token, tokens, items) {
                            /* build this entry's path spec */
                            entry = joindelim(PATH_SEP, prefix, token->data, NULL);
                            assert(entry != NULL);

                            /* expand macros */
DEBUG_PRINT("entry=|%s|\n", entry);
                            expanded = expand_files_lines(ri, file, entry);

                            /* save this glob in the right hash table */
DEBUG_PRINT("expanded=|%s|\n", expanded);
                            save_pathspec(expanded, type);

                            /* cleanup */
                            free(expanded);
                            free(entry);
                        }

                        list_free(tokens, free);
                    }

                    free(prefix);
                    prefix = NULL;
                    continue;
                }

                /* now scan horizontally through the line */
                tokens = strsplit(pathspec, " \t");

                if (tokens != NULL) {
                    type = FILES_GLOBS;

                    TAILQ_FOREACH(token, tokens, items) {
                        if (strprefix(token->data, SPEC_FILES_ATTR) ||
                            strprefix(token->data, SPEC_FILES_CONFIG) ||
                            strprefix(token->data, SPEC_FILES_VERIFY) ||
                            strprefix(token->data, SPEC_FILES_LANG) ||
                            strprefix(token->data, SPEC_FILES_CAPS)) {
                            /* skip %attr, %config, %verify, %lang, and %caps */
                            continue;
                        } else if (strprefix(token->data, SPEC_FILES_DIR)) {
                            /*
                             * this is a %dir specification so we
                             * record it in a different hash table
                             * because checking actual files in the
                             * binary RPMs will be checking path
                             * prefixes against these vs glob matching
                             */
                            type = FILES_DIRS;
                        } else if (strprefix(token->data, SPEC_FILES_EXCLUDE)) {
                            /*
                             * this is an %exclude specification which
                             * is a known glob that *SHOULD NOT* be
                             * present in built RPMs
                             */
                            type = FILES_EXCLUDES;
                        } else {
                            /* at this point we should have the pathspec portion */
                            pathspec = token->data;
                            break;
                        }
                    }

                    /*
                     * there is a possibility of %files modifiers
                     * existing without using space delimiters, so
                     * trim those
                     */
                    pathspec = trim_files_modifiers(pathspec);

                    /* trim and leading or trailing whitespace */
                    pathspec = strtrim(pathspec);

                    if (pathspec == NULL) {
                        /* no reason to continue if there's nothing here */
                        continue;
                    }

                    /* expand macros */
DEBUG_PRINT("   pathspec=|%s|\n", pathspec);
                    expanded = expand_files_lines(ri, file, pathspec);

                    if (expanded) {
                        /* save this glob in the right hash table */
DEBUG_PRINT("expanded=|%s|\n", expanded);
DEBUG_PRINT("    type=|%d|\n", type);
                        save_pathspec(expanded, type);
                    }

                    /* cleanup */
                    free(expanded);
                    list_free(tokens, free);
                }
            } else if (!found && strprefix(line->data, SPEC_SECTION_FILES)) {
                /* we found %files, start reading lines */
                found = true;
                continue;
            }
        }

        list_free(speclines, free);
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
