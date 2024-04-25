/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_elf.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019
 * @brief 'elf' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <err.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <libelf.h>

#ifndef PT_GNU_STACK
#define PT_GNU_STACK 0x647e551 /* Indicate executable stack */
#endif

#ifndef PT_GNU_RELRO
#define PT_GNU_RELRO 0x647e552 /* Make read-only after relocation */
#endif

#include "queue.h"
#include "inspect.h"
#include "readelf.h"
#include "rpminspect.h"

/* defined in inspect_elf_bits.c. See pic_bits.sh */
bool is_pic_reloc(Elf64_Half, Elf64_Xword);

/* used by callback functions that need access to ri */
static struct rpminspect *rip = NULL;

/**
 * @brief Check if execstack is present.
 *
 * Check whether the given object file has information about the stack
 * settings. This is contained in the GNU_STACK program header for
 * ET_EXEC and ET_DYN, and in the .note.GNU-stack section for ET_REL
 *
 * @param elf ELF object to check
 */
bool is_execstack_present(Elf *elf)
{
    GElf_Phdr phdr;

    switch (get_elf_type(elf)) {
        case ET_REL:
            return have_elf_section(elf, SHT_PROGBITS, ".note.GNU-stack");
        case ET_EXEC:
        case ET_DYN:
            return (get_elf_phdr(elf, PT_GNU_STACK, &phdr) != NULL);
        default:
            return false;
    }
}

/**
 * @brief Return the flags value from the execstack data.
 *
 * This is either the p_flags from the GNU_STACK program header entry
 * or the sh_flags from the .note.GNU-stack section.
 *
 * @param ELF object to check
 */
uint64_t get_execstack_flags(Elf *elf)
{
    GElf_Phdr phdr;
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;

    assert(elf != NULL);

    switch (get_elf_type(elf)) {
        case ET_REL:
            scn = get_elf_section(elf, SHT_PROGBITS, ".note.GNU-stack", NULL, &shdr);

            if (scn == NULL) {
                return 0;
            }

            return shdr.sh_flags;
        case ET_EXEC:
        case ET_DYN:
            if (get_elf_phdr(elf, PT_GNU_STACK, &phdr) == NULL) {
                return 0;
            }

            return phdr.p_flags;
        default:
            return 0;
    }
}

/**
 * @brief Return true if this object has a SHT_PROGBITS section with
 * SHF_EXECINSTR set.
 *
 * This is a way of filtering out the ET_REL DWARF objects in
 * /usr/lib/debug/.dwz, which have no executable code.
 *
 * @param elf ELF object to check
 */
bool has_executable_program(Elf *elf)
{
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;

    while ((scn = get_elf_section(elf, SHT_PROGBITS, NULL, scn, &shdr)) != NULL) {
        if (shdr.sh_flags & SHF_EXECINSTR) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check whether the given object's execstack information makes
 * sense.
 *
 * For ET_EXEC and ET_DYN, PF_W and PF_R must be set. For ET_REL,
 * nothing other than SHF_EXECINSTR should be set.
 * flags is the value returned by get_execstack_flags. The Elf pointer
 * is still required to figure out which flags they are.
 *
 * @param elf ELF object to check
 * @param flags segment flags to look for
 */
bool is_execstack_valid(Elf *elf, uint64_t flags)
{
    switch (get_elf_type(elf)) {
        case ET_REL:
            /* Mask out SHF_EXECINSTR, check that nothing else is set */
            return !(flags & ~(SHF_EXECINSTR));
        case ET_EXEC:
        case ET_DYN:
            /* PF_W and PF_R must be set, nothing besides those two and PF_X should be set */
            return ((flags & (PF_W | PF_R)) && !(flags & ~(PF_W | PF_R | PF_X)));
        default:
            return false;
    }
}

/**
 * @brief Like is_stack_valid but only look for executable flag.
 *
 * Return true if the relevant executable bit is set.
 *
 * @param elf ELF object to check
 * @param flags segment flags to look for
 */
bool is_stack_executable(Elf *elf, uint64_t flags)
{
    switch (get_elf_type(elf)) {
        case ET_REL:
            return flags & SHF_EXECINSTR;
        case ET_EXEC:
        case ET_DYN:
            return flags & PF_X;
        default:
            return false;
    }
}

/**
 * @brief Return true if this object has a DT_TEXTREL entry
 *
 * @param elf ELF object to check
 */
bool has_textrel(Elf *elf)
{
    return have_dynamic_tag(elf, DT_TEXTREL) || have_dynamic_flag(elf, DF_TEXTREL);
}

/**
 * @brief true if there is a PT_GNU_RELRO phdr
 *
 * @param elf ELF object to check
 */
bool has_relro(Elf *elf)
{
    GElf_Phdr phdr;
    return (get_elf_phdr(elf, PT_GNU_RELRO, &phdr) != NULL);
}

/**
 * @brief true if there is a DT_BIND_NOW entry
 *
 * @param elf ELF object to check
 */
bool has_bind_now(Elf *elf)
{
    return have_dynamic_tag(elf, DT_BIND_NOW) || have_dynamic_flag(elf, DF_BIND_NOW);
}

/**
 * @brief Check the referenced symbol for global binding.
 */
static bool is_global_reloc(GElf_Shdr *symtab_shdr, Elf_Data *symtab_data, Elf_Data *symtab_xdata, uint64_t r_sym)
{
    GElf_Sym sym;
    size_t num_syms = 0;

    /* Sanity check, make sure the symbol index isn't bigger than the symbol table */
    assert(symtab_shdr->sh_entsize > 0);
    num_syms = symtab_shdr->sh_size / symtab_shdr->sh_entsize;

    if (r_sym >= num_syms) {
        return false;
    }

    memset(&sym, '\0', sizeof(sym));

    if (gelf_getsymshndx(symtab_data, symtab_xdata, r_sym, &sym, NULL) == NULL) {
        return false;
    }

    if (GELF_ST_BIND(sym.st_info) == STB_GLOBAL) {
        return true;
    }

    return false;
}

/**
 * @brief Given the ET_REL object, return whether we think it was
 * compiled with -fPIC
 *
 * This is kind of iffy. Whether the relocations in a given ELF object
 * are PIC or not depend on the type of relocation encoded in r_info,
 * and all of the relocation types are processor specific. This code
 * uses the function is_pic_reloc, which is generated by pic_bits.sh,
 * which just greps the R_<arch>_* constants for the the ones that
 * include "PLT" or "GOT" in the macro name.  Any presence of TEXTREL
 * means -fPIC was not used.
 *
 * All together, the idea is:
 *   * iterate over all relocations
 *   * if the relocation is for a symbol of binding other than
 *     STB_GLOBAL, it's probably fine
 *   * otherwise, if the relocation type doesn't pass is_pic_reloc,
 *     return false.
 *
 * @param elf ELF object to check
 * @return True if the object was probably built with -fPIC, False
 *         otherwise.
 */
bool is_pic_ok(Elf *elf)
{
    GElf_Ehdr ehdr;
    Elf_Scn *rel_section = NULL;
    GElf_Shdr rel_shdr;
    Elf_Data *rel_data = NULL;

    Elf_Scn *symtab_section = NULL;
    GElf_Shdr symtab_shdr;
    Elf_Data *symtab_data = NULL;

    Elf_Scn *xndxscn = NULL;
    Elf_Data *xndxdata = NULL;

    GElf_Rel rel;
    GElf_Rela rela;

    size_t entry_size = 0;
    size_t i = 0;

    assert(elf != NULL);

    if (gelf_getehdr(elf, &ehdr) == NULL) {
        return true;
    }

    /* Fetch the symtab data */
    if ((symtab_section = get_elf_section(elf, SHT_SYMTAB, NULL, NULL, &symtab_shdr)) == NULL) {
        return true;
    }

    if ((symtab_data = elf_getdata(symtab_section, NULL)) == NULL) {
        return true;
    }

    if ((xndxscn = get_elf_extended_section(elf, symtab_section, NULL)) != NULL) {
        if ((xndxdata = elf_getdata(xndxscn, NULL)) == NULL) {
            return true;
        }
    }

    /* Look for a SHT_RELA section first */
    rel_section = get_elf_section(elf, SHT_RELA, ".rela.text", NULL, &rel_shdr);
    rel_data = NULL;

    if (rel_section != NULL) {
        while ((rel_data = elf_getdata(rel_section, rel_data)) != NULL) {
            if (!(entry_size = gelf_fsize(elf, rel_data->d_type, 1, EV_CURRENT))) {
                continue;
            }

            for (i = 0; i < rel_shdr.sh_size / entry_size; i++) {
                if (gelf_getrela(rel_data, i, &rela) == NULL) {
                    continue;
                }

                if (is_global_reloc(&symtab_shdr, symtab_data, xndxdata, GELF_R_SYM(rela.r_info)) || is_pic_reloc(ehdr.e_machine, GELF_R_TYPE(rela.r_info))) {
                    return true;
                }
            }
        }
    }

    /* Try again with SHT_REL */
    rel_section = get_elf_section(elf, SHT_REL, ".rel.text", NULL, &rel_shdr);
    rel_data = NULL;

    if (rel_section != NULL) {
        while ((rel_data = elf_getdata(rel_section, rel_data)) != NULL) {
            if (!(entry_size = gelf_fsize(elf, rel_data->d_type, 1, EV_CURRENT))) {
                continue;
            }

            for (i = 0; i < rel_shdr.sh_size / entry_size; i++) {
                if (gelf_getrel(rel_data, i, &rel) == NULL) {
                    continue;
                }

                if (is_global_reloc(&symtab_shdr, symtab_data, xndxdata, GELF_R_SYM(rela.r_info)) || is_pic_reloc(ehdr.e_machine, GELF_R_TYPE(rel.r_info))) {
                    return true;
                }
            }
        }
    }

    /* any TEXTREL presence means no -fPIC */
    if (has_textrel(elf)) {
        return false;
    }

    return true;
}

static const char *pflags_to_str(uint64_t flags)
{
    /* enough space for RWX?\0 */
    static char output[5];
    char *current = output;

    memset(output, 0, sizeof(output));

    if (flags & PF_R) {
        *current = 'R';
        current++;
    }

    if (flags & PF_W) {
        *current = 'W';
        current++;
    }

    if (flags & PF_X) {
        *current = 'X';
        current++;
    }

    if (flags & ~(PF_R|PF_W|PF_X)) {
        *current = '?';
        current++;
    }

    return output;
}

static void add_execstack_flag_str(string_list_t *list, const char *s)
{
    if (s == NULL) {
        return;
    }

    (void) list_add(list, s);

    return;
}

static bool inspect_elf_execstack(struct rpminspect *ri, Elf *after_elf, Elf *before_elf, rpmfile_entry_t *file, const char *arch)
{
    Elf64_Half elf_type;
    uint64_t execstack_flags;
    string_list_t *flaglist = NULL;
    char *fs = NULL;
    bool result = true;
    bool before_execstack = false;
    bool after_execstack = false;
    struct result_params params;

    /* If there is no executable code, there is no executable stack */
    if (!has_executable_program(after_elf)) {
        return true;
    }

    elf_type = get_elf_type(after_elf);

    /* If the peer file had an executable stack, turn down the result severity */
    after_execstack = is_execstack_present(after_elf);

    if (before_elf) {
        before_execstack = is_stack_executable(before_elf, get_execstack_flags(before_elf));
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.waiverauth = WAIVABLE_BY_SECURITY;
    params.header = NAME_ELF;
    params.arch = arch;
    params.file = file->localpath;

    /* Check if execstack information is present */
    if (!after_execstack) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_EXECSTACK);
        params.msg = NULL;

        if (elf_type == ET_REL) {
            /* Missing .note.GNU-stack will result in an executable stack */
            if (before_execstack && after_execstack) {
                xasprintf(&params.msg, _("Object still has executable stack (no GNU-stack note): %s on %s"), file->localpath, arch);
            } else if (after_execstack) {
                xasprintf(&params.msg, _("Object has executable stack (no GNU-stack note): %s on %s"), file->localpath, arch);
            }
        } else {
            xasprintf(&params.msg, _("Program built without GNU_STACK: %s on %s"), file->localpath, arch);
        }

        if (params.msg != NULL && params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            params.remedy = get_remedy(REMEDY_ELF_EXECSTACK_MISSING);
            params.verb = VERB_CHANGED;
            params.noun = _("GNU_STACK in ${FILE} on ${ARCH}");
            add_result(ri, &params);
            result = false;
        }

        free(params.msg);
        return result;
    }

    /* Check that the execstack flags make sense */
    execstack_flags = get_execstack_flags(after_elf);

    if (after_execstack && !is_execstack_valid(after_elf, execstack_flags)) {
        if (elf_type == ET_REL) {
            flaglist = (string_list_t *) calloc(1, sizeof(*flaglist));
            assert(flaglist != NULL);
            TAILQ_INIT(flaglist);

            if (execstack_flags & SHF_WRITE) {
                add_execstack_flag_str(flaglist, "SHF_WRITE");
            } else if (execstack_flags & SHF_ALLOC) {
                add_execstack_flag_str(flaglist, "SHF_ALLOC");
            } else if (execstack_flags & SHF_MERGE) {
                add_execstack_flag_str(flaglist, "SHF_MERGE");
            } else if (execstack_flags & SHF_STRINGS) {
                add_execstack_flag_str(flaglist, "SHF_STRINGS");
            } else if (execstack_flags & SHF_INFO_LINK) {
                add_execstack_flag_str(flaglist, "SHF_INFO_LINK");
            } else if (execstack_flags & SHF_LINK_ORDER) {
                add_execstack_flag_str(flaglist, "SHF_LINK_ORDER");
            } else if (execstack_flags & SHF_OS_NONCONFORMING) {
                add_execstack_flag_str(flaglist, "SHF_OS_NONCONFORMING");
            } else if (execstack_flags & SHF_GROUP) {
                add_execstack_flag_str(flaglist, "SHF_GROUP");
            } else if (execstack_flags & SHF_TLS) {
                add_execstack_flag_str(flaglist, "SHF_TLS");
            } else if (execstack_flags & SHF_COMPRESSED) {
                add_execstack_flag_str(flaglist, "SHF_COMPRESSED");
            } else if (execstack_flags & SHF_MASKOS) {
                add_execstack_flag_str(flaglist, "SHF_MASKOS");
            } else if (execstack_flags & SHF_MASKPROC) {
                add_execstack_flag_str(flaglist, "SHF_MASKPROC");
#ifdef SHF_ORDERED
            } else if (execstack_flags & SHF_ORDERED) {
                add_execstack_flag_str(flaglist, "SHF_ORDERED");
#endif
#ifdef SHF_EXCLUDE
            } else if (execstack_flags & SHF_EXCLUDE) {
                add_execstack_flag_str(flaglist, "SHF_EXCLUDE");
#endif
            }

            fs = list_to_string(flaglist, ", ");
            xasprintf(&params.msg, _("File %s has invalid execstack flags (%s) on %s"), file->localpath, fs, arch);

            free(fs);
            list_free(flaglist, free);
        } else {
            xasprintf(&params.msg, _("File %s has unrecognized GNU_STACK '%s' (expected RW or RWE) on %s"), file->localpath, pflags_to_str(execstack_flags), arch);
        }

        if (params.msg) {
            params.severity = get_secrule_result_severity(ri, file, SECRULE_EXECSTACK);

            if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                params.remedy = get_remedy(REMEDY_ELF_EXECSTACK_INVALID);
                params.verb = VERB_FAILED;
                params.noun = _("execstack in ${FILE} on ${ARCH}");
                add_result(ri, &params);
                result = false;
            }

            free(params.msg);
        }
    }

    /* Check that the stack is not marked as executable */
    if (after_execstack && is_stack_executable(after_elf, execstack_flags)) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_EXECSTACK);

        if (elf_type == ET_REL) {
            if (before_execstack) {
                xasprintf(&params.msg, _("Object still has executable stack (GNU-stack note = X): %s on %s"), file->localpath, arch);
            } else {
                xasprintf(&params.msg, _("Object has executable stack (GNU-stack note = X): %s on %s"), file->localpath, arch);
            }
        } else {
            if (before_execstack) {
                xasprintf(&params.msg, _("Stack is still executable: %s on %s"), file->localpath, arch);
            } else {
                xasprintf(&params.msg, _("Stack is executable: %s on %s"), file->localpath, arch);
            }
        }

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            params.remedy = get_remedy(REMEDY_ELF_EXECSTACK_EXECUTABLE);
            params.verb = VERB_FAILED;
            params.noun = _("execstack in ${FILE} on ${ARCH}");
            add_result(ri, &params);
            result = false;
        }

        free(params.msg);
    }

    return result;
}

static bool check_relro(struct rpminspect *ri, Elf *before_elf, Elf *after_elf, const rpmfile_entry_t *file, const char *arch)
{
    bool r = true;
    bool before_relro = has_relro(before_elf);
    bool before_bind_now = has_bind_now(before_elf);
    bool after_relro = has_relro(after_elf);
    bool after_bind_now = has_bind_now(after_elf);
    struct result_params params;

    init_result_params(&params);

    if (before_relro && before_bind_now && after_relro && !after_bind_now) {
        /* full relro in before, partial relro in after */
        xasprintf(&params.msg, _("%s lost full GNU_RELRO security protection on %s"), file->localpath, arch);
    } else if (before_relro && !after_relro) {
        /* partial or full relro in before, no relro in after */
        xasprintf(&params.msg, _("%s lost GNU_RELRO security protection on %s"), file->localpath, arch);
    }

    if (params.msg != NULL) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_RELRO);
        params.waiverauth = WAIVABLE_BY_SECURITY;
        params.header = NAME_ELF;
        params.remedy = get_remedy(REMEDY_ELF_GNU_RELRO);
        params.arch = arch;
        params.file = file->localpath;
        params.verb = VERB_REMOVED;
        params.noun = _("lost GNU_RELRO in ${FILE} on ${ARCH}");

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            add_result(ri, &params);
            r = false;
        }

        free(params.msg);
    }

    return r;
}

/**
 * @brief Helper for elf_archive_tests; add the archive member to the
 * list if compiled *without* -fPIC
 *
 * @param elf ELF object to check
 * @param user_data List of ELF archives compiled without -fPIC
 */
bool find_no_pic(Elf *elf, string_list_t **no_pic_list)
{
    Elf_Arhdr *arhdr = NULL;

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    assert(arhdr->ar_name != NULL);

    /* Skip the / entry */
    if (strprefix(arhdr->ar_name, "/")) {
        return true;
    }

    if (!is_pic_ok(elf)) {
        *no_pic_list = list_add(*no_pic_list, arhdr->ar_name);
    }

    return true;
}

/**
 * @brief Helper for elf_archive_tests, add the archive member to the
 * list if compiled *with* -fPIC
 *
 * @param elf ELF object to check
 * @param user_data List of ELF archives compiled with -fPIC
 */
bool find_pic(Elf *elf, string_list_t **pic_list)
{
    Elf_Arhdr *arhdr = NULL;

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    assert(arhdr->ar_name != NULL);

    /* Skip the / entry */
    if (strprefix(arhdr->ar_name, "/")) {
        return true;
    }

    if (is_pic_ok(elf)) {
        *pic_list = list_add(*pic_list, arhdr->ar_name);
    }

    return true;
}

/**
 * @brief Helper for elf_archive_tests, get all archive member names
 *
 * @param elf ELF object to check
 * @param user_data List of ELF archives
 */
bool find_all(Elf *elf, string_list_t **all_list)
{
    Elf_Arhdr *arhdr = NULL;

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    assert(arhdr->ar_name != NULL);

    /* Skip the / entry */
    if (strprefix(arhdr->ar_name, "/")) {
        return true;
    }

    *all_list = list_add(*all_list, arhdr->ar_name);

    return true;
}

static bool elf_archive_tests(struct rpminspect *ri, Elf *after_elf, int after_elf_fd, Elf *before_elf, int before_elf_fd, const rpmfile_entry_t *file, const char *arch, const char *name)
{
    string_list_t *after_no_pic = NULL;
    string_list_t *before_pic = NULL;
    string_list_t *before_all = NULL;
    string_list_t *after_lost_pic = NULL;
    string_list_t *after_new = NULL;
    string_entry_t *iter = NULL;
    FILE *output_stream = NULL;
    char *screendump = NULL;
    size_t screendump_size = 0;
    int output_result = 0;
    struct result_params params;
    bool result = true;

    /* ignore unused variable warnings if assert is disabled */
    (void) output_result;

    /* comparison-only, skip if no before */
    if (!before_elf) {
        return true;
    }

    /* initialize the list of objects in the after build without PIC */
    elf_archive_iterate(after_elf_fd, after_elf, find_no_pic, &after_no_pic);

    /* initialize the list of objects in the before build with PIC */
    elf_archive_iterate(before_elf_fd, before_elf, find_pic, &before_pic);

    if (after_no_pic == NULL || before_pic == NULL) {
        return true;
    }

    /* Gather data for two possible messages:
     *   - Objects in after that had -fPIC in before
     *   - Objects in after that are completely new
     *
     * It's still possible for this test to pass if everything without -fPIC in after
     * also did not have -fPIC in before.
     */

    output_stream = open_memstream(&screendump, &screendump_size);
    assert(output_stream != NULL);

    /* Report objects that lost -fPIC */
    after_lost_pic = list_intersection(before_pic, after_no_pic);

    if (after_lost_pic && !TAILQ_EMPTY(after_lost_pic)) {
        result = false;

        output_result = fprintf(output_stream, _("The following objects lost -fPIC:\n"));
        assert(output_result > 0);

        TAILQ_FOREACH(iter, after_lost_pic, items) {
            output_result = fprintf(output_stream, "\t%s\n", iter->data);
            assert(output_result > 0);
        }
    }

    /* Report new objects built without -fPIC */
    elf_archive_iterate(before_elf_fd, before_elf, find_all, &before_all);

    after_new = list_difference(after_no_pic, before_all);

    if (after_new && list_len(after_new) > 0) {
        result = false;

        output_result = fprintf(output_stream, _("The following new objects were built without -fPIC:\n"));
        assert(output_result > 0);

        TAILQ_FOREACH(iter, after_new, items) {
            output_result = fprintf(output_stream, "\t%s\n", iter->data);
            assert(output_result > 0);
        }
    }

    output_result = fclose(output_stream);
    assert(output_result == 0);

    init_result_params(&params);
    xasprintf(&params.msg, _("%s in %s has objects built without -fPIC on %s"), file->localpath, name, arch);
    params.waiverauth = WAIVABLE_BY_SECURITY;
    params.header = NAME_ELF;
    params.remedy = get_remedy(REMEDY_ELF_FPIC);
    params.details = screendump;
    params.arch = arch;
    params.file = file->localpath;
    params.verb = VERB_REMOVED;
    params.noun = _("missing -fPIC in ${FILE} on ${ARCH}");
    params.severity = get_secrule_result_severity(ri, file, SECRULE_PIC);

    if (!result && params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
        add_result(ri, &params);
    }

    if (params.severity == RESULT_SKIP) {
        result = true;
    }

    free(params.msg);

    list_free(after_lost_pic, free);
    list_free(after_new, free);
    list_free(after_no_pic, free);
    list_free(before_pic, free);
    list_free(before_all, free);

    free(screendump);

    return result;
}

static bool elf_regular_tests(struct rpminspect *ri, Elf *after_elf, Elf *before_elf, rpmfile_entry_t *file, const char *arch, const char *name)
{
    bool result = true;
    struct result_params params;

    init_result_params(&params);
    params.severity = get_secrule_result_severity(ri, file, SECRULE_TEXTREL);
    params.header = NAME_ELF;
    params.waiverauth = WAIVABLE_BY_SECURITY;
    params.remedy = get_remedy(REMEDY_ELF_TEXTREL);
    params.arch = arch;
    params.file = file->localpath;
    params.noun = _("TEXTREL relocations in ${FILE} on ${ARCH}");

    /* skip kernel eBPF machine type objects */
    if (get_elf_machine(after_elf) == EM_BPF) {
        DEBUG_PRINT("eBPF object encountered (%s), skipping\n", file->localpath);
        return true;
    }

    if (!inspect_elf_execstack(ri, after_elf, before_elf, file, arch)) {
        result = false;
    }

    if (has_textrel(after_elf)) {
        /* Only complain for baseline (no before), or for gaining TEXTREL between before and after. */
        if (before_elf && !has_textrel(before_elf)) {
            xasprintf(&params.msg, _("%s in %s acquired TEXTREL relocations on %s"), file->localpath, name, arch);
            params.verb = VERB_ADDED;
        } else if (!before_elf) {
            xasprintf(&params.msg, _("%s in %s has TEXTREL relocations on %s"), file->localpath, name, arch);
            params.verb = VERB_FAILED;
        }

        if (params.msg != NULL && (params.severity != RESULT_NULL && params.severity != RESULT_SKIP)) {
            add_result(ri, &params);
            result = false;
        }

        free(params.msg);
    }

    if (before_elf) {
        /* Check if we lost GNU_RELRO */
        if (!check_relro(ri, before_elf, after_elf, file, arch)) {
            result = false;
        }
    }

    return result;
}

static bool elf_driver(struct rpminspect *ri, rpmfile_entry_t *after)
{
    const char *arch = NULL;
    const char *name = NULL;
    Elf *after_elf = NULL;
    Elf *before_elf = NULL;
    int after_elf_fd = -1;
    int before_elf_fd = -1;
    bool result = true;

    name = headerGetString(after->rpm_header, RPMTAG_NAME);

    /* Skip source packages */
    if (headerIsSource(after->rpm_header)) {
        return true;
    }

    /* Skip debuginfo packages and debugsource packages */
    if (is_debuginfo_rpm(after->rpm_header) || is_debugsource_rpm(after->rpm_header)) {
        return true;
    }

    /* Skip anything that isn't a regular file */
    if (!after->fullpath || !S_ISREG(after->st.st_mode)) {
        return true;
    }

    /* Process our ignore paths from the config file */
    if (!process_file_path(after, ri->elf_path_include, ri->elf_path_exclude)) {
        return true;
    }

    /* Skip kernel modules */
    after_elf = get_elf(after, &after_elf_fd);

    if (after_elf != NULL
        && get_elf_type(after_elf) == ET_REL
        && have_elf_section(after_elf, SHT_PROGBITS, ".modinfo")
        && strsuffix(after->localpath, KERNEL_MODULE_FILENAME_EXTENSION)) {
        elf_end(after_elf);
        close(after_elf_fd);
        return true;
    }

    /* close this handle so the test can start over for .a vs .so */
    if (after_elf) {
        elf_end(after_elf);
        close(after_elf_fd);
    }

    arch = get_rpm_header_arch(after->rpm_header);

    /* Is this an archive or a regular ELF file? */
    if ((after_elf = get_elf_archive(after, &after_elf_fd)) != NULL) {
        if (after->peer_file != NULL) {
            before_elf = get_elf_archive(after->peer_file, &before_elf_fd);
        }

        result = elf_archive_tests(ri, after_elf, after_elf_fd, before_elf, before_elf_fd, after, arch, name);
    } else if ((after_elf = get_elf(after, &after_elf_fd)) != NULL) {
        if (after->peer_file != NULL) {
            before_elf = get_elf(after->peer_file, &before_elf_fd);
        }

        result = elf_regular_tests(ri, after_elf, before_elf, after, arch, name);
    }

    if (after_elf) {
        elf_end(after_elf);
        close(after_elf_fd);
    }

    if (before_elf) {
        elf_end(before_elf);
        close(before_elf_fd);
    }

    return result;
}

/**
 * @brief Perform the 'elf' inspection.
 *
 * Perform several checks on ELF files. First, check that ELF objects
 * do not contain an executable stack. Second, check that ELF objects
 * do not contain text relocations. When comparing builds, check that
 * the ELF objects in the after build did not lose a PT_GNU_RELRO
 * segment. When comparing builds, check that the ELF objects in the
 * after build did not lose -D_FORTIFY_SOURCE. Lastly, if there is a
 * list of forbidden library functions, make sure nothing uses them.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_elf(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    rip = ri;
    result = foreach_peer_file(ri, NAME_ELF, elf_driver);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_ELF;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
