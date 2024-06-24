/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <unistd.h>
#include <assert.h>
#include <err.h>
#include <ftw.h>
#include "uthash.h"
#include "rpminspect.h"

/*
 * @brief Given a vendor data dir and product release string, look for
 * an ABI compatibility level file.  If found, read it in and return
 * the newly constructed abi_t.  Caller is responsible for freeing the
 * returned data.
 */
abi_t *read_abi(const char *vendor_data_dir, const char *product_release)
{
    char *abifile = NULL;
    abi_t *r = NULL;
    abi_t *entry = NULL;
    string_entry_t *dsoval = NULL;
    string_list_t *contents = NULL;
    string_entry_t *line = NULL;
    string_list_t *linekv = NULL;
    string_list_t *dsos = NULL;
    string_entry_t *pkg = NULL;
    string_entry_t *dso = NULL;
    int found_level = 0;
    bool skip_entries = false;
    uint64_t levels = 0;

    assert(vendor_data_dir != NULL);
    assert(product_release != NULL);

    /* read the ABI compat level file to read */
    xasprintf(&abifile, "%s/%s/%s", vendor_data_dir, ABI_DIR, product_release);
    assert(abifile != NULL);
    contents = read_file(abifile);
    free(abifile);

    if (contents == NULL) {
        /* this product release lacks an ABI compat level file */
        return NULL;
    }

    /* read in the ABI compat levels */
    TAILQ_FOREACH(line, contents, items) {
        /* skip blank lines and comments */
        if (line->data == NULL || *line->data == '#' || *line->data == '\n' || *line->data == '\r' || !strcmp(line->data, "")) {
            continue;
        }

        /* determine if we are reading a new level or not */
        if (strprefix(line->data, "[") && strsuffix(line->data, "]") && (strcasestr(line->data, "level-") || strcasestr(line->data, "level "))) {
            /* new compat level section */

            /*
             * get the compat level we found and make sure we have
             * seen it before
             */
            if (sscanf(line->data + 7, "%d]", &found_level) == EOF) {
                warn(_("*** malformed ABI level identifier: %s"), line->data);
                skip_entries = true;
            }

            if (levels & (((uint64_t) 1) << found_level)) {
                warn(_("*** ABI level %d already defined"), found_level);
                skip_entries = true;
            }

            if (skip_entries) {
                continue;
            }
        } else {
            /* looking at an abi package entry line */

            /* split in to package and DSO list first */
            linekv = strsplit(line->data, "=");

            if (list_len(linekv) != 2) {
                warn(_("*** malformed ABI level entry: %s"), line->data);
                list_free(linekv, free);
                continue;
            }

            pkg = TAILQ_FIRST(linekv);
            dso = TAILQ_NEXT(pkg, items);

            /* split all the DSO values */
            dsos = strsplit(dso->data, ",\n\r");

            if (dsos == NULL) {
                warnx("*** malformed DSO list: %s", dso->data);
                list_free(linekv, free);
                continue;
            }

            /* try to find this package in the ABI compat level table */
            HASH_FIND_STR(r, pkg->data, entry);

            /* package is not found, add it to the table */
            if (entry == NULL) {
                entry = (abi_t *) xalloc(sizeof(*entry));
                entry->pkg = strdup(pkg->data);
                assert(entry->pkg != NULL);

                entry->level = found_level;
                entry->all = false;
                entry->dsos = NULL;

                HASH_ADD_KEYPTR(hh, r, entry->pkg, strlen(entry->pkg), entry);
            }

            /* collect all the DSO values */
            TAILQ_FOREACH(dsoval, dsos, items) {
                if (!strcasecmp(dsoval->data, "all-dsos")) {
                    /* flag "all DSOs" as part of this ABI level */
                    entry->all = true;
                } else {
                    entry->dsos = list_add(entry->dsos, dsoval->data);
                }
            }

            /* clean up */
            list_free(linekv, free);
            list_free(dsos, free);
            entry = NULL;          /* for the next loop iteration */
        }
    }

    /* clean up */
    list_free(contents, free);

    return r;
}

/*
 * Given an abi_t, free all the memory associated with it.
 */
void free_abi(abi_t *table)
{
    abi_t *entry = NULL;
    abi_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        list_free(entry->dsos, free);
        free(entry->pkg);
        free(entry);
    }

    free(table);
    return;
}

/*
 * Get any .abignore files that exist in SRPM files in the build.
 * These are passed to every invocation of abidiff(1) if they exist.
 */
string_list_t *get_abidiff_suppressions(const struct rpminspect *ri, const char *suppression_file)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(suppression_file != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!strcmp(file->localpath, suppression_file)) {
                if (list == NULL) {
                    list = (string_list_t *) xalloc(sizeof(*list));
                    TAILQ_INIT(list);
                }

                entry = (string_entry_t *) xalloc(sizeof(*entry));
                xasprintf(&entry->data, "%s %s", ABI_SUPPRESSIONS, file->fullpath);
                assert(entry->data != NULL);
                TAILQ_INSERT_TAIL(list, entry, items);
            }
        }
    }

    return list;
}
