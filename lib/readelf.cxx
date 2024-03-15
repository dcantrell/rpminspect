/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <dlfcn.h>
#include <link.h>

#include <gelf.h>
#include <libelf.h>
#include <ar.h>

#include "queue.h"
#include "readelf.h"
#include "rpminspect.h"

static GElf_Half _get_elf_helper(Elf *elf, elfinfo_t type, GElf_Half fail)
{
    GElf_Ehdr ehdr;
    Elf_Kind kind;

    assert(elf != NULL);
    kind = elf_kind(elf);
    assert(kind == ELF_K_ELF);

    if (gelf_getehdr(elf, &ehdr) == NULL) {
        warn("*** gelf_getehdr");
        return fail;
    }

    if (type == ELF_TYPE) {
        return ehdr.e_type;
    } else if (type == ELF_MACHINE) {
        return ehdr.e_machine;
    } else {
        warnx(_("*** unknown elftype_t %d"), type);
        return fail;
    }

    return fail;
}

GElf_Half get_elf_type(Elf *elf)
{
    return _get_elf_helper(elf, ELF_TYPE, ET_NONE);
}

GElf_Half get_elf_machine(Elf *elf)
{
    return _get_elf_helper(elf, ELF_MACHINE, EM_NONE);
}

static Elf *get_elf_with_kind(const char *fullpath, int *out_fd, Elf_Kind kind)
{
    int fd;
    Elf *elf = NULL;
    struct stat sbuf;

    static bool initialized = false;

    /* library version check */
    if (!initialized) {
        if (elf_version(EV_CURRENT) == EV_NONE) {
            warnx(_("*** libelf version mismatch"));
            return NULL;
        }

        initialized = true;
    }

    /* make sure this is a regular file */
    if (lstat(fullpath, &sbuf) != 0) {
        warn("*** lstat");
        return NULL;
    }

    if (!S_ISREG(sbuf.st_mode)) {
        return NULL;
    }

    /* verify we can access the file and it's an ELF object */
    if ((fd = open(fullpath, O_RDONLY)) == -1) {
        return NULL;
    }

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);

    if (elf_kind(elf) == kind) {
        *out_fd = fd;
        return elf;
    }

    /* clean up */
    elf_end(elf);
    close(fd);
    return NULL;
}

/* Return a read-only Elf object for the given path, or NULL on error or if the path is not an ELF object.
 * On success, the opened file descriptor is written to out_fd.
 */
Elf * get_elf(const char *fullpath, int *out_fd)
{
    return get_elf_with_kind(fullpath, out_fd, ELF_K_ELF);
}

/* Like above, but verifies that the file is an archive instead of an ELF file */
Elf * get_elf_archive(const char *fullpath, int *out_fd)
{
    return get_elf_with_kind(fullpath, out_fd, ELF_K_AR);
}

/*
 * Return true if a specified file is ELF, false otherwise.
 */
bool is_elf(const char *path)
{
    return (is_elf_file(path) || is_elf_archive(path));
}

static bool _is_elf_type(const char *path, int type)
{
    bool result = false;
    int fd = 0;
    Elf *elf = NULL;

    assert(path != NULL);

    elf = get_elf(path, &fd);

    if (elf && get_elf_type(elf) == type) {
        result = true;
    }

    close(fd);
    elf_end(elf);
    return result;
}

/*
 * Return true if a specified file is an ELF shared library file, that
 * is, of type ET_DYN.
 */
bool is_elf_shared_library(const char *path)
{
    return _is_elf_type(path, ET_DYN);
}

/*
 * Return true if a specified file is an ELF executable file, that
 * is, of type ET_EXEC.
 */
bool is_elf_executable(const char *path)
{
    return _is_elf_type(path, ET_EXEC);
}

/*
 * Return true for any ELF file.
 */
bool is_elf_file(const char *path)
{
    int fd = 0;
    Elf *elf = NULL;

    /* try it as a shared object */
    elf = get_elf(path, &fd);

    if (elf) {
        elf_end(elf);

        if (fd) {
            close(fd);
        }

        return true;
    }

    return false;
}

/*
 * Return true for any ELF archive.
 */
bool is_elf_archive(const char *path)
{
    int fd = 0;
    Elf *elf = NULL;

    /* try it as a static library */
    elf = get_elf_archive(path, &fd);

    if (elf) {
        elf_end(elf);

        if (fd) {
            close(fd);
        }

        return true;
    }

    return false;
}

bool have_elf_section(Elf *elf, int64_t section, const char *name)
{
    return get_elf_section(elf, section, name, NULL, NULL) != NULL;
}

string_list_t *get_elf_section_names(Elf *elf, size_t start)
{
    size_t shstrndx;
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    char *section_name = NULL;
    string_list_t *names = NULL;

    assert(elf != NULL);

    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        return NULL;
    }

    /* get the starting section */
    scn = elf_getscn(elf, start);

    if (scn == NULL) {
        return NULL;
    }

    /* iterate over the sections collecting the names */
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        /* get this header information */
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            list_free(names, free);
            return NULL;
        }

        section_name = elf_strptr(elf, shstrndx, shdr.sh_name);

        /* copy this section name in to the list */
        if (section_name != NULL) {
            names = list_add(names, section_name);
        }
    }

    return names;
}

/*
 * Look through an ELF object by section for a section by the given ID and
 * the specified name.  At least one parameter is required.  To not specify
 * a section number, pass in -1.  To not specify a section name, pass
 * in NULL.
 */
Elf_Scn * get_elf_section(Elf *elf, int64_t section, const char *name, Elf_Scn *start, GElf_Shdr *out_shdr)
{
    size_t shstrndx;
    char *section_name = NULL;
    Elf_Scn *scn = start;
    GElf_Shdr shdr;
    GElf_Shdr *shdrp = (out_shdr != NULL ? out_shdr : &shdr);

    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        return NULL;
    }

    /* we have an ELF object, iterate over the sections */
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        /* get this header information */
        if (gelf_getshdr(scn, shdrp) != shdrp) {
            return NULL;
        }

        section_name = elf_strptr(elf, shstrndx, shdrp->sh_name);

        /* Check if this section matches the requested type and name */
        if (((section < 0) || (shdrp->sh_type == (GElf_Word) section)) &&
            ((name == NULL) || ((section_name != NULL) &&
                                !strcmp(name, section_name)))) {
            return scn;
        }
    }

    return NULL;
}

bool have_dynamic_tag(Elf *elf, const Elf64_Sxword tag)
{
    return get_dynamic_tags(elf, tag, NULL, NULL, NULL);
}

/* Return the requested dynamic tags.
 *
 * If out is not NULL, it will get a pointer to an array of matching dynamic tags, and the
 * array size will be written to *out_size.
 *
 * If shdr_out is not NULL, it will get a copy of the .dynamic section header.
 */
bool get_dynamic_tags(Elf *elf, const Elf64_Sxword tag, GElf_Dyn **out, size_t *out_size, GElf_Shdr *shdr_out)
{
    Elf_Scn *dyn_section;
    GElf_Shdr shdr;
    Elf_Data *data = NULL;
    GElf_Dyn dyn;
    GElf_Dyn *dyn_tmp;

    size_t entry_size;
    size_t i;

    bool found = false;

    dyn_section = get_elf_section(elf, SHT_DYNAMIC, ".dynamic", NULL, &shdr);

    if (dyn_section == NULL) {
        return false;
    }

    if (shdr_out != NULL) {
        memmove(shdr_out, &shdr, sizeof(*shdr_out));
    }

    if (out != NULL) {
        *out = NULL;
        *out_size = 0;
    }

    while ((data = elf_getdata(dyn_section, data)) != NULL) {
        entry_size = gelf_fsize(elf, data->d_type, 1, EV_CURRENT);

        for (i = 0; i < (shdr.sh_size / entry_size); i++) {
            if (gelf_getdyn(data, i, &dyn) == NULL) {
                continue;
            }

            if (dyn.d_tag == tag) {
                /* If no data was requested, just return now */
                if (out == NULL) {
                    return true;
                }

                found = true;

                /*
                 * alloc one more GElf_Dyn worth of memory and
                 * add to the end of the array
                 */
                dyn_tmp = realloc(*out, ((*out_size) + 1) * (sizeof(GElf_Dyn)));
                assert(dyn_tmp != NULL);
                memmove(dyn_tmp + *out_size, &dyn, sizeof(GElf_Dyn));
                *out = dyn_tmp;
                (*out_size)++;
            }
        }
    }

    return found;
}

bool have_dynamic_flag(Elf *elf, const Elf64_Sxword flag)
{
    Elf_Scn *dyn_section;
    GElf_Shdr shdr;
    Elf_Data *data = NULL;
    GElf_Dyn dyn;
    size_t entry_size;
    size_t i;

    assert(elf != NULL);

    dyn_section = get_elf_section(elf, SHT_DYNAMIC, ".dynamic", NULL, &shdr);

    if (dyn_section == NULL) {
        return false;
    }

    while ((data = elf_getdata(dyn_section, data)) != NULL) {
        entry_size = gelf_fsize(elf, data->d_type, 1, EV_CURRENT);

        for (i = 0; i < (shdr.sh_size / entry_size); i++) {
            if (gelf_getdyn(data, i, &dyn) == NULL) {
                continue;
            }

            if (dyn.d_tag == DT_FLAGS && (dyn.d_un.d_val & flag)) {
                return true;
            }
        }
    }

    return false;
}

/* For the given SHT_SYMTAB section, look for a corresponding SHT_SYMTAB_SHNDX section */
Elf_Scn * get_elf_extended_section(Elf *elf, Elf_Scn *symtab, GElf_Shdr *out_shdr)
{
    size_t scnndx;
    Elf_Scn *xndxscn = NULL;
    GElf_Shdr xndxshdr;

    if ((scnndx = elf_ndxscn(symtab)) == SHN_UNDEF) {
        return NULL;
    }

    while ((xndxscn = get_elf_section(elf, SHT_SYMTAB_SHNDX, NULL, xndxscn, &xndxshdr)) != NULL) {
        if (xndxshdr.sh_link == scnndx) {
            if (out_shdr != NULL) {
                memcpy(out_shdr, &xndxshdr, sizeof(*out_shdr));
            }

            return xndxscn;
        }
    }

    return NULL;
}

/* Returns the program header of the given type
 * Returns NULL on error, otherwise returns the out pointer
 */
GElf_Phdr * get_elf_phdr(Elf *elf, Elf64_Word type, GElf_Phdr *out)
{
    unsigned int i;
    size_t phnum;

    if (elf_getphdrnum(elf, &phnum) < 0) {
        return NULL;
    }

    for (i = 0; i < phnum; i++) {
        if (gelf_getphdr(elf, i, out) == NULL) {
            return NULL;
        }

        if (out->p_type == type) {
            return out;
        }
    }

    return NULL;
}

/*
 * Returns the SONAME of the given file or NULL if unable.
 * Caller must free the returned string.
 */
char *get_elf_soname(const char *filepath)
{
    char *soname = NULL;
    Elf *e = NULL;
    int fd = 0;
    GElf_Dyn *tags = NULL;
    GElf_Shdr shdr;
    size_t sz = 0;
    bool found = false;

    assert(filepath != NULL);

    if ((e = get_elf(filepath, &fd)) == NULL) {
        close(fd);
        return NULL;
    }

    found = get_dynamic_tags(e, DT_SONAME, &tags, &sz, &shdr);

    /*
     * Expect exactly one SONAME, if we have more than that then the
     * ELF format changed and the world is strange and confusing.
     */
    if (found && sz == 1) {
        soname = strdup(elf_strptr(e, shdr.sh_link, (size_t) tags[0].d_un.d_ptr));
    }

    free(tags);
    elf_end(e);
    close(fd);
    return soname;
}

static string_list_t *get_elf_symbol_list(Elf *elf, bool (*filter)(const char *), uint32_t sh_type, const char *table_name)
{
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    Elf_Data *data = NULL;
    Elf_Scn *xndxscn = NULL;
    GElf_Shdr xndxshdr;
    Elf_Data *xndxdata = NULL;
    GElf_Sym sym;
    size_t nentries = 0;
    size_t i = 0;
    char *symstr = NULL;
    string_list_t *list = NULL;

    /* get the .dynsym section */
    if ((scn = get_elf_section(elf, sh_type, table_name, NULL, &shdr)) == NULL) {
        return list;
    }

    /* If this is a .symtab, look for an extended index table for this section */
    if (sh_type == SHT_SYMTAB) {
        xndxscn = get_elf_extended_section(elf, scn, &xndxshdr);
    }

    /* Get the section data */
    if ((data = elf_getdata(scn, NULL)) == NULL) {
        list_free(list, NULL);
        return NULL;
    }

    if ((xndxscn != NULL) && ((xndxdata = elf_getdata(xndxscn, NULL)) == NULL)) {
        list_free(list, NULL);
        return NULL;
    }

    /* Iterate over each symbol */
    assert(shdr.sh_entsize > 0);
    nentries = shdr.sh_size / shdr.sh_entsize;

    for (i = 0; i < nentries; i++) {
        if (gelf_getsymshndx(data, xndxdata, i, &sym, NULL) == NULL) {
            continue;
        }

        /* Fetch the name from the string table */
        if ((symstr = elf_strptr(elf, shdr.sh_link, sym.st_name)) == NULL) {
            continue;
        }

        /* Add the symbol to the head of the list */
        if ((filter == NULL) || filter(symstr)) {
            list = list_add(list, symstr);
        }
    }

    return list;
}

/* Returns a list of symbols used by an ELF object's .dynsym section.
 * If filter is not NULL, only the symbol's returned true by the filter will be added.
 *
 * The memory pointed to by the list elements is owned by the Elf* context. The list
 * itself should be freed with list_free(<list>, NULL) by the caller.
 */
string_list_t * get_elf_imported_functions(Elf *elf, bool (*filter)(const char *))
{
    return get_elf_symbol_list(elf, filter, SHT_DYNSYM, ".dynsym");
}

/* Returns a list of symbols exported by an ELF object's .symtab section.
 * Same parameter and return value semantics as get_elf_imported_functions.
 */
string_list_t * get_elf_exported_functions(Elf *elf, bool (*filter)(const char *))
{
    return get_elf_symbol_list(elf, filter, SHT_SYMTAB, ".symtab");
}

/* Iterate over an archive, performing action on each member until the end
 * of the archive is reached or action returns false.
 */
void elf_archive_iterate(int fd, Elf *archive, elf_ar_action action, string_list_t **user_data)
{
    Elf_Cmd cmd = ELF_C_READ_MMAP_PRIVATE;
    Elf *elf;
    bool result;

    while ((elf = elf_begin(fd, cmd, archive)) != NULL) {
        result = action(elf, user_data);
        cmd = elf_next(elf);
        elf_end(elf);

        if (!result) {
            break;
        }
    }

    /* Rewind the archive */
    elf_rand(archive, SARMAG);

    return;
}
