/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpminspect.h"

static bool dsodeps_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *bv = NULL;
    const char *av = NULL;
    const char *arch = NULL;
    int after_fd = -1;
    int before_fd = -1;
    Elf *after_elf = NULL;
    Elf *before_elf = NULL;
    GElf_Half after_type;
    GElf_Half before_type;
    size_t i;
    GElf_Dyn *dyn = NULL;
    size_t sz = 0;
    GElf_Shdr shdr;
    string_list_t *after_needed = NULL;
    string_list_t *before_needed = NULL;
    string_list_t *removed = NULL;
    string_list_t *added = NULL;
    string_entry_t *entry = NULL;
    char *tmp = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip files without a peer, other inspections handle new/missing files */
    if (!file->peer_file) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* Skip files in the debug path and debug source path */
    if (strprefix(file->localpath, DEBUG_PATH) || strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* Only run this inspection for builds that do not change versions */
    bv = headerGetString(file->peer_file->rpm_header, RPMTAG_VERSION);
    av = headerGetString(file->rpm_header, RPMTAG_VERSION);

    if (strcmp(bv, av)) {
        return true;
    }

    /* If we lack dynamic or shared ELF files, we're done */
    if ((after_elf = get_elf(file, &after_fd)) == NULL) {
        return true;
    }

    after_type = get_elf_type(after_elf);

    /* this inspection only operates on ET_DYN ELF types */
    if (after_type != ET_DYN) {
        goto done;
    }

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);
    assert(arch != NULL);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_DSODEPS;
    params.remedy = get_remedy(REMEDY_DSODEPS);
    params.arch = arch;
    params.file = file->localpath;

    if ((before_elf = get_elf(file, &before_fd)) == NULL) {
        xasprintf(&params.msg, _("%s was an ELF file and now is not on %s"), file->localpath, arch);
        params.verb = VERB_CHANGED;
        params.noun = _("ELF file ${FILE} on ${ARCH}");
        add_result(ri, &params);
        free(params.msg);
        result = false;
        goto done;
    }

    before_type = get_elf_type(before_elf);

    if (before_type != ET_EXEC && before_type != ET_DYN) {
        xasprintf(&params.msg, _("%s was a dynamic ELF file and now is not on %s"), file->localpath, arch);
        params.verb = VERB_CHANGED;
        params.noun = _("ELF file ${FILE} on ${ARCH}");
        add_result(ri, &params);
        free(params.msg);
        result = false;
        goto done;
    }

    /* Gather the DT_NEEDED entries */
    if (get_dynamic_tags(after_elf, DT_NEEDED, &dyn, &sz, &shdr)) {
        after_needed = calloc(1, sizeof(*after_needed));
        assert(after_needed != NULL);
        TAILQ_INIT(after_needed);

        for (i = 0; i < sz; i++) {
            after_needed = list_add(after_needed, elf_strptr(after_elf, shdr.sh_link, (size_t) (dyn[i].d_un.d_ptr)));
        }

        free(dyn);
    }

    if (get_dynamic_tags(before_elf, DT_NEEDED, &dyn, &sz, &shdr)) {
        before_needed = calloc(1, sizeof(*before_needed));
        assert(before_needed != NULL);
        TAILQ_INIT(before_needed);

        for (i = 0; i < sz; i++) {
            before_needed = list_add(before_needed, elf_strptr(before_elf, shdr.sh_link, (size_t) (dyn[i].d_un.d_ptr)));
        }

        free(dyn);
    }

    /* Figure out what symbol changes happened*/
    removed = list_difference(before_needed, after_needed);
    added = list_difference(after_needed, before_needed);

    /* Report out any findings */
    if (removed != NULL && !TAILQ_EMPTY(removed)) {
        xasprintf(&params.msg, _("DT_NEEDED symbol(s) removed from %s on %s"), file->localpath, arch);
        params.verb = VERB_REMOVED;
        params.noun = _("DT_NEEDED symbol(s) in ${FILE} on ${ARCH}");

        TAILQ_FOREACH(entry, removed, items) {
            xasprintf(&tmp, "%s\n", entry->data);
            free(params.details);
            params.details = tmp;
        }

        add_result(ri, &params);
        free(params.msg);
        free(params.details);
        result = false;
    }

    if (added != NULL && !TAILQ_EMPTY(added)) {
        xasprintf(&params.msg, _("DT_NEEDED symbol(s) added to %s on %s"), file->localpath, arch);
        params.verb = VERB_ADDED;
        params.noun = _("DT_NEEDED symbol(s) in ${FILE} on ${ARCH}");

        TAILQ_FOREACH(entry, added, items) {
            xasprintf(&tmp, "%s\n", entry->data);
            free(params.details);
            params.details = tmp;
        }

        add_result(ri, &params);
        free(params.msg);
        free(params.details);
        result = false;
    }

done:
    if (after_elf && after_fd != -1) {
        elf_end(after_elf);
        close(after_fd);
    }

    if (before_elf && before_fd != -1) {
        elf_end(before_elf);
        close(before_fd);
    }

    free(removed);
    free(added);
    list_free(before_needed, free);
    list_free(after_needed, free);

    return result;
}

/*
 * Main driver for the dsodeps inspection.
 */
bool inspect_dsodeps(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    assert(ri != NULL);

    /* run the dsodeps test across all ELF files */
    result = foreach_peer_file(ri, NAME_DSODEPS, dsodeps_driver);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_DSODEPS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
