/*
 * Copyright 2018 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file constants.h
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2018
 * @brief Constants and other defaults for librpminspect.
 * @copyright LGPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_CONSTANTS_H
#define _LIBRPMINSPECT_CONSTANTS_H

/**
 * @defgroup Constants and Other Defaults
 *
 * These macros define program defaults or other more or less standard
 * things so that when we need to change the value in the code, we
 * only need to change it in one place.
 *
 * @{
 */

/**
 * @defgroup Internal defaults used throughout librpminspect
 *
 * @{
 */

/**
 * @def DEFAULT_MESSAGE_DIGEST
 *
 * Default message digest to use internally.  The definition comes
 * from a macro in rpminspect.h.
 */
#define DEFAULT_MESSAGE_DIGEST SHA256SUM

/**
 * @def SHARED_LIB_PREFIX
 *
 * Filename prefix for shared libraries.
 */
#define SHARED_LIB_PREFIX "lib"

/**
 * @def SHARED_LIB_SUFFIX
 *
 * Filename suffix for shared libraries.  Note that this is really
 * more a substring that appears in shared library names given how ELF
 * libraries are installed and managed.
 */
#define SHARED_LIB_SUFFIX ".so"

/**
 * @def DEFAULT_TTY_WIDTH
 *
 * Fallback tty width if TIOCGWINSZ fails.
 */
#define DEFAULT_TTY_WIDTH 80

/** @} */

/**
 * @defgroup Configuration defaults
 *
 * @{
 */

/**
 * @def COMMAND_NAME
 *
 * The standard name for the rpminspect command.
 */
#define COMMAND_NAME "rpminspect"

/**
 * @def SOFTWARE_NAME
 *
 * Software name.  Used in Koji XMLRPC calls and logging.
 */
#define SOFTWARE_NAME "librpminspect"

/**
 * @def CFGFILE_DIR
 *
 * Configuration file directory.
 */
#define CFGFILE_DIR "/usr/share/rpminspect"

/**
 * @def CFGFILE
 *
 * Default configuration file.
 */
#define CFGFILE "rpminspect.yaml"

/**
 * @def PRODUCT_RELEASE_CFGFILE_SUBDIR
 *
 * The name of the product release configuration file subdirectory.
 * This will be under 'profiledir', which is defined in the main
 * configuration file.  This directory can exist and it can hold
 * configuration files that match product release strings.  If
 * rpminspect finds one during initialization that matches the product
 * release string, it will load it.
 */
#define PRODUCT_RELEASE_CFGFILE_SUBDIR "product-release"

/**
 * @def DEFAULT_WORKDIR
 *
 * Default working directory location.  rpminspect will create
 * subdirectories within this directory so multiple concurrent jobs
 * can run.
 */
#define DEFAULT_WORKDIR "/var/tmp/rpminspect"

/**
 * @def ROOT_SUBDIR
 *
 * The name of the root subdirectory used in the working directory.
 * This is where packages are extracted as a simulated root
 * filesystem.
 */
#define ROOT_SUBDIR "root"

/**
 * @def BEFORE_SUBDIR
 *
 * The name of the before build subdirectory in the working directory.
 */
#define BEFORE_SUBDIR "before"

/**
 * @def AFTER_SUBDIR
 *
 * The name of the after build subdirectory in the working directory.
 */
#define AFTER_SUBDIR "after"

/**
 * @def VENDOR_DATA_DIR
 *
 * Default location for the vendor-specific data.  These files should
 * be provided by the vendor-specific rpminspect-data package.
 */
#define VENDOR_DATA_DIR "/usr/share/rpminspect"

/**
 * @def INSPECTIONS
 *
 * Name of the [inspections] section in the config file.
 */
#define INSPECTIONS "inspections"

/** @} */

/**
 * @defgroup Vendor Data Subdirectories
 *
 * With a few exceptions, the files within vendor data subdirectories
 * will correspond to the product release rpminspect is using rules
 * from.  For example, if the product release is "fc35", then
 * rpminspect will expect files named "fc35" in these subdirectories.
 * If an expected file is not found in the subdirectory, that is the
 * same as rpminspect having no product release rules specific to that
 * data set.
 *
 * The notable exception is the license database, which is named in
 * the rpminspect configuration file and generally corresponds to the
 * vendor and not a specific product release.
 * @{
 */

/**
 * @def ABI_DIR
 *
 * Name of the ABI checking subdirectory in VENDOR_DATA_DIR.
 */
#define ABI_DIR "abi"

/**
 * @def CAPABILITIES_DIR
 *
 * Name of the capabilities(7) subdirectory in VENDOR_DATA_DIR.
 */
#define CAPABILITIES_DIR "capabilities"

/**
 * @def LICENSES_DIR
 *
 * Name of the license database subdirectory in VENDOR_DATA_DIR.
 */
#define LICENSES_DIR "licenses"

/**
 * @def FILEINFO_DIR
 *
 * Name of the fileinfo (stat(2)) list subdirectory in VENDOR_DATA_DIR.
 */
#define FILEINFO_DIR "fileinfo"

/**
 * @def REBASEABLE_DIR
 *
 * Name of the VENDOR_DATA_DIR subdirectory listing rebaseable
 * packages.  A rebaseable package is one where the version number
 * changes between the before and after build but is expected.
 * Putting the package on the rebaseable list prevents rpminspect from
 * reporting an unexpected package rebase.
 */
#define REBASEABLE_DIR "rebaseable"

/**
 * @def POLITICS_DIR
 *
 * Name of the VENDOR_DATA_DIR subdirectory with political related
 * file inclusion and exclusion rules.
 */
#define POLITICS_DIR "politics"

/**
 * @def SECURITY_DIR
 *
 * Name of the VENDOR_DATA_DIR subdirectory with security related
 * package rules.
 */
#define SECURITY_DIR "security"

/**
 * @def ICONS_DIR
 *
 * Name of the VENDOR_DATA_DIR subdirectory with standard icon names
 * defined per product.
 */
#define ICONS_DIR "icons"

/** @} */

/**
 * @defgroup Desktop Constants
 *
 * Constants related to desktop files.  Paths and where to find icons,
 * for instance.
 * @{
 */

/**
 * @def DESKTOP_ENTRY_FILES_DIR
 *
 * Standard location for desktop entry files.
 */
#define DESKTOP_ENTRY_FILES_DIR "/usr/share/applications"

/** @} */

/**
 * @defgroup Commands used by different inspections
 *
 * Avoid explicit paths, rpminspect assumes these commands are
 * available in the $PATH.
 *
 * @{
 */

/**
 * @def MSGUNFMT_CMD
 *
 * Executable providing msgunfmt(1)
 */
#define MSGUNFMT_CMD "msgunfmt"

/**
 * @def DESKTOP_FILE_VALIDATE_CMD
 *
 * Executable providing desktop-file-validate(1)
 */
#define DESKTOP_FILE_VALIDATE_CMD "desktop-file-validate"

/**
 * @def ABIDIFF_CMD
 *
 * Executable providing abidiff(1)
 */
#define ABIDIFF_CMD "abidiff"

/**
 * @def ABI_SUPPRESSION
 *
 * The command line option for abidiff(1) to use the suppression
 * specification file in the named file.  rpminspect will find the
 * suppression specification file in t he source package, but needs to
 * know the name of the command line option.
 */
#define ABI_SUPPRESSIONS "--suppressions"

/**
 * @def ABI_DEBUG_INFO_DIR1
 *
 * The command line option for abidiff(1) to specify the location of
 * debugging symbols for files in the before build.  For RPM packages,
 * this is a path to the corresponding directory in a debuginfo
 * package.
 */
#define ABI_DEBUG_INFO_DIR1 "--d1"

/**
 * @def ABI_DEBUG_INFO_DIR2
 *
 * The command line option for abidiff(1) to specify the location of
 * debugging symbols for files in the after build.  For RPM packages,
 * this is a path to the corresponding directory in a debuginfo
 * package.
 */
#define ABI_DEBUG_INFO_DIR2 "--d2"

/**
 * @def KMIDIFF_CMD
 *
 * Executable providing kmidiff(1), the Kernel Module Interface
 * comparison tool.
 */
#define KMIDIFF_CMD "kmidiff"

/**
 * @def KMIDIFF_VMLINUX1
 *
 * The command line option to kmidiff(1) specifying the location of
 * the Linux kernel file in the before build.
 */
#define KMIDIFF_VMLINUX1 "--vmlinux1"

/**
 * @def KMIDIFF_VMLINUX2
 *
 * The command line option to kmidiff(1) specifying the location of
 * the Linux kernel file in the after build.
 */
#define KMIDIFF_VMLINUX2 "--vmlinux2"

/**
 * @def KMIDIFF_KMI_WHITELIST
 *
 * The command line option to kmidiff(1) used to specify the kernel
 * module interface whitelist.  rpminspect will find this file in the
 * source package, but it needs to know the command line option to use
 * with kmidiff(1).
 */
#define KMIDIFF_KMI_WHITELIST "--kmi-whitelist"

/** @} */

/**
 * @defgroup System defaults
 *
 * @{
 */

/**
 * @def SRPM_ARCH_NAME
 *
 * The architecture name Koji uses for source RPMs.
 */
#define SRPM_ARCH_NAME "src"

/**
 * @def RPM_NOARCH_NAME
 *
 * The architecture name Koji uses for binary RPMs built with
 * 'BuildArch: noarch'
 */
#define RPM_NOARCH_NAME "noarch"

/**
 * @def BIN_OWNER
 *
 * Default executable file owner.
 */
#define BIN_OWNER "root"

/**
 * @def BIN_GROUP
 *
 * Default executable file group.
 */
#define BIN_GROUP "root"

/**
 * @def BUILD_ID_DIR
 *
 * Build ID subdirectory name.
 */
#define BUILD_ID_DIR "/.build-id/"

/**
 * @def DEBUGINFO_SUFFIX
 *
 * The debuginfo package name suffix string.
 */
#define DEBUGINFO_SUFFIX "-debuginfo"

/**
 * @def DEBUGSOURCE_SUFFIX
 *
 * The debugsource package name suffix string.
 */
#define DEBUGSOURCE_SUFFIX "-debugsource"

/**
 * @def DEBUG_PATH
 *
 * Where debuginfo packages are installed.
 */
#define DEBUG_PATH "/usr/lib/debug/"

/**
 * @def DEBUG_SRC_PATH
 *
 * Where debugsource packages are installed.
 */
#define DEBUG_SRC_PATH "/usr/src/debug/"

/**
 * @def DEBUG_SUBSTRING
 *
 * The substring that can appear in filenames installed with debugging
 * information.
 */
#define DEBUG_SUBSTRING "debug"

/**
 * @def DEBUG_FILE_SUFFIX
 *
 * The substring appearing at the end of debuginfo files.
 */
#define DEBUG_FILE_SUFFIX ".debug"

/**
 * @def KERNEL_MODULES_DIR
 *
 * Linux loadable kernel modules subdirectory.
 */
#define KERNEL_MODULES_DIR "/lib/modules/"

/**
 * @def ABIDIFF_SUPPRESSION_FILE
 *
 * Default ABI suppression file in source packages.  If this file is
 * found in a source package, it will be passed to abidiff(1) using
 * the ABI_SUPPRESSIONS option.
 */
#define ABI_SUPPRESSION_FILE ".abignore"

/**
 * @def INCLUDE_SUBDIR
 *
 * Default header file subdirectory name.  Anywhere we find an include
 * subdirectory, rpminspect will consider that a location for public
 * header files.
 */
#define INCLUDE_SUBDIR "/include"

/**
 * @def KERNEL_FILENAMES
 *
 * Default array of kernel executable filename possibilities.
 */
#define KERNEL_FILENAMES { "vmlinux", "vmlinuz", NULL }

/**
 * @def KMI_IGNORE_PATTERN
 *
 * File pattern used to find the Kernel Module Interface ignore list
 * for kmidiff(1).  The substring '${ARCH}' will be replaced with the
 * package architecture.
 */
#define KMI_IGNORE_PATTERN "/lib/modules/kabi-current/kabi_whitelist_${ARCH}"

/** @} */

/**
 * @defgroup Filename extensions
 *
 * @{
 */

/**
 * @def RPM_FILENAME_EXTENSION
 *
 * RPM filename extension
 */
#define RPM_FILENAME_EXTENSION ".rpm"

/**
 * @def SPEC_FILENAME_EXTENSION
 *
 * RPM spec filename extension
 */
#define SPEC_FILENAME_EXTENSION ".spec"

/**
 * @def JAR_FILENAME_EXTENSION
 *
 * Java jar filename extension
 */
#define JAR_FILENAME_EXTENSION ".jar"

/**
 * @def CLASS_FILENAME_EXTENSION
 *
 * Java class filename extension
 */
#define CLASS_FILENAME_EXTENSION ".class"

/**
 * @def EGGINFO_FILENAME_EXTENSION
 *
 * Python egg-info filename extension
 */
#define EGGINFO_FILENAME_EXTENSION ".egg-info"

/**
 * @def GZIPPED_FILENAME_EXTENSION
 *
 * Gzip filename extension
 */
#define GZIPPED_FILENAME_EXTENSION ".gz"

/**
 * @def DESKTOP_FILENAME_EXTENSION
 *
 * Desktop filename extension
 */
#define DESKTOP_FILENAME_EXTENSION ".desktop"

/**
 * @def DIRECTORY_FILENAME_EXTENSION
 *
 * Directory filename extension
 */
#define DIRECTORY_FILENAME_EXTENSION ".directory"

/**
 * @def MO_FILENAME_EXTENSION
 *
 * Machine object filename extension (compiled translation data)
 */
#define MO_FILENAME_EXTENSION ".mo"

/**
 * @def PYTHON_PYC_FILE_EXTENSION
 *
 * Python bytecode filename extension
 */
#define PYTHON_PYC_FILE_EXTENSION ".pyc"

/**
 * @def PYTHON_PYO_FILE_EXTENSION
 *
 * Python optimized bytecode filename extension
 */
#define PYTHON_PYO_FILE_EXTENSION ".pyo"

/**
 * @def KERNEL_MODULE_FILENAME_EXTENSION
 *
 * Linux loadable kernel module filename extension.  Note that kernel
 * modules can be compressed with a typical compression appened after
 * this string.  rpminspect can handle those cases.
 */
#define KERNEL_MODULE_FILENAME_EXTENSION ".ko"

/**
 * @def SVG_FILENAME_EXTENSION
 *
 * Scalable vector graphics filename extension
 */
#define SVG_FILENAME_EXTENSION ".svg"

/**
 * @def STATIC_LIB_FILENAME_EXTENSION
 *
 * Static ELF library filename extension
 */
#define STATIC_LIB_FILENAME_EXTENSION ".a"

/**
 * @def ELF_LIB_EXTENSION
 *
 * ELF shared library extension.  Note that this extension appears in
 * the middle of ELF library filenames because a version number comes
 * after it.  For files that end with just '.so', we do not care so
 * much because rpminspect will match the files correctly.  That's why
 * this extension ends with a period.
 */
#define ELF_LIB_EXTENSION ".so."

/**
 * @def YAML_FILENAME_EXTENSION
 *
 * YAML filename extension
 */
#define YAML_FILENAME_EXTENSION ".yaml"

/** @} */

/**
 * @defgroup RPM spec file defaults
 *
 * @{
 */

/**
 * @def SPEC_MACRO_DEFINE
 *
 * How macros are defined in spec files.
 */
#define SPEC_MACRO_DEFINE      "%define"

/**
 * @def SPEC_MACRO_GLOBAL
 *
 * How global macros are defined in spec files.
 */
#define SPEC_MACRO_GLOBAL      "%global"

/**
 * @def SPEC_MACRO_PATCH
 *
 * The %patchN macro used to apply patches.  N corresponds to the
 * patch number defined in the header.
 */
#define SPEC_MACRO_PATCH       "%patch"

/**
 * @def SPEC_SECTION_PREP
 *
 * Command or series of commands to prepare the software to be built,
 * for example, unpacking the archive in Source0. This directive can
 * contain a shell script.
 */
#define SPEC_SECTION_PREP      "%prep"

/**
 * @def SPEC_SECTION_BUILD
 *
 * Command or series of commands for actually building the software
 * into machine code (for compiled languages) or byte code (for some
 * interpreted languages).
 */
#define SPEC_SECTION_BUILD     "%build"

/**
 * @def SPEC_SECTION_INSTALL
 *
 * Command or series of commands for copying the desired build
 * artifacts from the %builddir (where the build happens) to the
 * %buildroot directory (which contains the directory structure with
 * the files to be packaged). This usually means copying files from
 * ~/rpmbuild/BUILD to ~/rpmbuild/BUILDROOT and creating the necessary
 * directories in ~/rpmbuild/BUILDROOT. This is only run when creating
 * a package, not when the end-user installs the package. See Working
 * with SPEC files for details.
 */
#define SPEC_SECTION_INSTALL   "%install"

/**
 * @def SPEC_SECTION_CHECK
 *
 * Command or series of commands to test the software. This normally
 * includes things such as unit tests.
 */
#define SPEC_SECTION_CHECK     "%check"

/**
 * @def SPEC_SECTION_FILES
 *
 * How file listings are specified in the spec file.
 */
#define SPEC_SECTION_FILES     "%files"

/**
 * @def SPEC_SECTION_CHANGELOG
 *
 * How the change log is specified in the spec file.
 */
#define SPEC_SECTION_CHANGELOG "%changelog"

/**
 * @def SPEC_TAG_RELEASE
 *
 * The name of RPMTAG_RELEASE in a spec file.
 */
#define SPEC_TAG_RELEASE       "Release:"

/**
 * @def SPEC_TAG_PATCH
 *
 * The leading text of the RPMTAG_PATCH identifier in a spec file.
 */
#define SPEC_TAG_PATCH         "Patch"

/**
 * @def SPEC_DISTTAG
 *
 * The distribution "dist tag" typically used in SPEC_TAG_RELEASE
 * strings.  Common in Fedora Linux and related distributions, but may
 * not be consistently defined in other RPM-based distributions.
 */
#define SPEC_DISTTAG           "%{?dist}"

/** @} */

/**
 * @defgroup Subdirectory names used by rpmbuild(1)
 *
 * Names of rpmbuild(1) subdirectories.  rpminspect models an rpmbuild
 * layout similar to how rpmbuild is used for distributions like
 * Fedora Linux.  The main idea is a top level directory is defined
 * (topdir) and all of the other directories used by rpmbuild are
 * within topdir.  For the purposes of rpminspect, the topdir
 * definition here is a subdirectory in the working directory location
 * rpminspect is using.
 *
 * @{
 */

/**
 * @def RPMBUILD_TOPDIR
 *
 * The rpmbuild(1) top directory.  The directory where all other
 * rpmbuild(1) subdirectories live.
 */
#define RPMBUILD_TOPDIR        "rpmbuild"

/**
 * @def RPMBUILD_BUILDDIR
 *
 * The BUILD subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_BUILDDIR      "BUILD"

/**
 * @def RPMBUILD_BUILDROOTDIR
 *
 * The BUILDROOT subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_BUILDROOTDIR  "BUILDROOT"

/**
 * @def RPMBUILD_RPMDIR
 *
 * The RPMS subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_RPMDIR        "RPMS"

/**
 * @def RPMBUILD_SOURCEDIR
 *
 * The SOURCES subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_SOURCEDIR     "SOURCES"

/**
 * @def RPMBUILD_SPECDIR
 *
 * The SPECS subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_SPECDIR       "SPECS"

/**
 * @def RPMBUILD_SRPMSDIR
 *
 * The SRPMS subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_SRPMDIR       "SRPMS"

/** @} */

/**
 * @defgroup 'abi' inspection defaults
 *
 * @{
 */

/**
 * @def DEFAULT_ABI_SECURITY_THRESHOLD
 *
 * Default ABI compat level security reporting threshold.
 */
#define DEFAULT_ABI_SECURITY_THRESHOLD 2

/** @} */

/**
 * @defgroup 'runpath' inspection defaults
 *
 * @{
 */

/**
 * @def RUNPATH_ORIGIN_STR
 *
 * The value of the DT_RPATH or DT_RUNPATH $ORIGIN string.
 */
#define RUNPATH_ORIGIN_STR "$ORIGIN"

/** @} */

/**
 * @defgroup ELF section names used through the code.
 *
 * @{
 */

/**
 * @def ELF_SYMTAB
 *
 * The '.symtab' ELF section.
 */
#define ELF_SYMTAB ".symtab"

/**
 * @def ELF_GDB_INDEX
 *
 * The '.gdb_index' ELF section.
 */
#define ELF_GDB_INDEX ".gdb_index"

/**
 * @def ELF_GNU_DEBUGDATA
 *
 * The '.gnu_debugdata' ELF section.
 */
#define ELF_GNU_DEBUGDATA ".gnu_debugdata"

/**
 * @def ELF_DEBUG_INFO
 *
 * The '.debug_info' ELF section.
 */
#define ELF_DEBUG_INFO ".debug_info"

/**
 * @def ELF_GOSYMTAB
 *
 * The '.gosymtab' ELF section.
 */
#define ELF_GOSYMTAB ".gosymtab"

/** @} */

/**
 * @defgroup Modularity constants.
 *
 * @{
 */

/**
 * @def MODULEMD_FILENAME
 *
 * The name of the module metadata file.
 */
#define MODULEMD_FILENAME "modulemd.txt"

/**
 * @def MODULEMD_ARCH_FILENAME
 *
 * The filename pattern for architecture-specific modulemd files.
 */
#define MODULEMD_ARCH_FILENAME "modulemd.%s.txt"

/** @} */

/** @} */

#endif
