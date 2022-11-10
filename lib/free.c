/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <regex.h>
#include <stdlib.h>
#include "queue.h"
#include "rpminspect.h"

static void free_string_list_map(string_list_map_t *table)
{
    string_list_map_t *entry = NULL;
    string_list_map_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        free(entry->key);
        list_free(entry->value, free);
        free(entry);
    }

    return;
}

static void free_deprule_ignore_map(deprule_ignore_map_t *table)
{
    deprule_ignore_map_t *entry = NULL;
    deprule_ignore_map_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        free_regex(entry->ignore);
        free(entry->pattern);
        free(entry);
    }

    return;
}

void free_regex(regex_t *regex)
{
    if (regex == NULL) {
        return;
    }

    regfree(regex);
    free(regex);
}

void free_string_map(string_map_t *table)
{
    string_map_t *entry = NULL;
    string_map_t *tmp_entry = NULL;

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, entry, tmp_entry) {
        HASH_DEL(table, entry);
        free(entry->key);
        free(entry->value);
        free(entry);
    }

    return;
}

/*
 * Free a struct rpminspect.  Called by applications using
 * librpminspect before they exit.
 */
void free_rpminspect(struct rpminspect *ri)
{
    fileinfo_entry_t *fientry = NULL;
    caps_entry_t *centry = NULL;
    caps_filelist_entry_t *cflentry = NULL;
    header_cache_t *hentry = NULL;
    header_cache_t *tmp_hentry = NULL;
    politics_entry_t *pentry = NULL;
    security_entry_t *sentry = NULL;
    secrule_t *srentry = NULL;
    secrule_t *tmp_srentry = NULL;

    if (ri == NULL) {
        return;
    }

    free(ri->progname);
    list_free(ri->cfgfiles, free);
    free(ri->workdir);
    free(ri->kojihub);
    free(ri->kojiursine);
    free(ri->kojimbs);
    free(ri->worksubdir);

    free(ri->vendor_data_dir);
    list_free(ri->licensedb, free);

    if (ri->fileinfo) {
        while (!TAILQ_EMPTY(ri->fileinfo)) {
            fientry = TAILQ_FIRST(ri->fileinfo);
            TAILQ_REMOVE(ri->fileinfo, fientry, items);

            free(fientry->owner);
            free(fientry->group);
            free(fientry->filename);

            free(fientry);
        }

        free(ri->fileinfo);
    }

    free(ri->fileinfo_filename);

    if (ri->caps) {
        while (!TAILQ_EMPTY(ri->caps)) {
            centry = TAILQ_FIRST(ri->caps);
            TAILQ_REMOVE(ri->caps, centry, items);

            free(centry->pkg);

            if (centry->files) {
                while (!TAILQ_EMPTY(centry->files)) {
                    cflentry = TAILQ_FIRST(centry->files);
                    TAILQ_REMOVE(centry->files, cflentry, items);

                    free(cflentry->path);
                    free(cflentry->caps);

                    free(cflentry);
                }

                free(centry->files);
            }

            free(centry);
        }

        free(ri->caps);
    }

    free(ri->caps_filename);
    list_free(ri->rebaseable, free);
    free(ri->rebaseable_filename);

    if (ri->politics) {
        while (!TAILQ_EMPTY(ri->politics)) {
            pentry = TAILQ_FIRST(ri->politics);
            TAILQ_REMOVE(ri->politics, pentry, items);

            free(pentry->pattern);
            free(pentry->digest);
            free(pentry);
        }

        free(ri->politics);
    }

    free(ri->politics_filename);

    if (ri->security) {
        while (!TAILQ_EMPTY(ri->security)) {
            sentry = TAILQ_FIRST(ri->security);
            TAILQ_REMOVE(ri->security, sentry, items);

            free(sentry->path);
            free(sentry->pkg);
            free(sentry->ver);
            free(sentry->rel);

            if (sentry->rules) {
                HASH_ITER(hh, sentry->rules, srentry, tmp_srentry) {
                    HASH_DEL(sentry->rules, srentry);
                    free(srentry);
                }
            }

            free(sentry);
        }

        free(ri->security);
    }

    free(ri->security_filename);
    list_free(ri->badwords, free);
    list_free(ri->icons, free);
    free(ri->icons_filename);

    free_regex(ri->elf_path_include);
    free_regex(ri->elf_path_exclude);
    free_regex(ri->manpage_path_include);
    free_regex(ri->manpage_path_exclude);
    free_regex(ri->xml_path_include);
    free_regex(ri->xml_path_exclude);

    free(ri->elf_path_include_pattern);
    free(ri->elf_path_exclude_pattern);
    list_free(ri->automacros, free);
    list_free(ri->bad_functions, free);
    free_string_list_map(ri->bad_functions_allowed);
    free(ri->manpage_path_include_pattern);
    free(ri->manpage_path_exclude_pattern);
    free(ri->xml_path_include_pattern);
    free(ri->xml_path_exclude_pattern);

    free(ri->desktop_entry_files_dir);
    free(ri->vendor);

    free(ri->commands.msgunfmt);
    free(ri->commands.desktop_file_validate);
    free(ri->commands.abidiff);
    free(ri->commands.kmidiff);

    list_free(ri->buildhost_subdomain, free);
    list_free(ri->macrofiles, free);
    list_free(ri->security_path_prefix, free);
    list_free(ri->header_file_extensions, free);
    list_free(ri->forbidden_path_prefixes, free);
    list_free(ri->forbidden_path_suffixes, free);
    list_free(ri->forbidden_directories, free);
    free(ri->before);
    free(ri->after);
    free(ri->product_release);
    list_free(ri->arches, free);
    list_free(ri->bin_paths, free);
    free(ri->bin_owner);
    free(ri->bin_group);
    list_free(ri->forbidden_owners, free);
    list_free(ri->forbidden_groups, free);
    list_free(ri->shells, free);
    free_string_map(ri->jvm);
    free_string_map(ri->annocheck);
    free(ri->annocheck_profile);
    free_string_map(ri->pathmigration);
    list_free(ri->pathmigration_excluded_paths, free);
    free_string_map(ri->products);
    list_free(ri->ignores, free);
    list_free(ri->lto_symbol_name_prefixes, free);
    list_free(ri->forbidden_paths, free);
    free(ri->abidiff_suppression_file);
    free(ri->abidiff_debuginfo_path);
    free(ri->abidiff_extra_args);
    free(ri->kmidiff_suppression_file);
    free(ri->kmidiff_debuginfo_path);
    free(ri->kmidiff_extra_args);
    list_free(ri->kernel_filenames, free);
    free(ri->kabi_dir);
    free(ri->kabi_filename);
    list_free(ri->patch_ignore_list, free);
    list_free(ri->runpath_allowed_paths, free);
    list_free(ri->runpath_allowed_origin_paths, free);
    list_free(ri->runpath_origin_prefix_trim, free);
    free_string_list_map(ri->inspection_ignores);
    list_free(ri->expected_empty_rpms, free);
    free_regex(ri->unicode_exclude);
    list_free(ri->unicode_excluded_mime_types, free);
    list_free(ri->unicode_forbidden_codepoints, free);
    free_deprule_ignore_map(ri->deprules_ignore);
    free(ri->debuginfo_sections);

    free_peers(ri->peers);

    HASH_ITER(hh, ri->header_cache, hentry, tmp_hentry) {
        HASH_DEL(ri->header_cache, hentry);
        free(hentry->pkg);
        headerFree(hentry->hdr);
        free(hentry);
    }

    free(ri->before_rel);
    free(ri->after_rel);
    free_pair(ri->macros);

    free_results(ri->results);

    return;
}

/*
 * Free the memory associate with a deprule_list_t list.
 */
void free_deprules(deprule_list_t *list)
{
    deprule_entry_t *entry = NULL;

    if (list == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(list)) {
        entry = TAILQ_FIRST(list);
        TAILQ_REMOVE(list, entry, items);
        free(entry->requirement);
        free(entry->version);
        list_free(entry->providers, free);
        free(entry);
    }

    free(list);
    return;
}
