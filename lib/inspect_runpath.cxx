/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <err.h>
#include <sys/types.h>
#include <regex.h>

#include "rpminspect.h"

static string_list_t *get_tag_list(Elf *elf, const Elf64_Sxword tag)
{
    GElf_Dyn *dyn = NULL;
    size_t sz = 0;
    GElf_Shdr shdr;
    size_t i = 0;
    string_list_t *result = NULL;

    assert(elf != NULL);

    if (get_dynamic_tags(elf, tag, &dyn, &sz, &shdr)) {
        for (i = 0; i < sz; i++) {
            result = list_add(result, elf_strptr(elf, shdr.sh_link, (size_t) (dyn[i].d_un.d_ptr)));
        }

        free(dyn);
    }

    return result;
}

/*
 * Given a working path, check to see if any packages in our build own
 * that path.  True if we find it, false otherwise.
 */
static bool build_contains(const struct rpminspect *ri, const char *working_path)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;

    assert(ri != NULL);
    assert(working_path != NULL);
    assert(ri->peers != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (S_ISDIR(file->st_mode) && !strcmp(file->localpath, working_path)) {
                return true;
            }
        }
    }

    return false;
}

/*
 * Return a bool that represents whether or not the runpath
 * string_list_t contains valid runpath entries.  If all entries are
 * valid, the function returns true.  Any invalid entries trigger a
 * false return value.  Individual entries are reported in this
 * function so the user can analyze them in the results output.
 */
static bool check_runpath(struct rpminspect *ri, const rpmfile_entry_t *file, const char *symbol, const string_list_t *runpath)
{
    bool r = true;
    bool valid = true;
    string_list_t *allowed = NULL;
    string_list_t *parts = NULL;
    string_entry_t *runpath_value = NULL;
    string_entry_t *entry = NULL;
    string_entry_t *prefix = NULL;
    char *working_path = NULL;
    regex_t origin_root;
    int reg_result = 0;
    char reg_error[BUFSIZ];
    regmatch_t origin_matches[3];
    const char *arch = NULL;
    struct result_params params;

    /* no values, that's fine */
    if (runpath == NULL) {
        return true;
    }

    assert(ri != NULL);
    assert(file != NULL);
    assert(symbol != NULL);

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);
    assert(arch != NULL);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_RUNPATH;
    params.remedy = get_remedy(REMEDY_RUNPATH);
    params.arch = arch;
    params.file = file->localpath;

    /* loop over all possible ELF symbol values */
    TAILQ_FOREACH(runpath_value, runpath, items) {
        /* split the incoming runpath on ':' */
        parts = strsplit(runpath_value->data, ":");

        /* check each part against our rules */
        TAILQ_FOREACH(entry, parts, items) {
            valid = false;
            working_path = NULL;
            allowed = NULL;

            /* set up the working path, skip prefix if necessary */
            if (!strcmp(entry->data, RUNPATH_ORIGIN_STR)) {
                valid = true;
            } else if (strprefix(entry->data, RUNPATH_ORIGIN_STR)) {
                working_path = entry->data + strlen(RUNPATH_ORIGIN_STR);

                /* trim any "prefix" that follows $ORIGIN */
                if (ri->runpath_origin_prefix_trim && !TAILQ_EMPTY(ri->runpath_origin_prefix_trim)) {
                    TAILQ_FOREACH(prefix, ri->runpath_origin_prefix_trim, items) {
                        /* compile this regular expression root prefix */
                        reg_result = regcomp(&origin_root, prefix->data, REG_EXTENDED);

                        if (reg_result != 0) {
                            regerror(reg_result, &origin_root, reg_error, sizeof(reg_error));
                            warn(_("*** unable to compile $ORIGIN root prefix regular expression: %s"), reg_error);
                            continue;
                        }

                        /* no match, continue */
                        reg_result = regexec(&origin_root, working_path, 3, origin_matches, REG_EXTENDED);

                        if (reg_result == REG_NOMATCH) {
                            /* there was no match, check the next one */
                            regfree(&origin_root);
                            continue;
                        }

                        if (reg_result != 0) {
                            regerror(reg_result, &origin_root, reg_error, sizeof(reg_error));
                            warnx("*** regexec: %s", reg_error);
                            regfree(&origin_root);
                            continue;
                        }

                        /* advance past the prefix if there is one */
                        if (origin_matches[0].rm_so == -1) {
                            regfree(&origin_root);
                            continue;
                        }

                        working_path = working_path + origin_matches[0].rm_eo;
                        regfree(&origin_root);
                        break;
                    }
                }

                /* use this list of allowed values */
                allowed = ri->runpath_allowed_origin_paths;
            } else {
                working_path = entry->data;

                /* use this list of allowed values */
                allowed = ri->runpath_allowed_paths;
            }

            if (!valid && working_path) {
                /* canonicalize the path string */
                working_path = abspath(working_path);

                /* check for the working path in the allowed paths */
                if (list_contains(allowed, working_path) || build_contains(ri, working_path)) {
                    valid = true;
                }

                free(working_path);
            }

            if (!valid) {
                xasprintf(&params.msg, _("%s has an invalid-looking %s on %s: %s"), file->localpath, symbol, arch, entry->data);
                params.verb = VERB_FAILED;
                params.noun = _("runtime search path in ${FILE} on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);
                r = false;
            }
        }

        list_free(parts, free);
    }

    return r;
}

static bool runpath_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    int fd = -1;
    Elf *elf = NULL;
    GElf_Half type;
    string_list_t *rpath = NULL;
    string_list_t *runpath = NULL;
    const char *arch = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st_mode)) {
        return true;
    }

    /* Skip files in the debug path and debug source path */
    if (strprefix(file->localpath, DEBUG_PATH) || strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* If we lack dynamic or shared ELF files, we're done */
    if ((elf = get_elf(file, &fd)) == NULL) {
        result = true;
        goto cleanup;
    }

    type = get_elf_type(elf);

    /* From here on, we expect ET_EXEC or ET_DYN; ignore all other types */
    if (type != ET_EXEC && type != ET_DYN) {
        result = true;
        goto cleanup;
    }

    /* Gather any DT_RPATH and DT_RUNPATH entries */
    rpath = get_tag_list(elf, DT_RPATH);
    runpath = get_tag_list(elf, DT_RUNPATH);

    /* No entries to check, just return successfully */
    if ((rpath == NULL || TAILQ_EMPTY(rpath)) && (runpath == NULL || TAILQ_EMPTY(runpath))) {
        result = true;
        goto cleanup;
    }

    /* We should never have both */
    if ((rpath && !TAILQ_EMPTY(rpath)) && (runpath && !TAILQ_EMPTY(runpath))) {
        arch = get_rpm_header_arch(file->rpm_header);
        assert(arch != NULL);

        init_result_params(&params);
        params.header = NAME_RUNPATH;
        params.severity = RESULT_BAD;
        params.waiverauth = NOT_WAIVABLE;
        params.remedy = get_remedy(REMEDY_RUNPATH_BOTH);
        params.file = file->localpath;
        params.arch = arch;
        params.verb = VERB_FAILED;
        params.noun = _("both DT_RPATH and DT_RUNPATH in ${FILE} on ${ARCH}");

        xasprintf(&params.msg, _("%s has both DT_RPATH and DT_RUNPATH on %s; this is not allowed"), file->localpath, arch);
        add_result(ri, &params);
        free(params.msg);

        result = false;
    }

    /* Check DT_RPATH */
    if (!check_runpath(ri, file, "DT_RPATH", rpath)) {
        result = false;
    }

    /* Check DT_RUNPATH */
    if (!check_runpath(ri, file, "DT_RUNPATH", runpath)) {
        result = false;
    }

cleanup:
    if (elf && fd != -1) {
        elf_end(elf);
        close(fd);
    }

    list_free(rpath, free);
    list_free(runpath, free);

    return result;
}

/*
 * Main driver for the runpath inspection.
 */
bool inspect_runpath(struct rpminspect *ri)
{
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    /* run the runpath test across all ELF files */
    result = foreach_peer_file(ri, NAME_RUNPATH, runpath_driver);

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_RUNPATH;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
