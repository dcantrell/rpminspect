/*
 * Copyright 2022 Red Hat, Inc.
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

#ifndef _LIBRPMINSPECT_INIT_H
#define _LIBRPMINSPECT_INIT_H

#define SECTION_ABIDIFF                  "abidiff"
#define SECTION_ALLOWED                  "allowed"
#define SECTION_ALLOWED_ORIGIN_PATHS     "allowed_origin_paths"
#define SECTION_ALLOWED_PATHS            "allowed_paths"
#define SECTION_ANNOCHECK                "annocheck"
#define SECTION_AUTOMACROS               "automacros"
#define SECTION_BADWORDS                 "badwords"
#define SECTION_BIN_GROUP                "bin_group"
#define SECTION_BIN_OWNER                "bin_owner"
#define SECTION_BIN_PATHS                "bin_paths"
#define SECTION_BUILDHOST_SUBDOMAIN      "buildhost_subdomain"
#define SECTION_COMMANDS                 "commands"
#define SECTION_COMMON                   "common"
#define SECTION_CONFLICTS                "conflicts"
#define SECTION_DEBUGINFO_PATH           "debuginfo_path"
#define SECTION_DEBUGINFO_SECTIONS       "debuginfo_sections"
#define SECTION_DESKTOP_ENTRY_FILES_DIR  "desktop_entry_files_dir"
#define SECTION_DESKTOP_FILE_VALIDATE    "desktop-file-validate"
#define SECTION_DOWNLOAD_MBS             "download_mbs"
#define SECTION_DOWNLOAD_URSINE          "download_ursine"
#define SECTION_ENHANCES                 "enhances"
#define SECTION_ENVIRONMENT              "environment"
#define SECTION_EXCLUDED_MIME_TYPES      "excluded_mime_types"
#define SECTION_EXCLUDED_PATHS           "excluded_paths"
#define SECTION_EXCLUDE_PATH             "exclude_path"
#define SECTION_EXPECTED_EMPTY           "expected_empty"
#define SECTION_EXTRA_ARGS               "extra_args"
#define SECTION_EXTRA_OPTS               "extra_opts"
#define SECTION_FAILURE_SEVERITY         "failure_severity"
#define SECTION_FAVOR_RELEASE            "favor_release"
#define SECTION_FORBIDDEN_CODEPOINTS     "forbidden_codepoints"
#define SECTION_FORBIDDEN_DIRECTORIES    "forbidden_directories"
#define SECTION_FORBIDDEN_GROUPS         "forbidden_groups"
#define SECTION_FORBIDDEN_OWNERS         "forbidden_owners"
#define SECTION_FORBIDDEN_PATH_PREFIXES  "forbidden_path_prefixes"
#define SECTION_FORBIDDEN_PATHS          "forbidden_paths"
#define SECTION_FORBIDDEN_PATH_SUFFIXES  "forbidden_path_suffixes"
#define SECTION_HEADER_FILE_EXTENSIONS   "header_file_extensions"
#define SECTION_HUB                      "hub"
#define SECTION_IGNORE                   "ignore"
#define SECTION_IGNORE_LIST              "ignore_list"
#define SECTION_INCLUDE_PATH             "include_path"
#define SECTION_INSPECTIONS              "inspections"
#define SECTION_JOBS                     "jobs"
#define SECTION_KABI_DIR                 "kabi_dir"
#define SECTION_KABI_FILENAME            "kabi_filename"
#define SECTION_KERNEL_FILENAMES         "kernel_filenames"
#define SECTION_KMIDIFF                  "kmidiff"
#define SECTION_KOJI                     "koji"
#define SECTION_LICENSEDB                "licensedb"
#define SECTION_LTO_SYMBOL_NAME_PREFIXES "lto_symbol_name_prefixes"
#define SECTION_MACROFILES               "macrofiles"
#define SECTION_MATCH                    "match"
#define SECTION_MIGRATED_PATHS           "migrated_paths"
#define SECTION_MSGUNFMT                 "msgunfmt"
#define SECTION_OBSOLETES                "obsoletes"
#define SECTION_ORIGIN_PREFIX_TRIM       "origin_prefix_trim"
#define SECTION_PRIMARY                  "primary"
#define SECTION_PRODUCT_RELEASE          "product_release"
#define SECTION_PRODUCTS                 "products"
#define SECTION_PROFILE                  "profile"
#define SECTION_PROFILEDIR               "profiledir"
#define SECTION_PROVIDES                 "provides"
#define SECTION_RECOMMENDS               "recommends"
#define SECTION_REQUIRES                 "requires"
#define SECTION_SECURITY_LEVEL_THRESHOLD "security_level_threshold"
#define SECTION_SECURITY_PATH_PREFIX     "security_path_prefix"
#define SECTION_SHELLS                   "shells"
#define SECTION_SIZE_THRESHOLD           "size_threshold"
#define SECTION_STATIC_CONTEXT           "static_context"
#define SECTION_SUGGESTS                 "suggests"
#define SECTION_SUPPLEMENTS              "supplements"
#define SECTION_SUPPRESSION_FILE         "suppression_file"
#define SECTION_VENDOR_DATA_DIR          "vendor_data_dir"
#define SECTION_VENDOR                   "vendor"
#define SECTION_WORKDIR                  "workdir"

#define TOKEN_NONE                       "none"
#define TOKEN_OLDEST                     "oldest"
#define TOKEN_NEWEST                     "newest"
#define TOKEN_FULL                       "full"
#define TOKEN_PREFIX                     "prefix"
#define TOKEN_SUFFIX                     "suffix"
#define TOKEN_NAME                       "name"
#define TOKEN_FILENAME                   "filename"
#define TOKEN_INFO                       "info"
#define TOKEN_INFO_ONLY                  "info-only"
#define TOKEN_INFO_ONLY2                 "info_only"
#define TOKEN_ON                         "on"
#define TOKEN_OFF                        "off"
#define TOKEN_REQUIRED                   "required"
#define TOKEN_FORBIDDEN                  "forbidden"
#define TOKEN_RECOMMEND                  "recommend"

#endif
