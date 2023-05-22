/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*
 * This header defines types used by librpminspect
 */

#include <regex.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmfi.h>
#include <unicode/utypes.h>

#ifdef _WITH_LIBKMOD
#ifdef _LIBKMOD_HEADER_SUBDIR
#include <kmod/libkmod.h>
#else
#include <libkmod.h>
#endif /* _LIBKMOD_HEADER_HEADER */
#endif /* _WITH_LIBKMOD */

#ifdef _WITH_LIBCAP
#include <sys/capability.h>
#endif

#include "secrules.h"
#include "queue.h"
#include "uthash.h"

#ifndef _LIBRPMINSPECT_TYPES_H
#define _LIBRPMINSPECT_TYPES_H

/*
 * List of strings. Used by some of the inspections.
 */
typedef struct _string_entry_t {
    char *data;
    TAILQ_ENTRY(_string_entry_t) items;
} string_entry_t;

typedef TAILQ_HEAD(string_entry_s, _string_entry_t) string_list_t;

/*
 * List of UChars.  Used by at least the unicode inspection.
 */
typedef struct _UChar_entry_t {
    UChar data;
    TAILQ_ENTRY(_UChar_entry_t) items;
} UChar_entry_t;

typedef TAILQ_HEAD(UChar_entry_s, _UChar_entry_t) UChar_list_t;

/*
 * List of string pairs. Used to later convert in to a newly allocated hash table.
 */
typedef struct _pair_entry_t {
    char *key;
    char *value;
    TAILQ_ENTRY(_pair_entry_t) items;
} pair_entry_t;

typedef TAILQ_HEAD(pair_entry_s, _pair_entry_t) pair_list_t;

/*
 * A file is information about a file in an RPM payload.
 *
 * If fullpath is not NULL, it is the absolute path of the unpacked
 * file.  Not every file is unpacked (e.g., block and char special
 * files are skipped).  The ownership and permissions of the unpacked
 * file may not match the intended owner and mode from the RPM
 * metadata.
 *
 * "localpath" is file path from the RPM payload, and "st" is the
 * metadata about the file, as described by the RPM payload. localpath
 * and st may not necessarily match the description of the file in the
 * RPM header.
 *
 * The rpm_header field is shared by multiple files. Each file entry
 * must call headerFree to dereference the header.
 *
 * idx is the index for this file into the RPM array tags such as
 * RPMTAG_FILESIZES.
 *
 * type is the MIME type string that you would get from 'file
 * --mime-type'.
 *
 * cap is the getcap() value for the file.
 *
 * checksum is a string containing the human-readable checksum digest
 *
 * moved_path is true if the file moved path locations between the
 * before and after build, false otherwise
 *
 * moved_subpackage is true if the file moved between subpackages
 * between the before and after build, false otherwise.
 */
typedef struct _rpmfile_entry_t {
    Header rpm_header;
    char *fullpath;
    char *localpath;
    struct stat st;
    int idx;
    char *type;
    char *checksum;
#ifdef _WITH_LIBCAP
    cap_t cap;
#endif
    rpmfileAttrs flags;
    struct _rpmfile_entry_t *peer_file;
    bool moved_path;
    bool moved_subpackage;
    TAILQ_ENTRY(_rpmfile_entry_t) items;
} rpmfile_entry_t;

typedef TAILQ_HEAD(rpmfile_s, _rpmfile_entry_t) rpmfile_t;

/*
 * RPM dependency information
 */

/* Dependency types */
typedef enum _dep_type_t {
    TYPE_NULL = 0,
    TYPE_REQUIRES = 1,
    TYPE_PROVIDES = 2,
    TYPE_CONFLICTS = 3,
    TYPE_OBSOLETES = 4,
    TYPE_ENHANCES = 5,
    TYPE_RECOMMENDS = 6,
    TYPE_SUGGESTS = 7,
    TYPE_SUPPLEMENTS = 8
} dep_type_t;

#define FIRST_DEP_TYPE TYPE_REQUIRES
#define LAST_DEP_TYPE TYPE_SUPPLEMENTS

/* Possible dependency operators */
typedef enum _dep_op_t {
    OP_NULL = 0,
    OP_EQUAL = 1,
    OP_LESS = 2,
    OP_GREATER = 3,
    OP_LESSEQUAL = 4,
    OP_GREATEREQUAL = 5
} dep_op_t;

/* Individual dependency entries and the list type */
typedef struct _deprule_entry_t {
    dep_type_t type;                       /* dependency type */
    char *requirement;                     /* dependency requirement name (e.g., glibc or /bin/sh) */
    dep_op_t operator;                     /* dependency operator (e.g., >, >=) */
    char *version;                         /* dependency version */
    bool rich;                             /* true if this dep is a rich dependency */
    bool explicit;                         /* true if this dep is matched for automatic shared lib deps */
    string_list_t *providers;              /* for TYPE_REQUIRES, list of subpackages providing it */
    struct _deprule_entry_t *peer_deprule; /* corresponding before/after deprule */
    TAILQ_ENTRY(_deprule_entry_t) items;
} deprule_entry_t;

typedef TAILQ_HEAD(deprule_entry_s, _deprule_entry_t) deprule_list_t;

/* Dependency rule ignore rules */
typedef struct _deprule_ignore_map_t {
    dep_type_t type;                  /* dependency rule type */
    regex_t *ignore;                  /* compiled pattern */
    char *pattern;                    /* pattern in config file for debug output */
    UT_hash_handle hh;
} deprule_ignore_map_t;

/*
 * A peer is a mapping of a built RPM from the before and after builds.
 * We can expand this struct as necessary based on what tests need to
 * reference.
 */
typedef struct _rpmpeer_entry_t {
    Header before_hdr;                       /* RPM header of the before package */
    Header after_hdr;                        /* RPM header of the after package */
    char *before_rpm;                        /* full path to the before RPM */
    char *after_rpm;                         /* full path to the after RPM */
    char *before_root;                       /* full path to the before RPM extracted root dir */
    char *after_root;                        /* full path to the after RPM extracted root dir */
    rpmfile_t *before_files;                 /* list of files in the payload of the before RPM */
    rpmfile_t *after_files;                  /* list of files in the payload of the after RPM */
    deprule_list_t *before_deprules;         /* dependency rules for the before RPM */
    deprule_list_t *after_deprules;          /* dependency rules for the after RPM */
    unsigned long int before_unpacked_size;  /* size of unpacked RPM payload */
    unsigned long int after_unpacked_size;   /* size of unpacked RPM payload */
    TAILQ_ENTRY(_rpmpeer_entry_t) items;
} rpmpeer_entry_t;

typedef TAILQ_HEAD(rpmpeer_s, _rpmpeer_entry_t) rpmpeer_t;

/*
 * And individual inspection result and the list to hold them.
 * NOTE: This enum needs to go from least bad to worst result
 * codes because that's how rpminspect determines what the exit
 * code of the program is.  For example, RESULT_OK needs to be
 * a lower int than RESULT_BAD.
 */
typedef enum _severity_t {
    RESULT_NULL   = 0,      /* used to indicate internal error */
    RESULT_DIAG   = 1,      /* only used by the 'diagnostics' inspection */
    RESULT_SKIP   = 2,
    RESULT_OK     = 3,
    RESULT_INFO   = 4,
    RESULT_VERIFY = 5,
    RESULT_BAD    = 6
} severity_t;

typedef enum _waiverauth_t {
    NULL_WAIVERAUTH      = 0,
    NOT_WAIVABLE         = 1,
    WAIVABLE_BY_ANYONE   = 2,
    WAIVABLE_BY_SECURITY = 3
} waiverauth_t;

typedef enum _verb_t {
    VERB_NIL = 0,       /* not used, same as "not set" */
    VERB_ADDED = 1,     /* new file or metadata */
    VERB_REMOVED = 2,   /* removed file or metadata */
    VERB_CHANGED = 3,   /* changed file or metadata */
    VERB_FAILED = 4,    /* check failing */
    VERB_OK = 5,        /* the everything is ok alarm */
    VERB_SKIP = 6       /* for skipped inspections or checks */
} verb_t;

/*
 * struct to make it easier to make multiple calls to add_result()
 * See the results_entry_t for details.
 */
struct result_params {
    severity_t severity;
    waiverauth_t waiverauth;
    const char *header;
    char *msg;
    char *details;
    char *remedy;
    verb_t verb;
    const char *noun;
    const char *arch;
    const char *file;
};

typedef struct _results_entry_t {
    severity_t severity;      /* see results.h */
    waiverauth_t waiverauth;  /* who can waive an inspection result */
    const char *header;       /* header string for reporting */
    char *msg;                /* the result message */
    char *details;            /* details (optional, can be NULL) */
    char *remedy;             /* suggested correction for the result */
    verb_t verb;              /* verb indicating what happened */
    char *noun;               /* noun impacted by 'verb', one line
                                 (e.g., a file path or an RPM dependency
                                        string) */
    char *arch;               /* architecture impacted (${ARCH}) */
    char *file;               /* file impacted (${FILE}) */
    TAILQ_ENTRY(_results_entry_t) items;
} results_entry_t;

typedef TAILQ_HEAD(results_s, _results_entry_t) results_t;

/*
 * Known types of Koji builds
 */
typedef enum _koji_build_type_t {
    KOJI_BUILD_NULL = 0,       /* initializer, not an actual build */
    KOJI_BUILD_IMAGE = 1,      /* not supported */
    KOJI_BUILD_MAVEN = 2,      /* not supported */
    KOJI_BUILD_MODULE = 3,
    KOJI_BUILD_RPM = 4,
    KOJI_BUILD_WIN = 5         /* not supported */
} koji_build_type_t;

/*
 * fileinfo for a product release. Used by some of the inspections.
 */
typedef enum _fileinfo_field_t {
    MODE = 0,
    OWNER = 1,
    GROUP = 2,
    FILENAME = 3
} fileinfo_field_t;

typedef struct _fileinfo_entry_t {
    mode_t mode;
    char *owner;
    char *group;
    char *filename;
    TAILQ_ENTRY(_fileinfo_entry_t) items;
} fileinfo_entry_t;

typedef TAILQ_HEAD(fileinfo_entry_s, _fileinfo_entry_t) fileinfo_t;

/*
 * caps list for a product release.  Used by some of the inspections.
 */
typedef enum _caps_field_t {
    PACKAGE = 0,
    FILEPATH = 1,
    EQUAL = 2,
    CAPABILITIES = 3
} caps_field_t;

typedef struct _caps_filelist_entry_t {
    char *path;
    char *caps;
    TAILQ_ENTRY(_caps_filelist_entry_t) items;
} caps_filelist_entry_t;

typedef TAILQ_HEAD(caps_filelist_entry_s, _caps_filelist_entry_t) caps_filelist_t;

typedef struct _caps_entry_t {
    char *pkg;
    caps_filelist_t *files;
    TAILQ_ENTRY(_caps_entry_t) items;
} caps_entry_t;

typedef TAILQ_HEAD(caps_entry_s, _caps_entry_t) caps_t;

#ifdef _HAVE_MODULARITYLABEL

/* Modularity static context types */
typedef enum _static_context_t {
    STATIC_CONTEXT_NULL = 0,
    STATIC_CONTEXT_REQUIRED = 1,
    STATIC_CONTEXT_FORBIDDEN = 2,
    STATIC_CONTEXT_RECOMMEND = 3
} static_context_t;

#endif

/* Spec filename matching types */
typedef enum _specname_match_t {
    MATCH_NULL = 0,
    MATCH_FULL = 1,
    MATCH_PREFIX = 2,
    MATCH_SUFFIX = 3
} specname_match_t;

typedef enum _specname_primary_t {
    PRIMARY_NULL = 0,
    PRIMARY_NAME = 1,
    PRIMARY_FILENAME = 2
} specname_primary_t;

/* RPM header cache so we don't balloon out our memory */
typedef struct _header_cache_t {
    char *pkg;
    Header hdr;
    UT_hash_handle hh;
} header_cache_t;

/* Product release string favoring */
typedef enum _favor_release_t {
    FAVOR_NONE = 0,
    FAVOR_OLDEST = 1,
    FAVOR_NEWEST = 2
} favor_release_t;

/*
 * Politics list
 */
typedef struct _politics_entry_t {
    char *pattern;
    char *digest;
    bool allowed;
    TAILQ_ENTRY(_politics_entry_t) items;
} politics_entry_t;

typedef TAILQ_HEAD(politics_entry_s, _politics_entry_t) politics_list_t;

typedef enum _politics_field_t {
    PATTERN = 0,
    DIGEST = 1,
    PERMISSION = 2
} politics_field_t;

/* Commands used by rpminspect at runtime. */
struct command_paths {
    char *diff;
    char *msgunfmt;
    char *desktop_file_validate;
    char *abidiff;
    char *kmidiff;
#ifdef _WITH_ANNOCHECK
    char *annocheck;
#endif
};

/* Hash table used for key/value situations where each is a string. */
typedef struct _string_map_t {
    char *key;
    char *value;
    UT_hash_handle hh;
} string_map_t;

/* Hash table with a string key and a string_list_t value. */
typedef struct _string_list_map_t {
    char *key;
    string_list_t *value;
    UT_hash_handle hh;
} string_list_map_t;

/*
 * Security rule actions hash table
 * There is one of these for each row in the vendor security
 * definitions file.
 */
typedef struct _secrule_t {
    int type;
    severity_t severity;
    UT_hash_handle hh;
} secrule_t;

/*
 * Security rules list
 * This list holds the rows as read from the vendor security
 * definitions files.
 */
typedef struct _security_entry_t {
    char *path;
    char *pkg;
    char *ver;
    char *rel;
    secrule_t *rules;
    TAILQ_ENTRY(_security_entry_t) items;
} security_entry_t;

typedef TAILQ_HEAD(security_entry_s, _security_entry_t) security_list_t;

/*
 * Patches hash table used by the patches inspection
 * Maps the patch file name to the patch number in the spec file (that
 * is, the PatchN: and corresponding %patchN number where N is the
 * number).
 */
typedef struct _patches_t {
    char *patch;
    long num;              /* -1 means the patch file has no PatchN line */
    UT_hash_handle hh;
} patches_t;

/*
 * Hash table used to record the number of %patchN macros used
 */
typedef struct _applied_patches_t {
    long num;
    char *opts;
    UT_hash_handle hh;
} applied_patches_t;

/*
 * Configuration and state instance for librpminspect run.
 * Applications using librpminspect should initialize the
 * library and retain this structure through the run of
 * their program.  You need to free this struct on exit.
 */
struct rpminspect {
    char *progname;            /* Full path to the program */
    string_list_t *cfgfiles;   /* list of full path config files read (in order) */
    char *localcfg;            /* Name of the optional local config file */
    string_list_t *locallines; /* Contents of optional local config file */
    char *workdir;             /* full path to working directory */
    char *profiledir;          /* full path to profiles directory */
    char *worksubdir;          /* within workdir, where these builds go */

    /* Commands */
    struct command_paths commands;

    /* environment section present? */
    bool have_environment;

    /* Vendor data */
    char *vendor_data_dir;     /* main vendor data directory */
    string_list_t *licensedb;  /* names of files under licenses/ to use */
    favor_release_t favor_release;

    /* Populated at runtime for the product release */
    char *fileinfo_filename;
    fileinfo_t *fileinfo;
    caps_t *caps;
    char *caps_filename;
    string_list_t *rebaseable;
    char *rebaseable_filename;
    politics_list_t *politics;
    char *politics_filename;
    security_list_t *security;
    char *security_filename;
    bool security_initialized;
    string_list_t *icons;
    char *icons_filename;
    bool librpm_initialized;

    /* Koji information (from config file) */
    char *kojihub;             /* URL of Koji hub */
    char *kojiursine;          /* URL to access packages built in Koji */
    char *kojimbs;             /* URL to access module packages in Koji */

    /* Information used by different tests */
    string_list_t *badwords;   /* Space-delimited list of words prohibited
                                * from certain package strings.
                                */
    char *vendor;              /* Required vendor string */

#ifdef _HAVE_MODULARITYLABEL
    /* Modularity values */
    static_context_t modularity_static_context;
#endif

    /* Required subdomain for buildhosts -- multiple subdomains allowed */
    string_list_t *buildhost_subdomain;

    /* Optional: list of RPM macro file paths */
    string_list_t *macrofiles;
    bool macros_loaded;

    /*
     * Optional: if not NULL, contains list of path prefixes for files
     * that are of security concern.
     */
    string_list_t *security_path_prefix;

    /*
     * Optional: if not NULL, contains a list of filename extensions
     * for C and C++ header files.
     */
    string_list_t *header_file_extensions;

    /*
     * Optional: Lists of path substrings and directories to forbid.
     */
    string_list_t *forbidden_path_prefixes;
    string_list_t *forbidden_path_suffixes;
    string_list_t *forbidden_directories;

    /*
     * Optional: List of auto macros that handle patch setup.
     */
    string_list_t *automacros;

    /*
     * Optional: if not NULL, contains a list of forbidden function
     * names.
     */
    string_list_t *bad_functions;

    /*
     * Optional: if not NULL, contains a map of file paths in packages
     * and a list of allowed forbidden functions it can use.
     */
    string_list_map_t *bad_functions_allowed;

    /* Optional: if not NULL, contains list of architectures */
    /* if not specified on the command line, this becomes the list of
       all architectures downloaded */
    string_list_t *arches;

    regex_t *elf_path_include;
    regex_t *elf_path_exclude;
    regex_t *manpage_path_include;
    regex_t *manpage_path_exclude;
    regex_t *xml_path_include;
    regex_t *xml_path_exclude;

    /* copies of regex pattern strings used for debug mode output */
    char *elf_path_include_pattern;
    char *elf_path_exclude_pattern;
    char *manpage_path_include_pattern;
    char *manpage_path_exclude_pattern;
    char *xml_path_include_pattern;
    char *xml_path_exclude_pattern;

    /* Where desktop entry files live */
    char *desktop_entry_files_dir;

    /* Executable path prefixes and required ownership */
    string_list_t *bin_paths;
    char *bin_owner;
    char *bin_group;

    /* Optional: Forbidden file owners and groups */
    string_list_t *forbidden_owners;
    string_list_t *forbidden_groups;

    /* List of shells to check script syntax */
    string_list_t *shells;

    /* Optional: file size change threshold for inc/dec reporting (%) */
    long int size_threshold;

    /* Optional: ELF LTO symbol prefixes */
    string_list_t *lto_symbol_name_prefixes;

    /* Spec filename matching type */
    specname_match_t specmatch;
    specname_primary_t specprimary;

    /* hash table of product release -> JVM major versions */
    string_map_t *jvm;

    /* hash table of annocheck tests */
    string_map_t *annocheck;
    severity_t annocheck_failure_severity;
    char *annocheck_profile;

    /* hash table of path migrations */
    string_map_t *pathmigration;
    string_list_t *pathmigration_excluded_paths;

    /* hash table of product release regexps */
    string_map_t *products;

    /* list of paths to ignore (these strings allow glob(3) syntax) */
    string_list_t *ignores;

    /* list of forbidden path references for %files sections */
    string_list_t *forbidden_paths;

    /* name of the optional ABI suppression file in the SRPM */
    char *abidiff_suppression_file;

    /* path where debuginfo files are found in packages */
    char *abidiff_debuginfo_path;

    /* extra arguments for abidiff(1) */
    char *abidiff_extra_args;

    /* ABI compat level security reporting threshold */
    long int abi_security_threshold;

    /* name of the optional ABI suppression file in the SRPM */
    char *kmidiff_suppression_file;

    /* path where debuginfo files are found in packages */
    char *kmidiff_debuginfo_path;

    /* extra arguments for kmidiff(1) */
    char *kmidiff_extra_args;

    /* list of valid kernel executable filenames (e.g., "vmlinux") */
    string_list_t *kernel_filenames;

    /*
     * directory where kernel ABI (KABI) files are kept (in any
     * subpackage in a kernel build)
     */
    char *kabi_dir;

    /*
     * name of KABI files in kabi_dir, can use $ARCH or ${ARCH} for
     * architecture
     */
    char *kabi_filename;

    /* list of patches to ignore in the 'patches' inspection */
    string_list_t *patch_ignore_list;

    /* runpath inspection lists */
    string_list_t *runpath_allowed_paths;
    string_list_t *runpath_allowed_origin_paths;
    string_list_t *runpath_origin_prefix_trim;

    /* optional per-inspection ignore lists */
    /*
     * NOTE: the 'ignores' struct member is a global list of ignore
     * globs.  This hash table provides per-inspection ignore globs.
     */
    string_list_map_t *inspection_ignores;

    /* Optional list of expected RPMs with empty payloads */
    string_list_t *expected_empty_rpms;

    /* unicode inspection lists */
    regex_t *unicode_exclude;
    string_list_t *unicode_excluded_mime_types;
    string_list_t *unicode_forbidden_codepoints;

    /* RPM dependency ignores -- regexps to match requirements to ignore */
    deprule_ignore_map_t *deprules_ignore;

    /* debuginfo ELF section name(s) when checking for debugging symbols */
    char *debuginfo_sections;

    /* Options specified by the user */
    char *before;              /* before build ID arg given on cmdline */
    char *after;               /* after build ID arg given on cmdline */
    uint64_t tests;            /* which tests to run (default: ALL) */
    bool verbose;              /* verbose inspection output? */
    bool rebase_detection;     /* Is rebase detection enabled for
                                  builds? (default true) */

    /* Failure threshold and results suppression threshold */
    severity_t threshold;
    severity_t worst_result;
    severity_t suppress;

    /* The product release we are inspecting against */
    char *product_release;

    /* The type of Koji build we are looking at */
    /*
     * NOTE: rpminspect works with RPMs at the lowest level, so
     * build types that are other collections of RPMs may be
     * supported. But supporting non-RPM containers in rpminspect
     * is not really in scope.
     */
    koji_build_type_t buildtype;

    /* accumulated data of the build set */
    rpmpeer_t *peers;               /* list of packages */
    header_cache_t *header_cache;   /* RPM header cache */
    char *before_rel;               /* before Release w/o %{?dist} */
    char *after_rel;                /* after Release w/o ${?dist} */
    int rebase_build;               /* indicates if this is a rebased build */

    /* specific to module builds */
    bool before_static_context;
    bool after_static_context;

    /* local disk space requirements */
    unsigned long int download_size;
    unsigned long int unpacked_size;

    /* spec file macros */
    pair_list_t *macros;

    /* inspection results */
    results_t *results;
};

/*
 * Definition for a build type.
 */
struct buildtype {
    /* the build type that maps to koji_build_type_t */
    koji_build_type_t type;

    /* name of the build type */
    char *name;

    /* whether or not this type is supported */
    bool supported;
};

/*
 * Definition for an output format.
 */
struct format {
    /* The output format type from output.h */
    int type;

    /* short name of the format */
    char *name;

    /* output driver function */
    void (*driver)(const results_t *, const char *, const severity_t, const severity_t);
};

/*
 * Definition for an inspection.  Inspections are assigned a flag (see
 * inspect.h), a short name, and a function pointer to the driver.  The
 * driver function needs to take a struct rpminspect pointer as the only
 * argument.  The driver returns true on success and false on failure.
 */
struct inspect {
    /* the inspection flag from inspect.h */
    uint64_t flag;

    /* short name of inspection */
    char *name;

    /*
     * Does this inspection perform any security checks?  That is,
     * checks that if they fail would result in a WAIVABLE_BY_SECURITY
     * result.
     */
    bool security_checks;

    /*
     * Does this inspection require a before and after package?
     * Some inspections run just against a single build, which
     * we can use when the user wants to run rpminspect against
     * a single build.
     *
     * True if this inspection is for a single build (which is
     * always the 'after' build throughout the code).
     */
    bool single_build;

    /* the driver function for the inspection */
    bool (*driver)(struct rpminspect *);
};

/*
 * List of RPMs from a Koji build.  Only the information we need.
 */
typedef struct _koji_rpmlist_entry_t {
    char *arch;
    char *name;
    char *version;
    char *release;
    int32_t epoch;
    unsigned long int size;
    TAILQ_ENTRY(_koji_rpmlist_entry_t) items;
} koji_rpmlist_entry_t;

typedef TAILQ_HEAD(koji_rpmlist_s, _koji_rpmlist_entry_t) koji_rpmlist_t;

/*
 * List of build IDs from a Koji build.
 */
typedef struct _koji_buildlist_entry_t {
    int32_t build_id;
    char *package_name;
    char *owner_name;
    int32_t task_id;
    int32_t state;
    char *nvr;
    char *start_time;
    int32_t create_event;
    int32_t creation_event_id;
    char *creation_time;
    int32_t epoch;
    int32_t tag_id;
    char *completion_time;
    char *tag_name;
    char *version;
    int32_t volume_id;
    char *release;
    int32_t package_id;
    int32_t owner_id;
    int32_t id;
    char *volume_name;
    char *name;

    /* List of RPMs in this build */
    koji_rpmlist_t *rpms;

    TAILQ_ENTRY(_koji_buildlist_entry_t) builditems;
} koji_buildlist_entry_t;

typedef TAILQ_HEAD(koji_buildlist_s, _koji_buildlist_entry_t) koji_buildlist_t;

/*
 * Koji build structure.  This is determined by looking at the
 * output of a getBuild XMLRPC call to a Koji hub.
 *
 * You can examine example values by running xmlrpc on the command
 * line and giving it a build specification, e.g.:
 *     xmlrpc KOJI_HUB_URL getBuild NVR
 * Not all things returned are represented in this struct.
 */
struct koji_build {
    /* These are all relevant to the name of the build */
    char *package_name;
    int32_t epoch;
    char *name;
    char *version;
    char *release;
    char *nvr;

    /* The source used to drive this build (usually a VCS link) */
    char *source;

    /* Koji-specific information about the build */
    char *creation_time;
    char *completion_time;
    int32_t package_id;
    int32_t id;
    int32_t state;
    double completion_ts;
    int32_t owner_id;
    char *owner_name;
    char *start_time;
    int32_t creation_event_id;
    double start_ts;
    double creation_ts;
    int32_t task_id;

    /* Where to find the resulting build artifacts */
    int32_t volume_id;
    char *volume_name;

    /*
     * Original source URL, for some reason
     * (not present for module builds)
     */
    char *original_url;

    /* Content Generator information (currently not used in rpminspect) */
    int32_t cg_id;
    char *cg_name;

    /* Module metadata -- only if this build is a module */
    char *modulemd_str;
    char *module_name;
    char *module_stream;
    int32_t module_build_service_id;
    char *module_version;
    char *module_context;
    char *module_content_koji_tag;

    /*
     * Total size of all RPMs (restricted to specified architectures
     * by user)
     */
    unsigned long int total_size;

    /*
     * Total size of all unpacked RPMs (restricted to specified
     * architectures by user)
     */
    unsigned long int total_unpacked_size;

    /* List of build IDs associated with this build */
    koji_buildlist_t *builds;
};

/*
 * Koji task structure.  This is determined by looking at the
 * output of a getTaskInfo XMLRPC call to a Koji hub.
 *
 * You can examine example values by running xmlrpc on the command
 * line and giving it a task ID, e.g.:
 *     xmlrpc KOJI_HUB_URL getTaskInfo ID
 * Not all things returned are represented in this struct.
 */
typedef TAILQ_HEAD(koji_task_list_s, _koji_task_entry_t) koji_task_list_t;

struct koji_task {
    /* members returned from getTaskInfo */
    double weight;
    int32_t parent;
    char *completion_time;
    char *start_time;
    double start_ts;
    bool waiting;
    bool awaited;
    char *label;
    int32_t priority;
    int32_t channel_id;
    int32_t state;
    char *create_time;
    double create_ts;
    int32_t owner;
    int32_t host_id;
    char *method;
    double completion_ts;
    char *arch;
    int32_t id;

    /*
     * Total size of all RPMs (restricted to specified architectures
     * by user)
     */
    unsigned long int total_size;

    /*
     * Total size of all unpacked RPMs (restricted to specified
     * architectures by user)
     */
    unsigned long int total_unpacked_size;

    /* Descendent tasks (where files are) */
    koji_task_list_t *descendents;
};

/* A generic list of koji tasks */
typedef struct _koji_task_entry_t {
    /* main task information */
    struct koji_task *task;

    /* results from getTaskResult */
    int32_t brootid;
    string_list_t *srpms;
    string_list_t *rpms;
    string_list_t *logs;

    TAILQ_ENTRY(_koji_task_entry_t) items;
} koji_task_entry_t;

/* Types of files -- used by internal nftw() callbacks */
typedef enum _filetype_t {
    FILETYPE_NULL = 0,
    FILETYPE_EXECUTABLE = 1,
    FILETYPE_ICON = 2
} filetype_t;

/* Kernel module handling */
#ifdef _WITH_LIBKMOD

typedef void (*modinfo_to_entries)(string_list_t *, const struct kmod_list *);
typedef void (*module_alias_callback)(const char *, const string_list_t *, const string_list_t *, void *);

/* mapping of an alias string to a module name */
typedef struct _kernel_alias_data_t {
    char *alias;
    string_list_t *modules;
    UT_hash_handle hh;
} kernel_alias_data_t;

#endif

/* Types of workdirs */
typedef enum _workdir_t {
    NULL_WORKDIR = 0,          /* unused                    */
    LOCAL_WORKDIR = 1,         /* locally cached koji build */
    TASK_WORKDIR = 2,          /* like for scratch builds   */
    BUILD_WORKDIR = 3          /* remote koji build spec    */
} workdir_t;

/**
 * @brief Callback function to pass to foreach_peer_file.
 *
 * Given the program's main struct rpminspect and a single
 * rpmfile_entry_t, perform a defined action and return true if the
 * action was successful and false otherwise.  This is used for
 * callback functions in inspection functions.  You can iterate over
 * each file in a package and perform the inspection.  True means
 * everything passed, false means something failed.  Since the
 * callback received the struct rpminspect, you can add results as the
 * program returns and not worry about losing inspection details.
 */
typedef bool (*foreach_peer_file_func)(struct rpminspect *, rpmfile_entry_t *);

/* Types of ELF information we can return */
typedef enum _elfinfo_t {
    ELF_TYPE    = 0,
    ELF_MACHINE = 1
} elfinfo_t;

/*
 * Exit status for abidiff and abicompat tools.  It's actually a bit
 * mask.  The value of each enumerator is a power of two.
 *
 * (This is lifted from abg-tools-utils.h in libabigail because it
 * cannot be included directly because libabigail is C++.)
 */

/*
 * This is for when the compared ABIs are equal.
 * Its numerical value is 0.
 */
#define ABIDIFF_OK 0

/*
 * This bit is set if there an application error.
 * Its numerical value is 1.
 */
#define ABIDIFF_ERROR 1

/*
 * This bit is set if the tool is invoked in an non appropriate
 * manner.
 * Its numerical value is 2.
 */
#define ABIDIFF_USAGE_ERROR (1 << 1)

/*
 * This bit is set if the ABIs being compared are different.
 * Its numerical value is 4.
 */
#define ABIDIFF_ABI_CHANGE (1 << 2)

/*
 * This bit is set if the ABIs being compared are different *and*
 * are incompatible.
 *
 * Its numerical value is 8.
 */
#define ABIDIFF_ABI_INCOMPATIBLE_CHANGE (1 << 3)

/*
 * ABI compatibility level types
 */
typedef struct _abi_t {
    char *pkg;
    int level;
    bool all;
    string_list_t *dsos;
    UT_hash_handle hh;
} abi_t;

/*
 * Patch stats for reporting in the patches inspection
 */
typedef struct _patchstat_t {
    long int files;
    long int lines;
} patchstat_t;

#endif
