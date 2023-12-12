/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _LIBRPMINSPECT_RESULTS_H
#define _LIBRPMINSPECT_RESULTS_H

/**
 * @defgroup Inspection remedy strings.
 *
 * The purpose of the inspection remedy strings is to give the user an
 * idea of what the finding means when reading the results from
 * rpminspect.  Explain what was found and what steps can be taken to
 * correct the error or where to look for additional information.
 *
 * @{
 */

/**
 * @defgroup metadata inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_VENDOR
 *
 * How to address an invalid Vendor tag in an RPM header.
 */
#define REMEDY_VENDOR _("Change the string specified on the 'Vendor:' line in the spec file.")

/**
 * @def REMEDY_BUILDHOST
 *
 * How to address an invalid BuildHost in an RPM header.
 */
#define REMEDY_BUILDHOST _("Make sure the SRPM is built on a host within the expected subdomain.")

/**
 * @def REMEDY_BADWORDS
 *
 * How to address unprofessional language found in an RPM description or Summary.
 */
#define REMEDY_BADWORDS _("Unprofessional language as defined in the configuration file was found in the text shown.  Remove or change the offending words and rebuild.")

/** @} */

/**
 * @defgroup emptyrpm inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_EMPTYRPM
 *
 * How to address an unexpected empty RPM in the build.
 */
#define REMEDY_EMPTYRPM _("Check to see if you eliminated a subpackage but still have the %package and/or the %files section for it.")

/** @} */

/**
 * @defgroup lostpayload inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_LOSTPAYLOAD
 *
 * How to address an unexpected lost payload in a built RPM.
 */
#define REMEDY_LOSTPAYLOAD _("Check to see if you eliminated a subpackage but still have the %package and/or the %files section for it.")

/** @} */

/**
 * @defgroup license inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_LICENSE
 *
 * How to address an invalid License tag expression in the spec file.
 */
#define REMEDY_LICENSE _("The License tag must contain an approved license string as defined by the distribution (e.g., GPLv2+).  If the license in question is approved, the license database needs updating in the rpminspect-data package.")

/**
 * @def REMEDY_LICENSEDB
 *
 * How to address a missing license database file for rpminspect.
 */
#define REMEDY_LICENSEDB _("Make sure the licensedb setting in the rpminspect configuration is set to a valid licensedb file.  This is also commonly due to a missing vendor specific rpminspect-data package on the system.")

/**
 * @def REMEDY_UNAPPROVED_LICENSE
 *
 * How to address an unapproved license found in the License tag.
 */
#define REMEDY_UNAPPROVED_LICENSE _("The specified license abbreviation is not listed as approved in the license database.  The license database is specified in the rpminspect configuration file.  Check this file and send a pull request to the appropriate upstream project to update the database.  If the license is listed in the database but marked unapproved, you may need to work with the legal team regarding options for this software.")

/** @} */

/**
 * @defgroup elf inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_ELF_TEXTREL
 *
 * How to address ELF objects containing non-position independent code.
 */
#define REMEDY_ELF_TEXTREL _("Ensure all object files are compiled with -fPIC")

/**
 * @def REMEDY_ELF_EXECSTACK_MISSING
 *
 * How to address ELF objects missing execstack compiler flags.
 */
#define REMEDY_ELF_EXECSTACK_MISSING _("Ensure that the package is being built with the correct compiler and compiler flags")

/**
 * @def REMEDY_ELF_EXECSTACK_INVALID
 *
 * How to address invalid ELF objects.
 */
#define REMEDY_ELF_EXECSTACK_INVALID _("The data in an ELF file appears to be corrupt; ensure that packaged ELF files are not being truncated or incorrectly modified")

/**
 * @def REMEDY_ELF_EXECSTACK_EXECUTABLE
 *
 * How to address ELF objects with an executable execstack.
 */
#define REMEDY_ELF_EXECSTACK_EXECUTABLE _("An ELF stack is marked as executable. Ensure that no execstack options are being passed to the linker, and that no functions are defined on the stack.")

/**
 * @def REMEDY_ELF_GNU_RELRO
 *
 * How to address ELF objects without read-only relocations.
 */
#define REMEDY_ELF_GNU_RELRO _("Ensure executables are linked with with '-z relro -z now'")

/**
 * @def REMEDY_ELF_FPIC
 *
 * How to address ELF objects with non-position independent code.
 */
#define REMEDY_ELF_FPIC _("Ensure all object files are compiled with -fPIC")

/** @} */

/**
 * @defgroup man inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_MAN_ERRORS
 *
 * How to address man page errors.
 */
#define REMEDY_MAN_ERRORS _("Correct the errors in the man page as reported by the libmandoc parser.")

/**
 * @def REMEDY_MAN_PATH
 *
 * How to address man page installation programs.
 */
#define REMEDY_MAN_PATH _("Correct the installation path for the man page. Man pages must be installed in the directory beneath /usr/share/man that matches the section number of the page.")

/** @} */

/**
 * @defgroup xml inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_XML
 *
 * How to address XML file validity problems.
 */
#define REMEDY_XML _("Correct the reported errors in the XML document")

/** @} */

/**
 * @defgroup desktop inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_DESKTOP
 *
 * How to address *.desktop file validity problems.
 */
#define REMEDY_DESKTOP _("Refer to the Desktop Entry Specification at https://standards.freedesktop.org/desktop-entry-spec/latest/ for help correcting the errors and warnings")

/** @} */

/**
 * @defgroup disttag inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_DISTTAG
 *
 * How to deal with the Release field missing a dist tag in the spec file.
 */
#define REMEDY_DISTTAG _("The Release: tag in the spec file must include a '%{?dist}' string.  Please add this to the spec file per the distribution packaging guidelines.")

/** @} */

/**
 * @defgroup specname inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_SPECNAME
 *
 * How to handle spec file naming problems.
 */
#define REMEDY_SPECNAME _("The spec file name does not match the expected NAME.spec format.  Rename the spec file to conform to this policy.")

/** @} */

#ifdef _HAVE_MODULARITYLABEL

/**
 * @defgroup modularity inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_MODULARITY_LABEL
 *
 * How to address modularity problems in RPMs that are built as part of modules.
 */
#define REMEDY_MODULARITY_LABEL _("This package is part of a module but is missing the %{modularitylabel} header tag.  Add this as a %define in the spec file and rebuild.")

/**
 * @def REMEDY_MODULARITY_RELEASE
 *
 * How to address a non-conformant Release value in RPMs that are build as part of modules.
 */
#define REMEDY_MODULARITY_RELEASE _("This package is part of a module but lacks a conformant Release tag value.  A Release tag in a modular RPM needs to carry a substring that is more specific than a major release dist tag (e.g., el8.9.0 rather than el8) and must carry '+module' as a substring before that specific dist tag.")

/**
 * @def REMEDY_MODULARITY_STATIC_CONTEXT
 *
 * How to address problems with /data/static_context in module metadata.
 */
#define REMEDY_MODULARITY_STATIC_CONTEXT _("This build either contains a valid or invalid /data/static_context setting.  Refer to the module rules for the product you are building to determine what the setting should be.  The rpminspect configuration settings also set the rules determining if the /data/static_context setting is required, forbidden, or recommend.")

/** @} */

#endif

/**
 * @defgroup javabytecode inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_JAVABYTECODE
 *
 * How to address Java major byte code version problems.
 */
#define REMEDY_JAVABYTECODE _("The Java bytecode version for one or more class files in the build was not met for the product release.  Ensure you are using the correct JDK for the build.")

/** @} */

/**
 * @defgroup changedfiles inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_CHANGEDFILES
 *
 * How to handle file changes.
 */
#define REMEDY_CHANGEDFILES _("File changes were found.  In most cases these are expected, but it is a good idea to verify the changes found are deliberate.")

/** @} */

/**
 * @defgroup movedfiles inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_MOVEDFILES
 *
 * How to handle unexpected moved files.
 */
#define REMEDY_MOVEDFILES _("Unexpected file moves were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file moves between builds.")

/** @} */

/**
 * @defgroup removedfiles inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_REMOVEDFILES
 *
 * How to handle unexpected removed files.
 */
#define REMEDY_REMOVEDFILES _("Unexpected file removals were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file removals between builds.")

/** @} */

/**
 * @defgroup addedfiles inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_ADDEDFILES
 *
 * How to handle unexpected file additions.
 */
#define REMEDY_ADDEDFILES _("Unexpected file additions were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file additions between builds.  If they are correct, update %s and send a patch to the rpminspect data project owning that file so rpminspect knows to expect this change.  You may also need to update the data package or local configuration file and change the forbidden_path_prefixes or forbidden_path_suffixes list.")

/** @} */

/**
 * @defgroup upstream inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_UPSTREAM
 *
 * How to handle unexpected changes in upstream source content.
 */
#define REMEDY_UPSTREAM _("Unexpected changed source archive content. The version of the package did not change between builds, but the source archive content did. This may be deliberate, but needs inspection. If this change is expected, update %s and send a patch to the project that owns that file.")

/** @} */

/**
 * @defgroup ownership inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_OWNERSHIP_DEFATTR
 *
 * How to handle unexpected ownership settings in the RPM payload.
 */
#define REMEDY_OWNERSHIP_DEFATTR _("Make sure the %%files section includes the %%defattr macro. If these permissions are expected, update %s and send a patch to the project that owns it.")

/**
 * @def REMEDY_OWNERSHIP_BIN_OWNER
 *
 * How to handle incorrect ownership of bin path files.
 */
#define REMEDY_OWNERSHIP_BIN_OWNER _("Bin path files must be owned by the bin_owner set in the rpminspect configuration, which is usually root. If this ownership is expected, update %s and send a patch to the project that owns it.")

/**
 * @def REMEDY_OWNERSHIP_BIN_GROUP
 *
 * How to handle incorrect group ownership of bin path files.
 */
#define REMEDY_OWNERSHIP_BIN_GROUP _("Bin path files must be owned by the bin_group set in the rpminspect configuration, which is usually root. If this ownership is expect, update %s and send a patch to the project that owns it.")

/**
 * @def REMEDY_OWNERSHIP_IXOTH
 *
 * How to handle files with the world execute bit set where it should not be.
 */
#define REMEDY_OWNERSHIP_IXOTH _("Either chgrp the file to the bin_group set in the rpminspect configuration or remove the world execute bit on the file (chmod o-x). If this ownership is expected, update %s and send a patch to the project that owns it.")

/**
 * @def REMEDY_OWNERSHIP_IWGRP
 *
 * How to handle files with the group write bit set where it should not be.
 */
#define REMEDY_OWNERSHIP_IWGRP _("Either chgrp the file to the bin_group set in the rpminspect configuration or remove the group write bit on the file (chmod g-w). If this ownership is expected, update %s and send a patch to the project that owns it.")

/**
 * @def REMEDY_OWNERSHIP_CHANGED
 *
 * How to handle unexpected file and directory ownership changes.
 */
#define REMEDY_OWNERSHIP_CHANGED _("Verify the ownership changes are expected. If not, adjust the package build process to set correct owner and group information. If expected, update %s and send a patch to the project that owns it.")

/**
 * @def REMEDY_FILEINFO_RULE
 *
 * How to handle a missing fileinfo rule.
 */
#define REMEDY_FILEINFO_RULE _("rpminspect is expecting a fileinfo rule from the vendor data package for this file. Usually this means the file carries a non-standard set of permissions (e.g., setuid) which is a condition where rpminspect would check the fileinfo list to ensure the package conforms to the vendor rules. To remedy, add a fileinfo rule for this file to the vendor data package under the appropriate product release file.")

/** @} */

/**
 * @defgroup shellsyntax inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_SHELLSYNTAX
 *
 * How to handle shell syntax checker errors.
 */
#define REMEDY_SHELLSYNTAX _("Consult the shell documentation for proper syntax.")

/**
 * @def REMEDY_SHELLSYNTAX_GAINED_SHELL
 *
 * How to handle an existing file that has now become a shell script.
 */
#define REMEDY_SHELLSYNTAX_GAINED_SHELL _("The file referenced was not a known shell script in the before build but is now a shell script in the after build.")

/**
 * @def REMEDY_SHELLSYNTAX_BAD
 *
 * How to address invalid syntax in a shell script.
 */
#define REMEDY_SHELLSYNTAX_BAD _("The referenced shell script is invalid. Consider debugging it with the '-n' option on the shell to find and fix the problem.")

/** @} */

/**
 * @defgroup annocheck inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_ANNOCHECK
 *
 * How to handle annocheck(1) findings.
 */
#define REMEDY_ANNOCHECK _("See annocheck(1) for more information.")

/**
 * @def REMEDY_ANNOCHECK_FORTIFY_SOURCE
 *
 * How to handle annocheck(1) reporting a file is not built with _FORTIFY_SOURCE enabled.
 */
#define REMEDY_ANNOCHECK_FORTIFY_SOURCE _("Ensure all object files are compiled with '-O2 -D_FORTIFY_SOURCE=2', and that all appropriate headers are included (no implicit function declarations). Symbols may also appear as unfortified if the compiler is unable to determine the size of a buffer, which is not necessarily an error.")

/** @} */

/**
 * @defgroup dsodeps inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_DSODEPS
 *
 * How to handle a change in DT_NEEDED symbols between builds.
 */
#define REMEDY_DSODEPS _("DT_NEEDED symbols have been added or removed.  This happens when the build environment has different versions of the required libraries.  Sometimes this is deliberate but sometimes not.  Verify these changes are expected.  If they are not, modify the package spec file to ensure the build links with the correct shared libraries.")

/** @} */

/**
 * @defgroup filesize inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_FILESIZE_GREW
 *
 * How to address an unexpected increase in file size.
 */
#define REMEDY_FILESIZE_GREW _("A file grew by a noticeable amount.  Ensure this change is intended.  If it is, you can adjust the filesize inspection settings in the rpminspect.yaml file.")

/**
 * @def REMEDY_FILESIZE_SHRANK
 *
 * How to address an unexpected decrease in file size.
 */
#define REMEDY_FILESIZE_SHRANK _("A file shrank by a noticeable amount.  Ensure this change is intended.  If it is, you can adjust the filesize inspection settings in the rpminspect.yaml file.")

/**
 * @def REMEDY_FILESIZE_BECAME_NOT_EMPTY
 *
 * How to address a previously known empty file now containing data.
 */
#define REMEDY_FILESIZE_BECAME_NOT_EMPTY _("A previously empty file is no longer empty.  Make sure this change is intended and fix the package spec file if necessary.")

/**
 * @def REMEDY_FILESIZE_BECAME_EMPTY
 *
 * How to address an existing file becoming empty.
 */
#define REMEDY_FILESIZE_BECAME_EMPTY _("A previously non-empty file is now empty.  Make sure this change is intended and fix the package space file if necessary.")

/** @} */

/**
 * @defgroup capabilities inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_CAPABILITIES
 *
 * How to address unexpected file capabilities(7).
 */
#define REMEDY_CAPABILITIES _("Unexpected capabilities were found on the indicated file.  Consult capabilities(7) and either adjust the files in the package or modify the capabilities list in the rpminspect vendor data package.  The security team may also be of help for this situation.  If necessary, update %s with the changes found here and send a patch to the project that owns the data file.")

/** @} */

/**
 * @defgroup kmod inspection remedy strings
 *
 * @{
 */

/**
 * @def REMEDY_KMOD_PARM
 *
 * How to address a loss of kernel module parameters between builds.
 */
#define REMEDY_KMOD_PARM _("Kernel module parameters were removed between builds.  This may present usability problems for users if module parameters were removed in a maintenance update.")

/**
 * @def REMEDY_KMOD_DEPS
 *
 * How to address a change in kernel module dependencies between builds.
 */
#define REMEDY_KMOD_DEPS _("Kernel module dependencies changed between builds.  This may present usability problems for users if module dependencies changed in a maintenance update.")

/**
 * @def REMEDY_KMOD_ALIAS
 *
 * How to address a change in kernel module device aliases between builds.
 */
#define REMEDY_KMOD_ALIAS _("Kernel module device aliases changed between builds.  This may present usability problems for users if module device aliases changed in a maintenance update.")

/** @} */

/**
 * @defgroup arch inspection remedy strings
 *
 * @{
 */

#define REMEDY_ARCH_LOST _("An architecture present in the before build is now missing in the after build.  This may be deliberate, but check to make sure you do not have any unexpected ExclusiveArch lines in the spec file.")
#define REMEDY_ARCH_GAIN _("A new architecture has appeared in the after build.  This may indicate progress in the world of computing.")

/** @} */

/**
 * @defgroup subpackages inspection remedy strings
 *
 * @{
 */

#define REMEDY_SUBPACKAGES_LOST _("A subpackage present in the before build is now missing in the after build.  This may be deliberate, but check to make sure you have correct syntax defining the subpackage in the spec file")
#define REMEDY_SUBPACKAGES_GAIN _("A new subpackage has appeared in the after build.  This may indicate progress in the world of computing.")

/** @} */

/**
 * @defgroup changelog inspection remedy strings
 *
 * @{
 */

#define REMEDY_CHANGELOG _("Make sure the spec file in the after build contains a valid %changelog section.")

/** @} */

/**
 * @defgroup pathmigration inspection remedy strings
 *
 * @{
 */

#define REMEDY_PATHMIGRATION _("Files should not be installed in old directory names.  Modify the package to install the affected file to the preferred directory.")

/** @} */

/**
 * @defgroup lto inspection remedy strings
 *
 * @{
 */

#define REMEDY_LTO _("ELF .o and .a files should not carry LTO (Link Time Optimization) bytecode.  Make sure you have stripped LTO bytecode from those files at install time.")

/** @} */

/**
 * @defgroup symlinks inspection remedy strings
 *
 * @{
 */

#define REMEDY_SYMLINKS _("Make sure symlinks point to a valid destination in one of the subpackages of the build; dangling symlinks are not allowed.  If you are comparing builds and have a non-symlink turn in to a symlink, ensure this is deliberate.  NOTE:  You cannot turn a directory in to a symlink due to RPM limitations.");
#define REMEDY_SYMLINKS_DIRECTORY _("Make sure symlinks point to a valid destination in one of the subpackages of the build; dangling symlinks are not allowed.  If you are comparing builds and have a non-symlink turn in to a symlink, ensure this is deliberate.  NOTE:  You cannot turn a directory in to a symlink due to RPM limitations.  If you absolutely must do that, make sure you include the %pretrans scriptlet for replacing a directory.  See the packaging guidelines for 'Scriptlet to replace a directory' for more information.");

/** @} */

/**
 * @defgroup files inspection remedy strings
 *
 * @{
 */

#define REMEDY_FILES _("Remove forbidden path references from the indicated line in the %files section.  In many cases you can use RPM macros to specify path locations.  See the RPM documentation or distribution package maintainer guide for more information.")

/** @} */

/**
 * @defgroup types inspection remedy strings
 *
 * @{
 */

#define REMEDY_TYPES _("In many cases the changing MIME type is deliberate.  Verify that the change is intended and if necessary fix the spec file so the correct file is included in the built package.")

/** @} */

/**
 * @defgroup abidiff inspection remedy strings
 *
 * @{
 */

#define REMEDY_ABIDIFF _("ABI changes introduced during maintenance updates can lead to problems for users.  See the abidiff(1) documentation and the distribution ABI policies to determine if this detected change is allowed.")

/** @} */

/**
 * @defgroup kmidiff inspection remedy strings
 *
 * @{
 */

#define REMEDY_KMIDIFF _("Kernel Module Interface introduced during maintenance updates can lead to problems for users.  See the libabigail documentation and the distribution KMI policy to determine if this detected change is allowed.")

/** @} */

/**
 * @defgroup config inspection remedy strings
 *
 * @{
 */

#define REMEDY_CONFIG _("Changes to %config should be done carefully.  Make sure you have installed the correct file and in the correct location.  If a package is restructuring configuration files, make sure the package can handle upgrading an existing package -or- honor the old file locations.")

/** @} */

/**
 * @defgroup doc inspection remedy strings
 *
 * @{
 */

#define REMEDY_DOC _("Changes found among the %doc files.  Verify these changes are intended if the package is not a rebase.  Sometimes upstream projects rename or move documentation files and the spec file needs to account for those changes.")

/** @} */

/**
 * @defgroup patches inspection remedy strings
 *
 * @{
 */

#define REMEDY_PATCHES_CORRUPT _("An invalid patch file was found.  This is usually the result of generating a collection of patches by comparing two trees.  When files disappear that can lead to zero length patches in the resulting collection.  Check to see if the source package has any zero length or otherwise invalid patches and correct the problem.")

#define REMEDY_PATCHES_MISSING_MACRO _("The named patch is defined in the source RPM header (this means it has a PatchN: definition in the spec file) but is not applied anywhere in the spec file.  It is missing a corresponding %patch macro and the spec file lacks the %autosetup or %autopatch macros.  You can fix this by adding the appropriate %patch macro in the spec file (usually in the %prep section).  The number specified with the %patch macro corresponds to the number used to define the patch at the top of the spec file.  So Patch47 is applied with either a '%patch 47', '%patch -P 47', '%patch -P47', or '%patch47' macro.")

#define REMEDY_PATCHES_MISMATCHED_MACRO _("The named patch is defined but is mismatched by number with the %patch macro.  Make sure all numbered patches have corresponding %patch macros.  For example, Patch47 needs to have either a '%patch 47', '%patch -P 47', '%patch -P47', or '%patch47' macro.")

#define REMEDY_PATCHES_UNHANDLED_PATCH _("The defined patch file is not something rpminspect can handle.  This is likely a bug and should be reported to the upstream rpminspect project.")

/** @} */

/**
 * @defgroup virus inspection remedy strings
 *
 * @{
 */

#define REMEDY_VIRUS _("ClamAV has found a virus in the named file.  This may be a false positive, but you should manually inspect the file in question to ensure it is clean.  This may be a problem with the ClamAV database or detection.  If you are sure the file in question is clean, please file a bug with rpminspect for further help.")

/** @} */

/**
 * @defgroup politics inspection remedy strings
 *
 * @{
 */

#define REMEDY_POLITICS _("A file with potential politically sensitive content was found in the package.  If this file is permitted, it should be added to the rpminspect vendor data package for the product.  Modify the %s file and send a patch to the project that owns it.")

/** @} */

/**
 * @defgroup badfuncs inspection remedy strings
 *
 * @{
 */

#define REMEDY_BADFUNCS _("Forbidden symbols were found in an ELF file in the package.  The configuration settings for rpminspect indicate the named symbols are forbidden in packages.  If this is deliberate, you may want to disable the badfuncs inspection.  If it is not deliberate, check the man pages for the named symbols to see what API functions have replaced the forbidden symbols.  Usually a function is marked as deprecated but still provided in order to allow for backwards compatibility.  Whenever possible the deprecated functions should not be used.")

/** @} */

/**
 * @defgroup runpath inspection remedy strings
 *
 * @{
 */

#define REMEDY_RUNPATH _("Either DT_RPATH or DT_RUNPATH properties were found on ELF shared objects in this package.  The use of DT_RPATH and DT_RUNPATH is discouraged except in certain situations.  Check to see that you a disabling rpath during the %build stage of the spec file.  If you are unable to do this easily, you can try using a program such as patchelf to remove these properties from the ELF files.")

#define REMEDY_RUNPATH_BOTH _("Both DT_RPATH and DT_RUNPATH properties were found in an ELF shared object.  This indicates a linker error and should not happen.  ELF objects should only carry DT_RPATH or DT_RUNPATH, never both.")

/** @} */

/**
 * @defgroup unicode inspection remedy strings
 *
 * @{
 */

#define REMEDY_UNICODE _("The rpminspect configuration file contains a list of forbidden Unicode code points.  One was found in the extracted and patched source tree or in one of the text source files in the source RPM.  Either remove this code point or discuss the situation with the Product Security Team to determine the correct course of action.")

#define REMEDY_UNICODE_PREP_FAILED _("The %prep section of the spec file could not be executed for some reason.  This usually results from a failure in librpmbuild, which is usually tied to archive extraction problems or the filesystem changing while rpminspect is running.  A common cause is removal of the working directory while the program is executing.")

/** @} */

/**
 * @defgroup rpmdeps inspection remedy strings
 *
 * @{
 */

#define REMEDY_RPMDEPS_MACROS _("Unexpanded RPM spec file macros were found in the noted dependency rule.  Check the spec file for this dependency and ensure you have not misspelled a macro or used a macro name that does not exist.")

#define REMEDY_RPMDEPS_EXPLICIT _("Add the indicated explicit Requires to the spec file for the named subpackage.  Subpackages depending on shared libraries in another subpackage must carry an explicit 'Requires: SUBPACKAGE_NAME = %{version}-%{release}' in the spec file.")

#define REMEDY_RPMDEPS_EXPLICIT_EPOCH _("Add the indicated explicit Requires to the spec file for the named subpackage.  Subpackages depending on shared libraries in another subpackage must carry an explicit 'Requires: SUBPACKAGE_NAME = %{epoch}:%{version}-%{release}' in the spec file.")

#define REMEDY_RPMDEPS_MULTIPLE _("Check subpackage %files sections and explicit Provides statements.  Only one subpackage should provide a given shared library.  Shared library names are automatically added as Provides, so there is no need to specify them in the spec file but you do need to make sure only one subpackage is packaging up the shared library in question.")

#define REMEDY_RPMDEPS_CHANGED _("A dependency listed in the before build changed to the indicated dependency in the after build.  If this is a VERIFY result, it means rpminspect noticed the change in what it considers a maintenance update in a package.  An INFO result means it noticed this change, but deems it ok because it is comparing a rebased build.")

#define REMEDY_RPMDEPS_GAINED _("A new dependency is seen in the after build that was not present in the before build.  If this is a VERIFY result, it means rpminspect noticed the change in what it considers a maintenance update in a package.  An INFO result means it noticed this change, but deems it ok because it is comparing a rebased build.")

#define REMEDY_RPMDEPS_LOST _("A dependency seen in the before build is not seen in the after build meaning it was removed or lost.  If this is a VERIFY result, it means rpminspect noticed the change in what it considers a maintenance update in a package.  An INFO result means it noticed this change, but deems it ok because it is comparing a rebased build.")

#define REMEDY_RPMDEPS_EPOCH _("The package has an Epoch value greater than zero, but the explicit subpackage dependencies are not consistently using it.  For the dependency reported, the '= %{version}-%{release}' needs to change to '= %{epoch}:%{version}-%{release}' to capture the package package Epoch in the dependency.")

/** @} */

/**
 * @def REMEDY_UDEVRULES
 *
 * How to address syntax check issues in udev rules files.
 */
#define REMEDY_UDEVRULES _("Refer to the udev documentation at https://www.freedesktop.org/software/systemd/man/udev.html for help correcting the errors and warnings.")

/**
 * @def REMEDY_SCRIPTLETS
 *
 * How to address scriptlet issues.
 */
#define REMEDY_SCRIPTLETS _("Refer to the packaging guidelines.")

/**
 * @def REMEDY_REQUIRED_USERADD_OPTIONS
 *
 * The required options for the useradd command in RPM scriptlets.
 */
#define REMEDY_REQUIRED_USERADD_OPTIONS "The useradd(8) command requires either the -r or --system option to specify a system account with automatic UID selection or the -u or --uid option with an explicit UID value specified.  The UID value specified must be below the UID value boundary in the configuration file.  Lastly, the useradd(8) command requires the use of the -s or --shell option to specify the login shell, but the shell must either be `/usr/sbin/nologin' or `/sbin/nologin'."

/**
 * @def REMEDY_USERADD_OPTION_UID
 *
 * The required -u/--uid option.
 */
#define REMEDY_USERADD_OPTION_UID "The useradd(8) command requires either the -r or --system option to specify a system account with automatic UID selection or the -u or --uid option with an explicit UID value specified.  The UID value specified must be below the UID value boundary in the configuration file."

/**
 * @def REMEDY_USERADD_UID_BOUNDARY
 *
 * The upper UID boundary for the -u/--uid option.
 */
#define REMEDY_USERADD_UID_BOUNDARY "The UID value specified with the -u or --uid option of the useradd(8) command must be below the UID value boundary in the configuration file."

/**
 * @def REMEDY_USERADD_OPTION_SHELL
 *
 * The -s/--shell option for the useradd command.
 */
#define REMEDY_USERADD_OPTION_SHELL "The useradd(8) command requires the use of the -s or --shell option to specify the login shell, but the shell must either be `/usr/sbin/nologin' or `/sbin/nologin'."

/** @} */

/** @} */

#endif

#ifdef __cplusplus
}
#endif
