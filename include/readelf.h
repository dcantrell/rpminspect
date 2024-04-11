/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file readelf.h
 * @author David Shea &lt;dshea@redhat.com&gt; and David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019
 * @brief ELF specific definitions and function prototypes.
 * @copyright LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _READELF_H
#define _READELF_H

#include <stdbool.h>
#include <libelf.h>
#include <gelf.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"

/**
 * @defgroup Constants and Other Defaults
 *
 * @{
 */

#ifndef EM_BPF
/**
 * @def EM_PDF
 *
 * Linux BPF, an in-kernel virtual machine.  This is defined on
 * systems that lack the define in the system header files.
 */
#define EM_BPF 247
#endif

/** @} */

/**
 * @defgroup Function Prototypes
 *
 * @{
 */

/**
 * @brief Get an Elf object for the named ELF shared object.
 *
 * Given a path to a file containing an ELF shared object and a
 * pointer to an int, try to open the file as an ELF_K_ELF Elf object.
 * The read-only file descriptor is also returned via the second
 * parameter (the int pointer).  On error, NULL is returned and the
 * int pointer is undefined.
 *
 * This function is used to open ELF shared libraries and executables.
 *
 * The caller is responsible for closing and destroying the Elf object
 * and closing the file descriptor.
 *
 * @param path Full path to the ELF shared object file to open.
 * @param out_fd The int pointer to hold the read-only file descriptor
 *        of path.
 * @return The Elf object for path or NULL on error.
 */
Elf *get_elf(rpmfile_entry_t *file, int *out_fd);

/**
 * @brief Get an Elf object for the named ELF archive object.
 *
 * Given a path to a file containing an ELF archive and a pointer to
 * an int, try to open the file as an ELF_K_AR Elf object.  The
 * read-only file descriptor is also returned via the second parameter
 * (the int pointer).  On error, NULL is returned and the int pointer
 * is undefined.
 *
 * This function is used to open ELF static libraries (.a files).
 *
 * The caller is responsible for closing and destroying the Elf object
 * and closing the file descriptor.
 *
 * @param path Full path to the ELF archive file to open.
 * @param out_fd The int pointer to hold the read-only file descriptor
 *        of path.
 * @return The Elf object for path or NULL on error.
 */
Elf *get_elf_archive(rpmfile_entry_t *file, int *out_fd);

/**
 * @brief Return the ELF_TYPE of the given Elf object.
 *
 * This function takes an Elf object and returns the ELF_TYPE.  The
 * ELF types are defined in elf.h in the system headers and begin with
 * ET_.  Examples include ET_DYN and ET_EXEC.
 *
 * If the type is unknown, -1 is returned.
 *
 * @param elf The Elf object.
 * @return The ELF type of the Elf object.
 */
GElf_Half get_elf_type(Elf *elf);

/**
 * @brief Return the ELF_MACHINE of the given Elf object.
 *
 * This function takes an Elf object and returns the ELF_MACHINE.  The
 * ELF types are defined in elf.h in the system headers and begin with
 * EM_.  Examples include EM_SPARC, EM_68K, and EM_PPC.
 *
 * If the type is unknown, -1 is returned.
 *
 * @param elf The Elf object.
 * @return The ELF machine type of the Elf object.
 */
GElf_Half get_elf_machine(Elf *elf);

/**
 * @brief Determine if the specified file is any ELF file type.
 *
 * Given a path to a file, this function returns true if the file is
 * either an ELF archive or an ELF file.  In all other cases, it
 * returns false.
 *
 * @param path The full path to the file in question.
 * @return True if the file is ELF, false otherwise.
 */
bool is_elf(rpmfile_entry_t *file);

/**
 * @brief Determine if the specified file is an ELF shared library.
 *
 * Given a path to a file, this function returns true if the file is
 * an ELF shared library.  That is, if it is ELF type ET_DYN.
 *
 * @param path The fullpath to the file in question.
 * @return True if the file is an ELF shared library, false otherwise.
 */
bool is_elf_shared_library(rpmfile_entry_t *file);

/**
 * @brief Determine if the specified file is an ELF executable.
 *
 * Given a path to a file, this function returns true if the file is
 * an ELF executable.  That is, if it is ELF type ET_EXEC.
 *
 * @param path The fullpath to the file in question.
 * @return True if the file is an ELF executable, false otherwise.
 */
bool is_elf_executable(rpmfile_entry_t *file);

/**
 * @brief Determine if the specified file is an ELF file.
 *
 * Given a path to a file, this function returns true if the file is
 * an ELF file.  That is, an ELF executable, shared library, or shared
 * object.
 *
 * @param path The fullpath to the file in question.
 * @return True if the file is an ELF file, false otherwise.
 */
bool is_elf_file(rpmfile_entry_t *file);

/**
 * @brief Determine if the specified file is an ELF archive.
 *
 * Given a path to a file, this function returns true if the file is
 * an ELF archive.  That is, a '.a' file consisting of ELF object
 * files.
 *
 * @param path The fullpath to the file in question.
 * @return True if the file is an ELF archive, false otherwise.
 */
bool is_elf_archive(rpmfile_entry_t *file);

/**
 * @brief Determine if the specified Elf object contains the specified
 * section.
 *
 * Given an Elf object, determine if it contains the specified
 * section.  The section may be specified by type or name or both.
 * The section type corresponds to valid values for the sh_type member
 * of a GElf_Shdr.  To not specify a type, pass -1 for that parameter.
 * To not specify a name, pass NULL for that parameter.
 *
 * @param elf The Elf object
 * @param section The ELF section type (or -1 for unspecified)
 * @param name The ELF section name (or NULL for unspecified)
 * @return True if the section exists in the Elf object, false
 *         otherwise.
 */
bool have_elf_section(Elf *elf, int64_t section, const char *name);

/**
 * @brief Given an ELF starting section, collect all section names.
 *
 * Sometimes you want to look at section names and do partial string
 * comparisons.  This function will collect all section names that
 * exist given a starting section.  The caller is responsible for
 * freeing the memory allocated with the returned list.
 *
 * @param elf The Elf object to scan.
 * @param start The starting ELF section (optional).
 * @return A string_list_t list of all ELF section names found, NULL
 *         indicates none found.
 */
string_list_t *get_elf_section_names(Elf *elf, size_t start);

/**
 * @brief Return the specified ELF section
 *
 * Given an Elf object, search for and return the section identified
 * by either section type or name or both.  If you do not want to
 * specify a section type, pass -1 for that parameter.  If you do not
 * want to specify a section name, pass NULL for that parameter.  If
 * the section is found and start and out_shdr are not NULL, the
 * function will set those according to the location of the found
 * section.  The pointer start will receive the pointer to the Elf_Scn
 * where the section was found.  The out_shdr pointer will receive the
 * GElf_Shdr containing the found section.
 *
 * @param elf The Elf object to scan.
 * @param section The ELF section type (or -1 for unspecified)
 * @param name The ELF section name (or NULL for unspecified)
 * @param start Pointer to receive Elf_Scn where section was found
 *              (optional)
 * @param out_shdr Pointer to receive GElf_Shdr where section was
 *                 found (optional)
 * @return The Elf_Scn matching the type and/or name
 */
Elf_Scn *get_elf_section(Elf *elf, int64_t section, const char *name, Elf_Scn *start, GElf_Shdr *out_shdr);

/**
 * @brief For the given SHT_SYMTAB return corresponding
 * SHT_SYMTAB_SHNDX section.
 *
 * Given an Elf object and an SHT_SYMTAB section, look for the
 * corresponding SHT_SYMTAB_SHNDX section and return it.  Also return
 * the GElf_Shdr containing the SHT_SYMTAB_SHNDX section.  If not
 * found, return NULL.
 *
 * @param elf The Elf object to scan
 * @param symtab The SHT_SYMTAB section
 * @param out_shdr Receives the pointer to the GElf_Shdr containing
 *                 the SHT_SYMTAB_SHNDX section if found.
 * @return The SHT_SYMTAB_SHNDX section or NULL if not found.
 */
Elf_Scn *get_elf_extended_section(Elf *elf, Elf_Scn *symtab, GElf_Shdr *out_shdr);

/**
 * @brief Return the program header of the given type.
 *
 * Given an Elf object, return the program header (GElf_Phdr) that
 * corresponds to type, or NULL if not found.
 *
 * @param elf The Elf object to scan
 * @param type The program header type
 * @param out The pointer to the program header
 * @return Pointer to the program header or NULL if not found
 */
GElf_Phdr *get_elf_phdr(Elf *elf, Elf64_Word type, GElf_Phdr *out);

/**
 * @brief Return DT_SONAME value of the specified file.
 *
 * Take a file path specified and read its ELF data and return the
 * DT_SONAME as a newly allocated string.  The caller must free this
 * string.  The function returns NULL if DT_SONAME is not found.
 *
 * @param filepath The path to the file to search
 * @return Newly allocated string containing the DT_SONAME value or
 *         NULL
 */
char *get_elf_soname(rpmfile_entry_t *file);

/**
 * @brief Check for tag in the specified ELF object
 *
 * Search an ELF object for a specified dynamic tag.  If found, return
 * true.  Return false otherwise.
 *
 * @param elf The Elf object to scan
 * @param tag The dynamic tag
 * @return True if found, false otherwise
 */
bool have_dynamic_tag(Elf *elf, const Elf64_Sxword tag);

/**
 * @brief Check for the specified DT_FLAG flag in an ELF object
 *
 * Check for the specified DT_FLAG flag in d_val for the Elf object.
 *
 * @param elf The Elf object to scan
 * @param flag The dynamic flag
 * @return True if found, false otherwise
 */
bool have_dynamic_flag(Elf *elf, const Elf64_Sxword flag);

/**
 * @brief Return the requested dynamic tags.
 *
 * If out is not NULL, it will get a pointer to an array of matching
 * dynamic tags and the array size will be written to out_size.
 *
 * If shdr_out is not NULL, it will get a copy of the .dynamic section
 * header.
 *
 * @param elf The Elf object to scan
 * @param tag The dynamic tag
 * @param out Array to hold dynamic tags found (optional)
 * @param out_size Number of elements in out (optional)
 * @param shdr_out The .dynamic section header
 */
bool get_dynamic_tags(Elf *elf, const Elf64_Sxword tag, GElf_Dyn **out, size_t *out_size, GElf_Shdr *shdr_out);

/**
 * @brief Return a list of symbols in an ELF object's .dynsym section.
 *
 * Returns a list of symbols used by an ELF object's .dynsym section.
 * If filter is not NULL, only the symbol's returned true by the
 * filter will be added.  The filter function just takes a const char
 * * as input and returns a bool.  The input will be each member of
 * the .dynsym section.  The memory pointed to by the list elements is
 * owned by the Elf * context.  The list itself should be free with
 * list_free(list, NULL) by the caller.
 *
 * @param elf The Elf object to scan
 * @param filter Optional filter callback function
 * @return List of symbols used by the .dynsym section
 */
string_list_t *get_elf_imported_functions(Elf *elf, bool (*filter)(const char *));

/**
 * @brief Return a list of symbols in an ELF object's .symtab section.
 *
 * Returns a list of symbols used by an ELF object's .symtab section.
 * If filter is not NULL, only the symbol's returned true by the
 * filter will be added.  The filter function just takes a const char
 * * as input and returns a bool.  The input will be each member of
 * the .symtab section.  The memory pointed to by the list elements is
 * owned by the Elf * context.  The list itself should be free with
 * list_free(list, NULL) by the caller.
 *
 * @param elf The Elf object to scan
 * @param filter Optional filter callback function
 * @return List of symbols used by the .symtab section
 */
string_list_t *get_elf_exported_functions(Elf *elf, bool (*filter)(const char *));

/**
 * @typedef elf_ar_action
 *
 * Function type for the action callback used by elf_archive_iterate.
 *
 * @param elf The ELF archive member
 * @param user_data Optional user_data
 * @return True to continue iteration, false to stop
 */
typedef bool (*elf_ar_action)(Elf *elf, string_list_t **user_data);

/**
 * @brief Iterate over an ELF archive performing an action
 *
 * Iterate over an ELF archive performing the specified action on each
 * member until the end of the archive is reached or the action
 * returns false.
 *
 * @param fd The file descriptor of the archive to iterate
 * @param archive The ELF archive object
 * @param action The action to perform on each member
 * @param user_data User data passed to action (optional)
 */
void elf_archive_iterate(int fd, Elf *archive, elf_ar_action action, string_list_t **user_data);

/** @} */

#endif

#ifdef __cplusplus
}
#endif
