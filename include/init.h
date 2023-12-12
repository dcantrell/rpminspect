/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _LIBRPMINSPECT_INIT_H
#define _LIBRPMINSPECT_INIT_H

/*
 * Config file keywords; used by read_cfgfile().  These are keywords
 * not already defined as another macro.
 */

#define RI_ALLOWED                  "allowed"
#define RI_ALLOWED_ORIGIN_PATHS     "allowed_origin_paths"
#define RI_ALLOWED_PATHS            "allowed_paths"
#define RI_ALLOWED_UIDS             "allowed_uids"
#define RI_AUTOMACROS               "automacros"
#define RI_BADWORDS                 "badwords"
#define RI_BIN_GROUP                "bin_group"
#define RI_BIN_OWNER                "bin_owner"
#define RI_BIN_PATHS                "bin_paths"
#define RI_BUILDHOST_SUBDOMAIN      "buildhost_subdomain"
#define RI_COMMANDS                 "commands"
#define RI_COMMON                   "common"
#define RI_CONFLICTS                "conflicts"
#define RI_DEBUGINFO_PATH           "debuginfo_path"
#define RI_DEBUGINFO_SECTIONS       "debuginfo_sections"
#define RI_DESKTOP_ENTRY_FILES_DIR  "desktop_entry_files_dir"
#define RI_DESKTOP_FILE_VALIDATE    "desktop-file-validate"
#define RI_DESKTOP_SKIP_EXEC_CHECK  "skip_exec_check"
#define RI_DESKTOP_SKIP_ICON_CHECK  "skip_icon_check"
#define RI_DOWNLOAD_MBS             "download_mbs"
#define RI_DOWNLOAD_URSINE          "download_ursine"
#define RI_ENHANCES                 "enhances"
#define RI_ENVIRONMENT              "environment"
#define RI_EXCLUDED_MIME_TYPES      "excluded_mime_types"
#define RI_EXCLUDED_PATHS           "excluded_paths"
#define RI_EXCLUDE                  "exclude"
#define RI_EXCLUDE_PATH             "exclude_path"
#define RI_EXPECTED_EMPTY           "expected_empty"
#define RI_EXTRA_ARGS               "extra_args"
#define RI_EXTRA_OPTS               "extra_opts"
#define RI_FAILURE_SEVERITY         "failure_severity"
#define RI_FAVOR_RELEASE            "favor_release"
#define RI_FILENAME                 "filename"
#define RI_FORBIDDEN_CODEPOINTS     "forbidden_codepoints"
#define RI_FORBIDDEN_DIRECTORIES    "forbidden_directories"
#define RI_FORBIDDEN                "forbidden"
#define RI_FORBIDDEN_GROUPS         "forbidden_groups"
#define RI_FORBIDDEN_OWNERS         "forbidden_owners"
#define RI_FORBIDDEN_PATH_PREFIXES  "forbidden_path_prefixes"
#define RI_FORBIDDEN_PATHS          "forbidden_paths"
#define RI_FORBIDDEN_PATH_SUFFIXES  "forbidden_path_suffixes"
#define RI_FULL                     "full"
#define RI_HEADER_FILE_EXTENSIONS   "header_file_extensions"
#define RI_HUB                      "hub"
#define RI_IGNORE                   "ignore"
#define RI_IGNORE_LIST              "ignore_list"
#define RI_IGNORES                  "ignores"
#define RI_INCLUDE_PATH             "include_path"
#define RI_INFO                     "info"
#define RI_INFO_ONLY0               "info_only"
#define RI_INFO_ONLY1               "info-only"
#define RI_INSPECTIONS              "inspections"
#define RI_JOBS                     "jobs"
#define RI_KABI_DIR                 "kabi_dir"
#define RI_KABI_FILENAME            "kabi_filename"
#define RI_KERNEL_FILENAMES         "kernel_filenames"
#define RI_KOJI                     "koji"
#define RI_LICENSEDB                "licensedb"
#define RI_LTO_SYMBOL_NAME_PREFIXES "lto_symbol_name_prefixes"
#define RI_MACROFILES               "macrofiles"
#define RI_MATCH                    "match"
#define RI_MIGRATED_PATHS           "migrated_paths"
#define RI_MSGUNFMT                 "msgunfmt"
#define RI_NAME                     "name"
#define RI_NEWEST                   "newest"
#define RI_OBSOLETES                "obsoletes"
#define RI_OFF                      "off"
#define RI_OLDEST                   "oldest"
#define RI_ON                       "on"
#define RI_ORIGIN_PREFIX_TRIM       "origin_prefix_trim"
#define RI_PREFIX                   "prefix"
#define RI_PRIMARY                  "primary"
#define RI_PRODUCT_RELEASE          "product_release"
#define RI_PRODUCTS                 "products"
#define RI_PROFILEDIR               "profiledir"
#define RI_PROFILE                  "profile"
#define RI_PROVIDES                 "provides"
#ifdef _HAVE_MODULARITYLABEL
#define RI_RECOMMEND                "recommend"
#endif
#define RI_RECOMMENDS               "recommends"
#ifdef _HAVE_MODULARITYLABEL
#define RI_RELEASE_REGEXP           "release_regexp"
#endif
#ifdef _HAVE_MODULARITYLABEL
#define RI_REQUIRED                 "required"
#endif
#define RI_REQUIRES                 "requires"
#define RI_SECURITY_KEYWORDS        "security_keywords"
#define RI_SECURITY_LEVEL_THRESHOLD "security_level_threshold"
#define RI_SECURITY_PATH_PREFIX     "security_path_prefix"
#define RI_SHELLS                   "shells"
#define RI_SIZE_THRESHOLD           "size_threshold"
#define RI_STATIC_CONTEXT           "static_context"
#define RI_SUFFIX                   "suffix"
#define RI_SUGGESTS                 "suggests"
#define RI_SUPPLEMENTS              "supplements"
#define RI_SUPPRESSION_FILE         "suppression_file"
#define RI_UDEVADM                  "udevadm"
#define RI_UDEV_RULES_DIRS          "udev_rules_dirs"
#define RI_UID_BOUNDARY             "uid_boundary"
#define RI_VENDOR_DATA_DIR          "vendor_data_dir"
#define RI_VENDOR                   "vendor"
#define RI_WORKDIR                  "workdir"

#endif

#ifdef __cplusplus
}
#endif
