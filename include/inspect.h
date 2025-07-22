/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect.h
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief Test functions provided by librpminspect and result constants.
 * @copyright LGPL-3.0-or-later
 *
 * All inspection drivers use the same prototype.  They return a bool
 * and take one parameter:  a struct rpminspect pointer.  Inspection
 * drivers are named as "inspect_NAME" where NAME is the short name
 * specified in the inspections array (see inspect.c).  The source
 * code filename should also match the driver function name.
 *
 * Add any helper functions you need in order to keep the inspection
 * driver simple.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <libelf.h>

#ifdef _WITH_LIBKMOD
#ifdef _LIBKMOD_HEADER_SUBDIR
#include <kmod/libkmod.h>
#else
#include <libkmod.h>
#endif /* _LIBKMOD_HEADER_HEADER */
#endif /* _WITH_LIBKMOD */

#include "types.h"

#ifndef _LIBRPMINSPECT_INSPECT_H
#define _LIBRPMINSPECT_INSPECT_H

/**
 * @defgroup INSPECT General inspection utility functions.
 *
 * @{
 */

/**
 * @brief Iterate over each file in each package in a build.
 *
 * Inspect each "after" file in each peer of an inspection.  If the
 * foreach_peer_file_func returns false for any file, the result will
 * be false.  foreach_peer_file_func is run on each file even if an
 * earlier file fails. This allows for multiple errors to be collected
 * for a single inspection.
 *
 * @param ri Pointer to the struct rpminspect used for the program.
 * @param inspection Name of the currently running inspection.
 * @param callback Callback function to iterate over each file.
 * @return True if the check_fn passed for each file, false otherwise.
 */
bool foreach_peer_file(struct rpminspect *ri, const char *inspection, foreach_peer_file_func check_fn);

/**
 * @brief Return inspection ID given its name string.
 *
 * Given an inspection name as a string (e.g., from a command line
 * option), convert it to its corresponding ID value.  If the string
 * given is not a valid inspection name, the function returns
 * INSPECT_NULL.
 *
 * @param name Inspection name.
 * @return The inspection ID as a uint64_t or INSPECT_NULL.
 */
uint64_t inspection_id(const char *name);

/**
 * @brief Return inspection description string given its ID.
 *
 * Return the long description for the specified inspection or NULL if
 * given an invalid inspection ID.  NOTE: The caller must not free the
 * returned string.
 *
 * @param id Inspection ID constant.
 * @return The inspection description or NULL.
 */
const char *inspection_desc(const uint64_t id);

/**
 * @brief Return inspection description string given its results
 * header.
 *
 * Return the long description for the specified inspection or NULL if
 * the given header string is invalid.  NOTE: The caller must not free
 * the returned string.
 *
 * @param header Inspection header string.
 * @return The inspection description or NULL.
 */
const char *inspection_header_to_desc(const char *header);

/** @} */

/**
 * @defgroup INSPECTIONS Individual inspection functions
 *
 * @{
 */

/**
 * @brief Perform the 'elf' inspection.
 *
 * Perform several checks on ELF files. First, check that ELF objects
 * do not contain an executable stack. Second, check that ELF objects
 * do not contain text relocations. When comparing builds, check that
 * the ELF objects in the after build did not lose a PT_GNU_RELRO
 * segment. When comparing builds and there is a list of forbidden
 * library functions, make sure nothing uses them.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_elf(struct rpminspect *ri);

/**
 * @brief Perform the 'license' inspection.
 *
 * Verify the string specified in the License tag of the RPM metadata
 * describes permissible software licenses as defined by the license
 * database . Also checks to see if the License tag contains any
 * unprofessional words as defined in the configuration file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_license(struct rpminspect *ri);

/**
 * @brief Perform the 'emptyrpm' inspection.
 *
 * Check all binary RPMs in the build for any empty payloads. When
 * comparing two builds, report new packages in the after build with
 * empty payloads.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_emptyrpm(struct rpminspect *ri);

/**
 * @brief Perform the 'xml' inspection.
 *
 * Check that XML files included in the RPM payload are well-formed.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_xml(struct rpminspect *ri);

/**
 * @brief Perform the 'manpage' inspection.
 *
 * Perform some checks on man pages in the RPM payload. First, check
 * that each man page is compressed. Second, check that each man page
 * contains valid content. Lastly, check that each man page is
 * installed to the correct path.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_manpage(struct rpminspect *ri);

/**
 * @brief Perform the 'metadata' inspection.
 *
 * Perform some RPM header checks. First, check that the Vendor
 * contains the expected string as defined in the configuration
 * file. Second, check that the build host is in the expected
 * subdomain as defined in the configuration file. Third, check the
 * Summary string for any unprofessional words. Fourth, check the
 * Description for any unprofessional words. Lastly, if there is a
 * before build specified, check for differences between the before
 * and after build values of the pr evious RPM header values and
 * report them.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_metadata(struct rpminspect *ri);

/**
 * @brief Perform the 'desktop' inspection.
 *
 * Perform syntax and file reference checks on *.desktop files. Syntax
 * errors and invalid file references are reported as errors.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_desktop(struct rpminspect *ri);

/**
 * @brief Perform the 'disttag' inspection.
 *
 * Check that the 'Release' tag in the RPM spec file includes the
 * %{?dist} directive.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_disttag(struct rpminspect *ri);

/**
 * @brief Perform the 'specname' inspection.
 *
 * Ensure the spec file name conforms to the NAME.spec naming format.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_specname(struct rpminspect *ri);

/**
 * @brief Perform the 'modularity' inspection.
 *
 * Ensure compliance with modularity build and packaging policies
 * (only valid for module builds, no-op otherwise).
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_modularity(struct rpminspect *ri);

/**
 * @brief Perform the 'javabytecode' inspection.
 *
 * Check minimum required Java bytecode version in class files, report
 * bytecode version changes between builds, and report if bytecode
 * versions are exceeded.  The bytecode version is vendor specific to
 * releases and defined in the configuration file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_javabytecode(struct rpminspect *ri);

/**
 * @brief Perform the 'changedfiles' inspection.
 *
 * Report changed files from the before build to the after build.
 * Certain file changes will raise additional warnings if the concern
 * is more critical than just reporting changes (e.g., a suspected
 * security impact).  Any gzip, bzip2, or xz compressed files will
 * have their uncompressed content compared only, which will allow
 * changes through in the compression level used.  Message catalog
 * files (.mo) are unpacked and compared using diff(1).  Public C and
 * C++ header files are preprocessed and compared using diff(1).  Any
 * changes with diff output are included in the results.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_changedfiles(struct rpminspect *ri);

/**
 * @brief Perform the 'movedfiles' inspection.
 *
 * Report files that have moved installation paths or across
 * subpackages between builds.  Files moved with a security path
 * prefix generate special reporting in case a security review is
 * required.  Rebased packages report these findings at the INFO level
 * while non-rebased packages report them at the VERIFY level or
 * higher.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_movedfiles(struct rpminspect *ri);

/**
 * @brief Perform the 'removedfiles' inspection.
 *
 * Report removed files from the before build to the after build.
 * Shared libraries get additional reporting output as they may be
 * unexpected dependency removals.  Files removed with a security path
 * prefix generate special reporting in case a security review is
 * required.  Source RPMs and debuginfo files are ignored by this
 * inspection.  Rebased packages report these findings at the INFO
 * level while non-rebased packages report them at the VERIFY level or
 * higher.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_removedfiles(struct rpminspect *ri);

/**
 * @brief Perform the 'addedfiles' inspection.
 *
 * Report added files from the before build to the after build.
 * Debuginfo files are ignored as are files that match the patterns
 * defined in the configuration file.  Files added to security paths
 * generate special reporting in case a security review is required.
 * New setuid and setgid files raise a security warning unless the
 * file is in the fileinfo list.  Rebased packages report these
 * findings at the INFO level while non-rebased packages report them
 * at the VERIFY level or higher.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_addedfiles(struct rpminspect *ri);

/**
 * @brief Perform the 'upstream' inspection.
 *
 * Report Source archives defined in the RPM spec file changing
 * content between the before and after build. If the source archives
 * change and the package is on the rebaseable list, the change is
 * reported as informational. Otherwise the change is reported as a
 * rebase of the package and requires inspection.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_upstream(struct rpminspect *ri);

/**
 * @brief Perform the 'ownership' inspection.
 *
 * Report files and directories owned by unexpected users and
 * groups. Check to make sure executables are owned by the correct
 * user and group. If a before and after build have been specified,
 * also report ownership changes.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_ownership(struct rpminspect *ri);

/**
 * @brief Perform the 'shellsyntax' inspection.
 *
 * For all shell scripts in the build, perform a syntax check on it
 * using the shell defined in its #! line (shell must also be listed
 * in shell section of the configuration data). If the syntax check
 * returns non-zero, report it to the user and return a combined
 * stdout and stderr. If comparing two builds, perform the previous
 * check but also report if a previously bad script is now passing the
 * syntax check.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_shellsyntax(struct rpminspect *ri);

/**
 * @brief Perform the 'annocheck' inspection.
 *
 * Perform annocheck tests defined in the configuration file on all
 * ELF files in the build.  A single build specified will perform an
 * analysis only.  Two builds specified will compare the test results
 * between the before and after build.  If no annocheck tests are
 * defined in the configuration file, this inspection is skipped.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_annocheck(struct rpminspect *ri);

/**
 * @brief Perform the 'dsodeps' inspection.
 *
 * Compare DT_NEEDED entries in dynamic ELF executables and shared
 * libraries between the before and after build and report changes.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_dsodeps(struct rpminspect *ri);

/**
 * @brief Perform the 'filesize' inspection.
 *
 * Report file size changes between builds.  If empty files became
 * non-empty or non-empty files became empty, report those as results
 * needing verification.  Report file change percentages as info-only.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_filesize(struct rpminspect *ri);

/**
 * @brief Perform the 'permissions' inspection.
 *
 * Report stat(2) mode changes between builds.  Checks against the
 * fileinfo list for the product release specified or determined.  Any
 * setuid or setgid changes will raise a message requiring Security
 * Team review.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_permissions(struct rpminspect *ri);

/**
 * @brief Perform the 'capabilities' inspection.
 *
 * Report capabilities(7) changes between builds.  Checks against the
 * capabilities list for the product release specified or determined.
 * Any capabilities changes not listed will raise a message requiring
 * Security Team review.  Only available if rpminspect was built with
 * libcap support.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
#ifdef _WITH_LIBCAP
bool inspect_capabilities(struct rpminspect *ri);
#endif

/**
 * @brief Perform the 'kmod' inspection.
 *
 * Report kernel module parameter, dependency, PCI ID, or symbol
 * differences between builds.  Added and removed parameters are
 * reported and if the package version is unchanged, these messages
 * are reported as failures.  The same is true module dependencies,
 * PCI IDs, and symbols.  Only available if rpminspect was built with
 * libkmod support.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
#ifdef _WITH_LIBKMOD
bool inspect_kmod(struct rpminspect *ri);
#endif

/**
 * @brief Perform the 'arch' inspection.
 *
 * Report RPM architectures that appear and disappear between the
 * before and after builds.  This inspection does not report the loss
 * of noarch packages.  The purpose of this inspection is to report
 * the loss of target machine architectures as provided in the before
 * build and not seeing them in the after build.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_arch(struct rpminspect *ri);

/**
 * @brief Perform the 'subpackages' inspection.
 *
 * Report RPM subpackages that appear and disappear between the before
 * and after builds.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_subpackages(struct rpminspect *ri);

/**
 * @brief Perform the 'changelog' inspection.
 *
 * Ensure packages contain an entry in the %changelog for the version
 * built.  Reports any other differences in the existing changelog
 * between builds and that the new entry contains new text entries.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_changelog(struct rpminspect *ri);

/**
 * @brief Perform the 'pathmigration' inspection.
 *
 * Report files that are packaged in directories that are no longer
 * used by the product.  Usually this means a package has not been
 * updated to account for path migrations.  The main examples are /bin
 * migrating to /usr/bin and /sbin migrating to /usr/sbin.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_pathmigration(struct rpminspect *ri);

/**
 * @brief Main driver for the 'lto' inspection.
 *
 * Link Time Optimization (LTO) produces smaller and faster shared ELF
 * executables and libraries.  LTO bytecode is not stable from one
 * release of gcc to the next.  As such, LTO bytecode should not be
 * present in .a and .o ELF objects shipped in packages.  This
 * inspection looks for LTO bytecode in ELF relocatable objects and
 * reports if any is present.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_lto(struct rpminspect *ri);

/**
 * @brief Main driver for the 'symlinks' inspection.
 *
 * Symbolic links must be resolvable on the installed system.  This
 * inspection ensures absolute and relative symlinks are valid.  It
 * also checks for any symlink usage that will cause problems for RPM.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_symlinks(struct rpminspect *ri);

/**
 * @brief Main driver for the 'lostpayload' inspection.
 *
 * Check all binary RPMs in the before and after builds for any empty
 * payloads. Packages that lost payload data from the before build to
 * the after build are reported.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_lostpayload(struct rpminspect *ri);

/**
 * @brief Main driver for the 'files' inspection.
 *
 * Check %files sections in the spec file for any forbidden path
 * references.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_files(struct rpminspect *ri);

/**
 * @brief Main driver for the 'types' inspection.
 *
 * Compare MIME types of files between builds and report any changes
 * for verification.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_types(struct rpminspect *ri);

/**
 * @brief Main driver for the 'abidiff' inspection.
 *
 * When comparing two builds or two packages, compare ELF files using
 * abidiff(1) from the libabigail project.  Differences are reported.
 * If the package is a rebase and not on the rebaseable list and the
 * rebase inspection is enabled, ABI differences are reported as
 * failures.  The assumption here is that rpminspect is comparing
 * builds for maintenance purposes and you do not want to introduce
 * any ABI changes for users.  If you do not care about that, turn off
 * the abidiff inspection or add the package name to the rebaseable
 * list.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_abidiff(struct rpminspect *ri);

/**
 * @brief Main driver for the 'kmidiff' inspection.
 *
 * kmidiff compares the binary Kernel Module Interfaces of two Linux
 * kernel trees.  The binary KMI is the interface that the Linux
 * kernel exposes to its modules.  The trees we are interested in here
 * are the result of the build of the Linux kernel source tree.  If
 * the builds compared are not considered a rebase, an incompatible
 * change reported by kmidiff is reported for verification.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_kmidiff(struct rpminspect *ri);

/**
 * @brief Main driver for the 'config' inspection.
 *
 * Check for and report differences in configuration files marked with
 * %config in the spec file.  If changes are whitespace or formatting
 * only, the result is reported at the INFO level.  Content and
 * location changes are reporting at the VERIFY level unless the
 * comparison is between rebased packages.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_config(struct rpminspect *ri);

/**
 * @brief Main driver for the 'doc' inspection.
 *
 * Report changes in %doc marked files.  Files that have been added or
 * removed as documentation files, for instance.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_doc(struct rpminspect *ri);

/**
 * @brief Main driver for the 'patches' inspection.
 *
 * Inspects all patches defined in the spec file and reports changes
 * between builds.  At the INFO level, rpminspect reports patch file
 * count, line count, and patch size changes.  If thresholds are
 * reached regarding a change in the patch size or the number of files
 * the patch touches, rpminspect reports the change at the VERIFY
 * level unless the comparison is for a rebase.  The configuration
 * file can also list patch names that rpminspect should ignore during
 * the inspection.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_patches(struct rpminspect *ri);

/**
 * @brief Main driver for the 'virus' inspection.
 *
 * Performs a virus scan on every file in the build using libclamav.
 * Anything found by libclamav will fail the inspection.  The
 * ignore_path rules are not used in this inspection.  All files are
 * scanned.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_virus(struct rpminspect *ri);

/**
 * @brief Main driver for the 'politics' inspection.
 *
 * Check for known politically sensitive files in packages and report
 * if they are allowed or prohibited.  The rules come from the
 * politics/ subdirectory in the rpminspect-data package for the
 * product.  Files in politics/ subdirectory map to the product
 * release string.  The rules in each file define the politically
 * sensitive allow and deny rules for that release.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_politics(struct rpminspect *ri);

/**
 * @brief Main driver for the 'badfuncs' inspection.
 *
 * Check for forbidden functions in ELF files.  Forbidden functions
 * are defined in the runtime configuration files.  Usually this
 * inspection is used to catch built packages that make use of
 * deprecated API functions if you wish built packages to conform to
 * replacement APIs.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_badfuncs(struct rpminspect *ri);

/**
 * @brief Main driver for the 'runpath' inspection.
 *
 * Check for forbidden paths in both the DT_RPATH and DT_RUNPATH
 * settings in ELF shared objects.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_runpath(struct rpminspect *ri);

/**
 * @brief Main driver for the 'unicode' inspection.
 *
 * Scan extracted and patched source code files, scripts, and RPM spec
 * files for any prohibited Unicode code points, as defined in the
 * configuration file.  Any prohibited code points are reported as a
 * possible security risk.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_unicode(struct rpminspect *ri);

/**
 * @brief Main driver for the 'rpmdeps' inspection.
 *
 * Check for correct RPM dependency metadata.  Report incorrect or
 * conflicting findings as well as expected changes when comparing a
 * new build to an older build.  Changes are only reported when
 * comparing builds, but this inspection will check for correct RPM
 * dependency metadata when inspecting a single build and report
 * findings.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_rpmdeps(struct rpminspect *ri);

/**
 * @brief Main driver for the 'debuginfo' inspection.
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_debuginfo(struct rpminspect *ri);

/**
 * @brief Perform the 'udevrules' inspection.
 *
 * Perform syntax check on udev rules files using udevadm verify.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_udevrules(struct rpminspect *ri);

/**
 * @brief Perform the 'filesmatch' inspection.
 *
 * Perform path glob matching for every file in binary RPMs with the
 * glob patterns specified in the %files section(s) of the spec file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_filesmatch(struct rpminspect *ri);

/** @} */

/**
 * @defgroup INSPECTION_FLAGS Inspection flag values
 *
 * Inspections are referenced by flag.  These flags are set in bitfields
 * to indicate which ones we want to run.  When adding new ones, please
 * follow the existing convention.  Inspection names should be short but
 * descriptive.  Note that inspection names need to specified on the
 * command line.
 *
 * @{
 */

/**
 * @def INSPECT_NULL
 * NULL inspection, which also means an invalid inspection.
 */
#define INSPECT_NULL                        ((uint64_t) 0)

/**
 * @def INSPECT_LICENSE
 * 'license' inspection ID.
 */
#define INSPECT_LICENSE                     (((uint64_t) 1) << 1)

/**
 * @def INSPECT_EMPTYRPM
 * 'emptyrpm' inspection ID.
 */
#define INSPECT_EMPTYRPM                    (((uint64_t) 1) << 2)

/**
 * @def INSPECT_METADATA
 * 'metadata' inspection ID.
 */
#define INSPECT_METADATA                    (((uint64_t) 1) << 3)

/**
 * @def INSPECT_MANPAGE
 * 'manpage' inspection ID.
 */
#define INSPECT_MANPAGE                     (((uint64_t) 1) << 4)

/**
 * @def INSPECT_XML
 * 'xml' inspection ID.
 */
#define INSPECT_XML                         (((uint64_t) 1) << 5)

/**
 * @def INSPECT_ELF
 * 'elf' inspection ID.
 */
#define INSPECT_ELF                         (((uint64_t) 1) << 6)

/**
 * @def INSPECT_DESKTOP
 * 'desktop' inspection ID.
 */
#define INSPECT_DESKTOP                     (((uint64_t) 1) << 7)

/**
 * @def INSPECT_DISTTAG
 * 'disttag' inspection ID.
 */
#define INSPECT_DISTTAG                     (((uint64_t) 1) << 8)

/**
 * @def INSPECT_SPECNAME
 * 'specname' inspection ID.
 */
#define INSPECT_SPECNAME                    (((uint64_t) 1) << 9)

#ifdef _HAVE_MODULARITYLABEL

/**
 * @def INSPECT_MODULARITY
 * 'modularity' inspection ID.
 */
#define INSPECT_MODULARITY                  (((uint64_t) 1) << 10)

#endif

/**
 * @def INSPECT_JAVABYTECODE
 * 'javabytecode' inspection ID.
 */
#define INSPECT_JAVABYTECODE                (((uint64_t) 1) << 11)

/**
 * @def INSPECT_CHANGEDFILES
 * 'changedfiles' inspection ID.
 */
#define INSPECT_CHANGEDFILES                (((uint64_t) 1) << 12)

/**
 * @def INSPECT_MOVEDFILES
 * 'movedfiles' inspection ID.
 */
#define INSPECT_MOVEDFILES                  (((uint64_t) 1) << 13)

/**
 * @def INSPECT_REMOVEDFILES
 * 'removedfiles' inspection ID.
 */
#define INSPECT_REMOVEDFILES                (((uint64_t) 1) << 14)

/**
 * @def INSPECT_ADDEDFILES
 * 'addedfiles' inspection ID.
 */
#define INSPECT_ADDEDFILES                  (((uint64_t) 1) << 15)

/**
 * @def INSPECT_UPSTREAM
 * 'upstream' inspection ID.
 */
#define INSPECT_UPSTREAM                    (((uint64_t) 1) << 16)

/**
 * @def INSPECT_OWNERSHIP
 * 'ownership' inspection ID.
 */
#define INSPECT_OWNERSHIP                   (((uint64_t) 1) << 17)

/**
 * @def INSPECT_SHELLSYNTAX
 * 'shellsyntax' inspection ID.
 */
#define INSPECT_SHELLSYNTAX                 (((uint64_t) 1) << 18)

/**
 * @def INSPECT_ANNOCHECK
 * 'annocheck' inspection ID.
 */
#define INSPECT_ANNOCHECK                   (((uint64_t) 1) << 19)

/**
 * @def INSPECT_DSODEPS
 * 'dsodeps' inspection ID.
 */
#define INSPECT_DSODEPS                     (((uint64_t) 1) << 20)

/**
 * @def INSPECT_FILESIZE
 * 'filesize' inspection ID.
 */
#define INSPECT_FILESIZE                    (((uint64_t) 1) << 21)

/**
 * @def INSPECT_PERMISSIONS
 * 'permissions' inspection ID.
 */
#define INSPECT_PERMISSIONS                 (((uint64_t) 1) << 22)

#ifdef _WITH_LIBCAP
/**
 * @def INSPECT_CAPABILITIES
 * 'capabilities' inspection ID.
 */
#define INSPECT_CAPABILITIES                (((uint64_t) 1) << 23)
#endif

#ifdef _WITH_LIBKMOD
/**
 * @def INSPECT_KMOD
 * 'kmod' inspection ID.
 */
#define INSPECT_KMOD                        (((uint64_t) 1) << 24)
#endif

/**
 * @def INSPECT_ARCH
 * 'arch' inspection ID.
 */
#define INSPECT_ARCH                        (((uint64_t) 1) << 25)

/**
 * @def INSPECT_SUBPACKAGES
 * 'subpackages' inspection ID.
 */
#define INSPECT_SUBPACKAGES                 (((uint64_t) 1) << 26)

/**
 * @def INSPECT_CHANGELOG
 * 'changelog' inspection ID.
 */
#define INSPECT_CHANGELOG                   (((uint64_t) 1) << 27)

/**
 * @def INSPECT_PATHMIGRATION
 * 'pathmigration' inspection ID.
 */
#define INSPECT_PATHMIGRATION               (((uint64_t) 1) << 28)

/**
 * @def INSPECT_LTO
 * 'lto' inspection ID.
 */
#define INSPECT_LTO                         (((uint64_t) 1) << 29)

/**
 * @def INSPECT_SYMLINKS
 * 'symlinks' inspection ID.
 */
#define INSPECT_SYMLINKS                    (((uint64_t) 1) << 30)

/**
 * @def INSPECT_LOSTPAYLOAD
 * 'lostpayload' inspection ID.
 */
#define INSPECT_LOSTPAYLOAD                 (((uint64_t) 1) << 31)

/**
 * @def INSPECT_FILES
 * 'files' inspection ID.
 */
#define INSPECT_FILES                       (((uint64_t) 1) << 32)

/**
 * @def INSPECT_TYPES
 * 'types' inspection ID.
 */
#define INSPECT_TYPES                       (((uint64_t) 1) << 33)

/**
 * @def INSPECT_ABIDIFF
 * 'abidiff' inspection ID.
 */
#define INSPECT_ABIDIFF                     (((uint64_t) 1) << 34)

/**
 * @def INSPECT_KMIDIFF
 * 'kmidiff' inspection ID.
 */
#define INSPECT_KMIDIFF                     (((uint64_t) 1) << 35)

/**
 * @def INSPECT_CONFIG
 * 'config' inspection ID.
 */
#define INSPECT_CONFIG                      (((uint64_t) 1) << 36)

/**
 * @def INSPECT_DOC
 * 'doc' inspection ID.
 */
#define INSPECT_DOC                         (((uint64_t) 1) << 37)

/**
 * @def INSPECT_PATCHES
 * 'patches' inspection ID.
 */
#define INSPECT_PATCHES                     (((uint64_t) 1) << 38)

/**
 * @def INSPECT_VIRUS
 * 'virus' inspection ID.
 */
#define INSPECT_VIRUS                       (((uint64_t) 1) << 39)

/**
 * @def INSPECT_POLITICS
 * 'politics' inspection ID.
 */
#define INSPECT_POLITICS                    (((uint64_t) 1) << 40)

/**
 * @def INSPECT_BADFUNCS
 * 'badfuncs' inspection ID.
 */
#define INSPECT_BADFUNCS                    (((uint64_t) 1) << 41)

/**
 * @def INSPECT_RUNPATH
 * 'runpath' inspection ID.
 */
#define INSPECT_RUNPATH                     (((uint64_t) 1) << 42)

/**
 * @def INSPECT_UNICODE
 * 'unicode' inspection ID.
 */
#define INSPECT_UNICODE                     (((uint64_t) 1) << 43)

/**
 * @def INSPECT_RPMDEPS
 * 'rpmdeps' inspection ID.
 */
#define INSPECT_RPMDEPS                     (((uint64_t) 1) << 44)

/**
 * @def INSPECT_DEBUGINFO
 * 'debuginfo' inspection ID.
 */
#define INSPECT_DEBUGINFO                   (((uint64_t) 1) << 45)

/**
 * @def INSPECT_UDEVRULES
 * 'udevrules' inspection ID.
 */
#define INSPECT_UDEVRULES                   (((uint64_t) 1) << 46)

/**
 * @def INSPECT_FILESMATCH
 * 'filesmatch' inspection ID.
 */
#define INSPECT_FILESMATCH                  (((uint64_t) 1) << 47)

/** @} */

/**
 * @defgroup INSPECTION_NAMES Names of inspections
 *
 * @{
 */

/**
 * @def NAME_DIAGNOSTICS
 * The string "diagnostics"
 */
#define NAME_DIAGNOSTICS                    "diagnostics"

/**
 * @def NAME_LICENSE
 * The string "license"
 */
#define NAME_LICENSE                        "license"

/**
 * @def NAME_EMPTYRPM
 * The string "emptyrpm"
 */
#define NAME_EMPTYRPM                       "emptyrpm"

/**
 * @def NAME_METADATA
 * The string "metadata"
 */
#define NAME_METADATA                       "metadata"

/**
 * @def NAME_MANPAGE
 * The string "manpage"
 */
#define NAME_MANPAGE                        "manpage"

/**
 * @def NAME_XML
 * The string "xml"
 */
#define NAME_XML                            "xml"

/**
 * @def NAME_ELF
 * The string "elf"
 */
#define NAME_ELF                            "elf"

/**
 * @def NAME_DESKTOP
 * The string "desktop"
 */
#define NAME_DESKTOP                        "desktop"

/**
 * @def NAME_DISTTAG
 * The string "disttag"
 */
#define NAME_DISTTAG                        "disttag"

/**
 * @def NAME_SPECNAME
 * The string "specname"
 */
#define NAME_SPECNAME                       "specname"

#ifdef _HAVE_MODULARITYLABEL

/**
 * @def NAME_MODULARITY
 * The string "modularity"
 */
#define NAME_MODULARITY                     "modularity"

#endif

/**
 * @def NAME_JAVABYTECODE
 * The string "javabytecode"
 */
#define NAME_JAVABYTECODE                   "javabytecode"

/**
 * @def NAME_CHANGEDFILES
 * The string "changedfiles"
 */
#define NAME_CHANGEDFILES                   "changedfiles"

/**
 * @def NAME_MOVEDFILES
 * The string "movedfiles"
 */
#define NAME_MOVEDFILES                     "movedfiles"

/**
 * @def NAME_REMOVEDFILES
 * The string "removedfiles"
 */
#define NAME_REMOVEDFILES                   "removedfiles"

/**
 * @def NAME_ADDEDFILES
 * The string "addedfiles"
 */
#define NAME_ADDEDFILES                     "addedfiles"

/**
 * @def NAME_UPSTREAM
 * The string "upstream"
 */
#define NAME_UPSTREAM                       "upstream"

/**
 * @def NAME_OWNERSHIP
 * The string "ownership"
 */
#define NAME_OWNERSHIP                      "ownership"

/**
 * @def NAME_SHELLSYNTAX
 * The string "shellsyntax"
 */
#define NAME_SHELLSYNTAX                    "shellsyntax"

/**
 * @def NAME_ANNOCHECK
 * The string "annocheck"
 */
#define NAME_ANNOCHECK                      "annocheck"

/**
 * @def NAME_DSODEPS
 * The string "dsodeps"
 */
#define NAME_DSODEPS                        "dsodeps"

/**
 * @def NAME_FILESIZE
 * The string "filesize"
 */
#define NAME_FILESIZE                       "filesize"

/**
 * @def NAME_PERMISSIONS
 * The string "permissions"
 */
#define NAME_PERMISSIONS                    "permissions"

#ifdef _WITH_LIBCAP
/**
 * @def NAME_CAPABILITIES
 * The string "capabilities".  Only defined if rpminspect is built
 * with libcap support.
 */
#define NAME_CAPABILITIES                   "capabilities"
#endif

#ifdef _WITH_LIBKMOD
/**
 * @def NAME_KMOD
 * The string "kmod".  Only defined if rpminspect is built with
 * libkmod support.
 */
#define NAME_KMOD                           "kmod"
#endif

/**
 * @def NAME_ARCH
 * The string "arch"
 */
#define NAME_ARCH                           "arch"

/**
 * @def NAME_SUBPACKAGES
 * The string "subpackages"
 */
#define NAME_SUBPACKAGES                    "subpackages"

/**
 * @def NAME_CHANGELOG
 * The string "changelog"
 */
#define NAME_CHANGELOG                      "changelog"

/**
 * @def NAME_PATHMIGRATION
 * The string "pathmigration"
 */
#define NAME_PATHMIGRATION                  "pathmigration"

/**
 * @def NAME_LTO
 * The string "lto"
 */
#define NAME_LTO                            "lto"

/**
 * @def NAME_SYMLINKS
 * The string "symlinks"
 */
#define NAME_SYMLINKS                       "symlinks"

/**
 * @def NAME_LOSTPAYLOAD
 * The string "lostpayload"
 */
#define NAME_LOSTPAYLOAD                    "lostpayload"

/**
 * @def NAME_FILES
 * The string "files"
 */
#define NAME_FILES                          "files"

/**
 * @def NAME_TYPES
 * The string "types"
 */
#define NAME_TYPES                          "types"

/**
 * @def NAME_ABIDIFF
 * The string "abidiff"
 */
#define NAME_ABIDIFF                        "abidiff"

/**
 * @def NAME_KMIDIFF
 * The string "kmidiff"
 */
#define NAME_KMIDIFF                        "kmidiff"

/**
 * @def NAME_CONFIG
 * The string "config"
 */
#define NAME_CONFIG                         "config"

/**
 * @def NAME_DOC
 * The string "doc"
 */
#define NAME_DOC                            "doc"

/**
 * @def NAME_PATCHES
 * The string "patches"
 */
#define NAME_PATCHES                        "patches"

/**
 * @def NAME_VIRUS
 * The string "virus"
 */
#define NAME_VIRUS                          "virus"

/**
 * @def NAME_POLITICS
 * The string "politics"
 */
#define NAME_POLITICS                       "politics"

/**
 * @def NAME_BADFUNCS
 * The string "badfuncs"
 */
#define NAME_BADFUNCS                       "badfuncs"

/**
 * @def NAME_RUNPATH
 * The string "runpath"
 */
#define NAME_RUNPATH                        "runpath"

/**
 * @def NAME_UNICODE
 * The string "unicode"
 */
#define NAME_UNICODE                        "unicode"

/**
 * @def NAME_RPMDEPS
 * The string "rpmdeps"
 */
#define NAME_RPMDEPS                        "rpmdeps"

/**
 * @def NAME_DEBUGINFO
 * The string "debuginfo"
 */
#define NAME_DEBUGINFO                      "debuginfo"

/**
 * @def NAME_UDEVRULES
 * The string "udevrules"
 */
#define NAME_UDEVRULES                      "udevrules"

/**
 * @def NAME_FILESMATCH
 * The string "filesmatch"
 */
#define NAME_FILESMATCH                     "filesmatch"

/** @} */

/**
 * @defgroup INSPECTION_DESCS Long descriptions of the inspections
 *
 * @{
 */

/**
 * @def DESC_LICENSE
 * The description for the 'license' inspection.
 */
#define DESC_LICENSE _("Verify the string specified in the License tag of the RPM metadata describes permissible software licenses as defined by the license database. Also checks to see if the License tag contains any unprofessional words as defined in the configuration file.")

/**
 * @def DESC_EMPTYRPM
 * The description for the 'emptyrpm' inspection.
 */
#define DESC_EMPTYRPM _("Check all binary RPMs in the build for any empty payloads. When comparing two builds, report new packages in the after build with empty payloads.")

/**
 * @def DESC_LOSTPAYLOAD
 * The description for the 'lostpayload' inspection.
 */
#define DESC_LOSTPAYLOAD _("Check all binary RPMs in the before and after builds for any empty payloads. Packages that lost payload data from the before build to the after build are reported.")

/**
 * @def DESC_METADATA
 * The description for the 'metadata' inspection.
 */
#define DESC_METADATA _("Perform some RPM header checks. First, check that the Vendor contains the expected string as defined in the configuration file. Second, check that the build host is in the expected subdomain as defined in the configuration file. Third, check the Summary string for any unprofessional words. Fourth, check the Description for any unprofessional words. Lastly, if there is a before build specified, check for differences between the before and after build values of the previous RPM header values and report them.")

/**
 * @def DESC_MANPAGE
 * The description for the 'manpage' inspection.
 */
#define DESC_MANPAGE _("Perform some checks on man pages in the RPM payload. First, check that each man page is compressed. Second, check that each man page contains valid content. Lastly, check that each man page is installed to the correct path.")

/**
 * @def DESC_XML
 * The description for the 'xml' inspection.
 */
#define DESC_XML _("Check that XML files included in the RPM payload are well-formed.")

/**
 * @def DESC_ELF
 * The description for the 'elf' inspection.
 */
#define DESC_ELF _("Perform several checks on ELF files. First, check that ELF objects do not contain an executable stack. Second, check that ELF objects do not contain text relocations. When comparing builds, check that the ELF objects in the after build did not lose a PT_GNU_RELRO segment. When comparing builds and there is a list of forbidden library functions, make sure nothing uses them.")

/**
 * @def DESC_DESKTOP
 * The description for the 'desktop' inspection.
 */
#define DESC_DESKTOP _("Perform syntax and file reference checks on *.desktop files. Syntax errors and invalid file references are reported as errors.")

/**
 * @def DESC_DISTTAG
 * The description for the 'disttag' inspection.
 */
#define DESC_DISTTAG _("Check that the 'Release' tag in the RPM spec file includes the %{?dist} directive.")

/**
 * @def DESC_SPECNAME
 * The description for the 'specname' inspection.
 */
#define DESC_SPECNAME _("Ensure the spec file name conforms to the NAME.spec naming format.")

#ifdef _HAVE_MODULARITYLABEL

/**
 * @def DESC_MODULARITY
 * The description for the 'modularity' inspection.
 */
#define DESC_MODULARITY _("Ensure compliance with modularity build and packaging policies (only valid for module builds, no-op otherwise).")

#endif

/**
 * @def DESC_JAVABYTECODE
 * The description for the 'javabytecode' inspection.
 */
#define DESC_JAVABYTECODE _("Check minimum required Java bytecode version in class files, report bytecode version changes between builds, and report if bytecode versions are exceeded.  The bytecode version is vendor specific to releases and defined in the configuration file.")

/**
 * @def DESC_CHANGEDFILES
 * The description for the 'changedfiles' inspection.
 */
#define DESC_CHANGEDFILES _("Report changed files from the before build to the after build.  Certain file changes will raise additional warnings if the concern is more critical than just reporting changes (e.g., a suspected security impact).  Any gzip, bzip2, or xz compressed files will have their uncompressed content compared only, which will allow changes through in the compression level used.  Message catalog files (.mo) are unpacked and compared using diff(1).  Public C and C++ header files are preprocessed and compared using diff(1).  Any changes with diff output are included in the results.")

/**
 * @def DESC_MOVEDFILES
 * The description for the 'movedfiles' inspection.
 */
#define DESC_MOVEDFILES _("Report files that have moved installation paths or across subpackages between builds.  Files moved with a security path prefix generate special reporting in case a security review is required.  Rebased packages report these findings at the INFO level while non-rebased packages report them at the VERIFY level or higher.")

/**
 * @def DESC_REMOVEDFILES
 * The description for the 'removedfiles' inspection.
 */
#define DESC_REMOVEDFILES _("Report removed files from the before build to the after build.  Shared libraries get additional reporting output as they may be unexpected dependency removals.  Files removed with a security path prefix generate special reporting in case a security review is required.  Source RPMs and debuginfo files are ignored by this inspection.  Rebased packages report these findings at the INFO level while non-rebased packages report them at the VERIFY level or higher.")

/**
 * @def DESC_ADDEDFILES
 * The description for the 'addedfiles' inspection.
 */
#define DESC_ADDEDFILES _("Report added files from the before build to the after build.  Debuginfo files are ignored as are files that match the patterns defined in the configuration file.  Files added to security paths generate special reporting in case a security review is required.  New setuid and setgid files raise a security warning unless the file is in the fileinfo list.  Rebased packages report these findings at the INFO level while non-rebased packages report them at the VERIFY level or higher.")

/**
 * @def DESC_UPSTREAM
 * The description for the 'upstream' inspection.
 */
#define DESC_UPSTREAM _("Report Source archives defined in the RPM spec file changing content between the before and after build. If the source archives change and the package is on the rebaseable list, the change is reported as informational. Otherwise the change is reported as a rebase of the package and requires inspection.")

/**
 * @def DESC_OWNERSHIP
 * The description for the 'ownership' inspection.
 */
#define DESC_OWNERSHIP _("Report files and directories owned by unexpected users and groups. Check to make sure executables are owned by the correct user and group. If a before and after build have been specified, also report ownership changes.")

/**
 * @def DESC_SHELLSYNTAX
 * The description for the 'shellsyntax' inspection.
 */
#define DESC_SHELLSYNTAX _("For all shell scripts in the build, perform a syntax check on it using the shell defined in its #! line (shell must also be listed in shell section of the configuration data). If the syntax check returns non-zero, report it to the user and return a combined stdout and stderr. If comparing two builds, perform the previous check but also report if a previously bad script is now passing the syntax check.")

/**
 * @def DESC_ANNOCHECK
 * The description for the 'annocheck' inspection.
 */
#define DESC_ANNOCHECK _("Perform annocheck tests defined in the configuration file on all ELF files in the build.  A single build specified will perform an analysis only.  Two builds specified will compare the test results between the before and after build.  If no annocheck tests are defined in the configuration file, this inspection is skipped.")

/**
 * @def DESC_DSODEPS
 * The description for the 'dsodeps' inspection.
 */
#define DESC_DSODEPS _("Compare DT_NEEDED entries in dynamic ELF executables and shared libraries between the before and after build and report changes.")

/**
 * @def DESC_FILESIZE
 * The description for the 'filesize' inspection.
 */
#define DESC_FILESIZE _("Report file size changes between builds.  If empty files became non-empty or non-empty files became empty, report those as results needing verification.  Report file change percentages as info-only.")

/**
 * @def DESC_PERMISSIONS
 * The description for the 'permissions' inspection.
 */
#define DESC_PERMISSIONS _("Report stat(2) mode changes between builds.  Checks against the fileinfo list for the product release specified or determined.  Any setuid or setgid changes will raise a message requiring Security Team review.")

#ifdef _WITH_LIBCAP
/**
 * @def DESC_CAPABILITIES
 * The description for the 'capabilities' inspection.  Only defined if
 * rpminspect is built with libcap support.
 */
#define DESC_CAPABILITIES _("Report capabilities(7) changes between builds.  Checks against the capabilities list for the product release specified or determined.  Any capabilities changes not listed will raise a message requiring Security Team review.")
#endif

#ifdef _WITH_LIBKMOD
/**
 * @def DESC_KMOD
 * The description for the 'kmod' inspection.  Only defined if
 * rpminspect is built with libkmod support.
 */
#define DESC_KMOD _("Report kernel module parameter, dependency, PCI ID, or symbol differences between builds.  This inspection is intended as an information gathering tool to gather kernel module differences between two builds.")
#endif

/**
 * @def DESC_ARCH
 * The description for the 'arch'
 */
#define DESC_ARCH _("Report RPM architectures that appear and disappear between the before and after builds.  This inspection does not report the loss of noarch packages.  The purpose of this inspection is to report the loss of target machine architectures as provided in the before build and not seeing them in the after build.")

/**
 * @def DESC_SUBPACKAGES
 * The description for the 'subpackages' inspection.
 */
#define DESC_SUBPACKAGES _("Report RPM subpackages that appear and disappear between the before and after builds.")

/**
 * @def DESC_CHANGELOG
 * The description for the 'changelog' inspection.
 */
#define DESC_CHANGELOG _("Ensure packages contain an entry in the %changelog for the version built.  Reports any other differences in the existing changelog between builds and that the new entry contains new text entries.")

/**
 * @def DESC_PATHMIGRATION
 * The description for the 'pathmigration' inspection.
 */
#define DESC_PATHMIGRATION _("Report files that are packaged in directories that are no longer used by the product.  Usually this means a package has not been updated to account for path migrations.  The main examples are /bin migrating to /usr/bin and /sbin migrating to /usr/sbin.")

/**
 * @def DESC_LTO
 * The description for the 'lto' inspection.
 */
#define DESC_LTO _("Link Time Optimization (LTO) produces smaller and faster shared ELF executables and libraries.  LTO bytecode is not stable from one release of gcc to the next.  As such, LTO bytecode should not be present in .a and .o ELF objects shipped in packages.  This inspection looks for LTO bytecode in ELF relocatable objects and reports if any is present.")

/**
 * @def DESC_SYMLINKS
 * The description for the 'symlinks' inspection.
 */
#define DESC_SYMLINKS _("Symbolic links must be resolvable on the installed system.  This inspection ensures absolute and relative symlinks are valid.  It also checks for any symlink usage that will cause problems for RPM.")

/**
 * @def DESC_FILES
 * The description for the 'files' inspection.
 */
#define DESC_FILES _("Check %files sections in the spec file for any forbidden path references.")

/**
 * @def DESC_TYPES
 * The description for the 'types' inspection.
 */
#define DESC_TYPES _("Compare MIME types of files between builds and report any changes for verification.")

/**
 * @def DESC_ABIDIFF
 * The description for the 'abidiff' inspection.
 */
#define DESC_ABIDIFF _("When comparing two builds or two packages, compare ELF files using abidiff(1) from the libabigail project.  Differences are reported.  If the package is a rebase and not on the rebaseable list and the rebase inspection is enabled, ABI differences are reported as failures.  The assumption here is that rpminspect is comparing builds for maintenance purposes and you do not want to introduce any ABI changes for users.  If you do not care about that, turn off the abidiff inspection or add the package name to the rebaseable list.")

/**
 * @def DESC_KMIDIFF
 * The description for the 'kmidiff' inspection.
 */
#define DESC_KMIDIFF _("kmidiff compares the binary Kernel Module Interfaces of two Linux kernel trees.  The binary KMI is the interface that the Linux kernel exposes to its modules.  The trees we are interested in here are the result of the build of the Linux kernel source tree.  If the builds compared are not considered a rebase, an incompatible change reported by kmidiff is reported for verification.")

/**
 * @def DESC_CONFIG
 * The description for the 'config' inspection.
 */
#define DESC_CONFIG _("Check for and report differences in configuration files marked with %config in the spec file.  If changes are whitespace or formatting only, the result is reported at the INFO level.  Content and location changes are reporting at the VERIFY level unless the comparison is between rebased packages.")

/**
 * @def DESC_DOC
 * The description for the 'doc' inspection.
 */
#define DESC_DOC _("Report changes in %doc marked files.  Files that have been added or removed as documentation files, for instance.")

/**
 * @def DESC_PATCHES
 * The description for the 'patches' inspection.
 */
#define DESC_PATCHES _("Inspects all patches defined in the spec file and reports changes between builds.  The functional check here is to ensure all defined patches in the preamble have a corresponding application macro in the spec file.  At the INFO level, rpminspect reports file count, line count, and patch size changes.  If thresholds are reached regarding a change in the patch size or the number of files the patch touches, rpminspect reports the change at the VERIFY level unless the comparison is for a rebase.  The configuration file can also list patch names that rpminspect should ignore during the inspection.")

/**
 * @def DESC_VIRUS
 * The description for the 'virus' inspection.
 */
#define DESC_VIRUS _("Performs a virus scan on every file in the build using libclamav.  Anything found by libclamav will fail the inspection.  The ignore_path rules are not used in this inspection.  All files are scanned.")

/**
 * @def DESC_POLITICS
 * The description for the 'politics' inspection.
 */
#define DESC_POLITICS _("Check for known politically sensitive files in packages and report if they are allowed or prohibited.  The rules come from the politics/ subdirectory in the rpminspect-data package for the product.  Files in politics/ subdirectory map to the product release string.  The rules in each file define the politically sensitive allow and deny rules for that release.")

/**
 * @def DESC_BADFUNCS
 * The description for the 'badfuncs' inspection.
 */
#define DESC_BADFUNCS _("Check for forbidden functions in ELF files.  Forbidden functions are defined in the runtime configuration files.  Usually this inspection is used to catch built packages that make use of deprecated API functions if you wish built packages to conform to replacement APIs.")

/**
 * @def DESC_RUNPATH
 * The description for the 'runpath' inspection.
 */
#define DESC_RUNPATH _("Check for forbidden paths in both the DT_RPATH and DT_RUNPATH settings in ELF shared objects.")

/**
 * @def DESC_UNICODE
 * The description for the 'unicode' inspection.
 */
#define DESC_UNICODE _("Scan extracted and patched source code files, scripts, and RPM spec files for any prohibited Unicode code points, as defined in the configuration file.  Any prohibited code points are reported as a possible security risk.")

/**
 * @def DESC_RPMDEPS
 * The description for the 'rpmdeps' inspection.
 */
#define DESC_RPMDEPS _("Check for correct RPM dependency metadata.  Report incorrect or conflicting findings as well as expected changes when comparing a new build to an older build.  Changes are only reported when comparing builds, but this inspection will check for correct RPM dependency metadata when inspecting a single build and report findings.")

/**
 * @def DESC_DEBUGINFO
 * The description for the 'debuginfo' inspection.
 */
#define DESC_DEBUGINFO _("Checks that files in RPM packages have their debugging symbols stripped and files in debuginfo packages carry debugging symbols.  When comparing builds, report where symbols unexpectedly appear or disappear and what corrective action is needed.")

/**
 * @def DESC_UDEVRULES
 * The description for the 'udevrules' inspection.
 */
#define DESC_UDEVRULES _("Perform syntax check on udev rules files using udevadm verify.")

/**
 * @def DESC_FILESMATCH
 * The description for the 'filesmatch' inspection.
 */
#define DESC_FILESMATCH _("Match every file path glob(7) specified in the %%files section(s) of the spec file against every file found in the binary RPMs.  Report any discrepancies as a failure.")

/** @} */

#endif

#ifdef __cplusplus
}
#endif
