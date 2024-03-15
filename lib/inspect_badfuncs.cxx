/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_badfuncs.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2021
 * @brief 'badfuncs' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <libelf.h>

#include "rpminspect.h"

static bool allowed_symbol(const struct rpminspect *ri, const rpmfile_entry_t *file, const char *symbol)
{
    char *root = NULL;
    size_t lenr = 0;
    size_t lenp = 0;
    string_entry_t *entry = NULL;
    string_list_map_t *hentry = NULL;
    string_list_map_t *tmp_hentry = NULL;

    assert(ri != NULL);
    assert(file != NULL);
    assert(symbol != NULL);

    /* no allowed bad functions defined, false */
    if (ri->bad_functions_allowed == NULL) {
        return false;
    }

    /* construct the root path */
    root = strdup(file->fullpath);
    assert(root != NULL);
    lenr = strlen(root);
    lenp = strlen(file->localpath);

    if (lenr > lenp) {
        root[strlen(root) - strlen(file->localpath) + 1] = '\0';
    }

    /* look for the given path in the bad functions allowed hash */
    HASH_ITER(hh, ri->bad_functions_allowed, hentry, tmp_hentry) {
        if (match_path(hentry->key, root, file->localpath)) {
            /* we found a matching path */
            TAILQ_FOREACH(entry, hentry->value, items) {
                if (!strcmp(symbol, entry->data)) {
                    free(root);
                    return true;
                }
            }
        }
    }

    free(root);
    return false;
}

/**
 * @brief Check for forbidden function symbols in ELF objects.
 *
 * Forbidden functions are generally library functions the system
 * provides for backwards compatibility but that have otherwise been
 * deprecated in favor of a newer or more modern API.
 *
 * @param ri The struct rpminspect pointer for the run of the program
 * @param after The rpmfile_entry_t to inspect
 * @return true if the rpmfile_entry_t passes, false if it fails
 */
static bool badfuncs_driver(struct rpminspect *ri, rpmfile_entry_t *after)
{
    bool result = true;
    const char *arch;
    Elf *after_elf = NULL;
    int after_elf_fd = -1;
    string_list_t *after_symbols = NULL;
    string_list_t *used_symbols = NULL;
    string_list_t *sorted_used = NULL;
    string_entry_t *iter = NULL;
    struct result_params params;
    FILE *output_stream = NULL;
    char *output_buffer = NULL;
    size_t output_size = 0;
    int output_result = 0;

    /* Skip source packages */
    if (headerIsSource(after->rpm_header)) {
        return true;
    }

    if (!after->fullpath || !S_ISREG(after->st.st_mode)) {
        return true;
    }

    if (!process_file_path(after, ri->elf_path_include, ri->elf_path_exclude)) {
        return true;
    }

    arch = get_rpm_header_arch(after->rpm_header);

    /* get an ELF object of some sort, if we can */
    after_elf = get_elf_archive(after->fullpath, &after_elf_fd);

    if (after_elf == NULL) {
        after_elf = get_elf(after->fullpath, &after_elf_fd);
    }

    if (after_elf == NULL) {
        result = true;
        goto cleanup;
    }

    /* Don't filter the list -- filtering requires knowledge of the
     * forbidden functions. Since we can't pass custom arguments to
     * the filter, return them all and filter them locally. */
    after_symbols = get_elf_imported_functions(after_elf, NULL);

    /* Get a list of forbidden symbols that we used. */
    used_symbols = list_intersection(ri->bad_functions, after_symbols);

    if (!used_symbols || TAILQ_EMPTY(used_symbols)) {
        goto cleanup;
    }

    /* Filter out any allowed forbidden symbols for this file */
    TAILQ_FOREACH(iter, used_symbols, items) {
        if (allowed_symbol(ri, after, iter->data)) {
            TAILQ_REMOVE(used_symbols, iter, items);
            free(iter->data);
            free(iter);
        }
    }

    if (TAILQ_EMPTY(used_symbols)) {
        goto cleanup;
    }

    /* At this point, we're using offending symbols. Build the results. */
    result = false;

    output_stream = open_memstream(&output_buffer, &output_size);
    assert(output_stream != NULL);

    output_result = fprintf(output_stream, _("Forbidden function symbols found:\n"));
    assert(output_result > 0);

    sorted_used = list_sort(used_symbols);
    assert(sorted_used != NULL);

    TAILQ_FOREACH(iter, sorted_used, items) {
        output_result = fprintf(output_stream, "\t%s\n", iter->data);
        assert(output_result > 0);
    }

    fflush(output_stream);
    output_result = fclose(output_stream);
    assert(output_result == 0);

    init_result_params(&params);
    xasprintf(&params.msg, _("%s may use forbidden functions on %s"), after->localpath, arch);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_BADFUNCS;
    params.remedy = get_remedy(REMEDY_BADFUNCS);
    params.details = output_buffer;
    params.verb = VERB_FAILED;
    params.noun = _("forbidden functions in ${FILE} on ${ARCH}");
    params.file = after->localpath;
    params.arch = arch;
    add_result(ri, &params);
    free(params.msg);
    free(output_buffer);

cleanup:
    list_free(after_symbols, free);
    list_free(used_symbols, free);
    list_free(sorted_used, free);

    if (after_elf) {
        elf_end(after_elf);
        close(after_elf_fd);
    }

    return result;
}

/**
 * @brief Perform the 'badfuncs' inspection.
 *
 * Looks in each ELF file and reports any forbidden function symbols
 * found.  The list of forbidden symbols is defined in the
 * configuration file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_badfuncs(struct rpminspect *ri)
{
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    if (ri->bad_functions != NULL) {
        result = foreach_peer_file(ri, NAME_BADFUNCS, badfuncs_driver);
    }

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_BADFUNCS;
        add_result(ri, &params);
    }

    return result;
}
