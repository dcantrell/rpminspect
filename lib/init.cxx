/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <libgen.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <iostream>
#include <toml++/toml.hpp>
#include "internal/callbacks.h"
#include "parser.h"
#include "rpminspect.h"
#include "init.h"
#include "remedy.h"
#include "queue.h"
#include "uthash.h"

/*
 * Debug printing on the config file parser is verbose, so it can be
 * disabled by commenting this line out and uncommented the
 * INIT_DEBUG_PRINT line that expands to void.
 */
/* #define INIT_DEBUG_PRINT DEBUG_PRINT */
#define INIT_DEBUG_PRINT(...) (void)(0)

/* List defaults (not in constants.h to avoid cpp shenanigans) */

/**
 * @def BIN_PATHS
 * NULL terminated array of strings of paths where executable files
 * may reside.
 */
static const char *BIN_PATHS[] = {"/bin", "/sbin", "/usr/bin", "/usr/sbin", NULL};

/**
 * @def SHELLS
 * NULL terminated array of strings of shells to use for syntax
 * checking (only the basename is needed).  All shells listed must
 * support the '-n' option for syntax checking.  The shell should exit
 * 0 if the syntax checker passes, non-zero otherwise.  The 'rc' shell
 * is an exception and has special handling in the 'shellsyntax'
 * inspection.
 */
static const char *SHELLS[] = {"sh", "ksh", "zsh", "csh", "tcsh", "rc", "bash", NULL};

/* Supported config filename endings (we assume type by this) */
static const char *CFG_FILENAME_EXTENSIONS[] = {"yaml", "json", "dson", NULL};

/**
 * @def UDEV_RULES_DIRS
 * NULL terminated array of strings of directories where udev rules files
 * may reside.
 */
static const char *UDEV_RULES_DIRS[] = {"/etc/udev/rules.d/", "/usr/lib/udev/rules.d/", NULL};

/* flag indicating the remedy strings have been initialized */
static bool remedy_initialized = false;

static int add_regex(const char *pattern, regex_t **regex_out)
{
    int reg_result;
    char *errbuf = NULL;
    size_t errbuf_size;

    if (pattern == NULL || *pattern == '\0') {
        return 0;
    }

    assert(regex_out);

    free_regex(*regex_out);
    *regex_out = (regex_t *) calloc(1, sizeof(regex_t));
    assert(*regex_out != NULL);

    if ((reg_result = regcomp(*regex_out, pattern, REG_EXTENDED)) != 0) {
        /* Get the size needed for the error message */
        errbuf_size = regerror(reg_result, *regex_out, NULL, 0);
        errbuf = (char *) malloc(errbuf_size);
        assert(errbuf != NULL);

        regerror(reg_result, *regex_out, errbuf, errbuf_size);
        warnx("*** %s", errbuf);

        /* Clean up and return error */
        free(errbuf);
        free(*regex_out);
        return -1;
    }

    return 0;
}

/*
 * Given an inspection identifier from the config file reader and a
 * list value, add it to the per-inspection list of ignores.
 */
static void add_string_list_map_entry(string_list_map_t **table, const char *key, const char *value)
{
    string_list_map_t *mapentry = NULL;

    assert(key != NULL);
    assert(value != NULL);

    /* look for the list first, add if not found */
    HASH_FIND_STR(*table, key, mapentry);

    if (mapentry == NULL) {
        /* start a new entry for this inspection */
        mapentry = (string_list_map_t *) calloc(1, sizeof(*mapentry));
        assert(mapentry != NULL);
        mapentry->key = strdup(key);
        mapentry->value = list_add(mapentry->value, value);
        HASH_ADD_KEYPTR(hh, *table, mapentry->key, strlen(mapentry->key), mapentry);
    } else {
        /* add to existing ignore list */
        mapentry->value = list_add(mapentry->value, value);
    }

    return;
}

/**
 * Given a key and value, initialize a new hash table (clearing the
 * old one if necessary) and migrate the data from the list to the
 * hash table.  Clear the incoming hash table list.  The caller is
 * responsible for freeing all memory allocated to these structures.
 *
 * If required is set to true, this function will only append the
 * value to an existing key in the table.  Use this parameter if
 * processing a secondary table and you require the key to exist from
 * a previous processing run.
 *
 * NOTE: The incoming table entries will be removed and freed as the
 * data is migrated to the table and keys.  Before return, the
 * incoming table list will be freed.  The actual key and value
 * strings in each incoming table entry are not freed since they just
 * migrate over to the new structures, but the entries themselves are
 * removed and freed from the incoming table.
 *
 * @param key The key in the hash table (string).
 * @param value The value to add to the hash table (string).
 * @param required Set to true if key must exist to append value to list.
 * @param single Set to true if key must contain only a single value,
 *               otherwise it's a string list.
 * @param table Destination hash table.
 */
static void process_table(const char *key, const char *value, const bool required, const bool single, string_map_t **table)
{
    char *tmp = NULL;
    string_list_t *tokens = NULL;
    string_map_t *entry = NULL;

    assert(key != NULL);
    assert(value != NULL);

    /* look for the key first */
    HASH_FIND_STR(*table, key, entry);

    if (entry == NULL) {
        if (required) {
            warnx("*** missing key `%s', unable to append `%s'", key, value);
        } else {
            /* add the new key/value pair to the table */
            entry = (string_map_t *) calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->key = strdup(key);
            entry->value = strdup(value);
            HASH_ADD_KEYPTR(hh, *table, entry->key, strlen(entry->key), entry);
        }
    } else {
        /* entry found, handle the value */
        if (entry->value != NULL && !single) {
            tokens = strsplit(entry->value, " ");
            assert(tokens != NULL);

            tokens = list_add(tokens, value);

            tmp = list_to_string(tokens, " ");
            list_free(tokens, free);
        } else {
            tmp = strdup(value);
        }

        assert(tmp != NULL);
        free(entry->value);
        entry->value = tmp;
    }

    return;
}

/*
 * Convert a 10 character mode string for a file to a mode_t
 * For example, convert "-rwsr-xr-x" to a mode_t
 */
static mode_t parse_mode(const char *input)
{
    mode_t mode = 0;
    char i;

    assert(input != NULL);

    if (strlen(input) != 10) {
        warnx(_("*** invalid input string `%s`"), input);
        return mode;
    }

        /* type */
    i = input[0];
    if (i == 'd') {
        mode |= S_IFDIR;
    } else if (i == 'c') {
        mode |= S_IFCHR;
    } else if (i == 'b') {
        mode |= S_IFBLK;
    } else if (i == '-') {
        mode |= S_IFREG;
    } else if (i == 'l') {
        mode |= S_IFLNK;
    } else if (i == 's') {
        mode |= S_IFSOCK;
#ifdef S_IFIFO
    } else if (i == 'p') {
        mode |= S_IFIFO;
#endif
#ifdef S_IFWHT
    } else if (i == 'w') {
        mode |= S_IFWHT;
#endif
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    /* owner */
    i = input[1];
    if (i == 'r') {
        mode |= S_IRUSR;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[2];
    if (i == 'w') {
        mode |= S_IWUSR;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[3];
    if (i == 'x') {
        mode |= S_IXUSR;
    } else if (i == 'S') {
        mode |= S_ISUID;
    } else if (i == 's') {
        mode |= S_IXUSR | S_ISUID;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    /* group */
    i = input[4];
    if (i == 'r') {
        mode |= S_IRGRP;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[5];
    if (i == 'w') {
        mode |= S_IWGRP;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[6];
    if (i == 'x') {
        mode |= S_IXGRP;
    } else if (i == 'S') {
        mode |= S_ISGID;
    } else if (i == 's') {
        mode |= S_IXGRP | S_ISGID;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    /* other */
    i = input[7];
    if (i == 'r') {
        mode |= S_IROTH;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[8];
    if (i == 'w') {
        mode |= S_IWOTH;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    i = input[9];
    if (i == 'x') {
        mode |= S_IXOTH;
    } else if (i == 'T') {
        mode |= S_ISVTX;
    } else if (i == 't') {
        mode |= S_IXOTH | S_ISVTX;
    } else if (i != '-') {
        warnx(_("*** invalid mode string: %s"), input);
        return mode;
    }

    return mode;
}

/*
 * Retrieve a string from configuration, updating the specified field if it
 * has already been defined.
 */
static inline void strget(parser_plugin *p, parser_context *ctx, const char *key1, const char *key2, char **dest)
{
    char *res;

    res = p->getstr(ctx, key1, key2);

    if (res == NULL) {
        return;
    }

    free(*dest);
    *dest = res;
    return;
}

/* lambda for add_ignores() below. */
static bool add_ignores_cb(const char *entry, void *cb_data)
{
    add_ignores_cb_data *d = (add_ignores_cb_data *) cb_data;
    add_string_list_map_entry(d->inspection_ignores, d->inspection, entry);
    return false;
}

/* Load and add an inspection's ignores section. */
static inline void add_ignores(struct rpminspect *ri, parser_plugin *p, parser_context *ctx, const char *inspection)
{
    add_ignores_cb_data data = { &ri->inspection_ignores, inspection };

    if (p->strarray_foreach(ctx, inspection, RI_IGNORE, add_ignores_cb, &data)
        && p->strarray_foreach(ctx, inspection, RI_IGNORES, add_ignores_cb, &data)) {
        warnx(_("*** problem adding ignore entries to %s"), inspection);
    }

    return;
}

/*
 * Handle inclusion and exclusion paths for an inspection.  Pass the bare name
 * of an inspection, not a string.  Reuses variables from context.
 */
#define ADD_INCL_EXCL(inspection)                                       \
    s = p->getstr(ctx, #inspection, RI_INCLUDE_PATH);                   \
                                                                        \
    if (s != NULL) {                                                    \
        if (debug_mode) {                                               \
            ri->inspection ## _path_include_pattern = strdup(s);        \
        }                                                               \
                                                                        \
        if (add_regex(s, &ri->inspection ## _path_include) != 0) {      \
            warnx(_("*** error reading " #inspection " include path"));      \
        }                                                               \
                                                                        \
        free(s);                                                        \
    }                                                                   \
                                                                        \
    s = p->getstr(ctx, #inspection, RI_EXCLUDE_PATH);                   \
                                                                        \
    if (s != NULL) {                                                    \
        if (debug_mode) {                                               \
            ri->inspection ## _path_exclude_pattern = strdup(s);        \
        }                                                               \
                                                                        \
        if (add_regex(s, &ri->inspection ## _path_exclude) != 0) {      \
            warnx(_("*** error reading " #inspection " include path"));      \
        }                                                               \
                                                                        \
        free(s);                                                        \
    }

/* lambda for tabledict below. */
static bool tabledict_cb(const char *key, const char *value, void *cb_data)
{
    tabledict_cb_data *data = (tabledict_cb_data *) cb_data;

    /* javabytecode uses this at top-level, but also supports ignores. */
    if (!strcasecmp(key, RI_IGNORE)) {
        return false;
    }

    process_table(key, value, data->required, data->single, data->table);
    return false;
}

/* Process a dictionary of strings into the specified table. */
static void tabledict(parser_plugin *p, parser_context *ctx, const char *key1, const char *key2, string_map_t **table, bool required, bool single)
{
    tabledict_cb_data data = { required, single, table };

    if (p->strdict_foreach(ctx, key1, key2, tabledict_cb, &data)) {
        warnx(_("*** ignoring malformed section %s->%s"), key1, key2);
    }

    return;
}

/* lambda for handling old-style top-level annocheck jobs.  A special case of
 * tabledict_cb that skips more keys. */
static bool annocheck_cb(const char *key, const char *value, void *cb_data)
{
    tabledict_cb_data *data = (tabledict_cb_data *) cb_data;

    if (!strcasecmp(key, RI_FAILURE_SEVERITY) || !strcasecmp(key, RI_EXTRA_OPTS) || !strcasecmp(key, RI_IGNORE)) {
        return false;
    }

    process_table(key, value, data->required, data->single, data->table);
    return false;
}

/* lambda to handle entries in the rpmdeps config file section. */
static bool rpmdeps_cb(const char *key, const char *value, void *cb_data)
{
    deprule_ignore_map_t **deprules_ignore = (deprule_ignore_map_t **) cb_data;
    dep_type_t depkey = TYPE_NULL;
    deprule_ignore_map_t *drentry = NULL;

    if (!strcmp(key, RI_REQUIRES)) {
        depkey = TYPE_REQUIRES;
    } else if (!strcmp(key, RI_PROVIDES)) {
        depkey = TYPE_PROVIDES;
    } else if (!strcmp(key, RI_CONFLICTS)) {
        depkey = TYPE_CONFLICTS;
    } else if (!strcmp(key, RI_OBSOLETES)) {
        depkey = TYPE_OBSOLETES;
    } else if (!strcmp(key, RI_ENHANCES)) {
        depkey = TYPE_ENHANCES;
    } else if (!strcmp(key, RI_RECOMMENDS)) {
        depkey = TYPE_RECOMMENDS;
    } else if (!strcmp(key, RI_SUGGESTS)) {
        depkey = TYPE_SUGGESTS;
    } else if (!strcmp(key, RI_SUPPLEMENTS)) {
        depkey = TYPE_SUPPLEMENTS;
    }

    if (depkey != TYPE_NULL) {
        HASH_FIND_INT(*deprules_ignore, &depkey, drentry);
    }

    /* overwrite existing entry, otherwise create new one */
    if (drentry == NULL) {
        drentry = (deprule_ignore_map_t *) calloc(1, sizeof(*drentry));
        assert(drentry != NULL);
        drentry->type = depkey;
        drentry->pattern = strdup(value);

        if (add_regex(value, &drentry->ignore) != 0) {
            warnx(_("*** error reading %s ignore pattern"), get_deprule_desc(depkey));
        }

        HASH_ADD_INT(*deprules_ignore, type, drentry);
    } else {
        free(drentry->pattern);
        drentry->pattern = NULL;
        drentry->type = depkey;
        drentry->pattern = strdup(value);
        regfree(drentry->ignore);
        drentry->ignore = NULL;

        if (add_regex(value, &drentry->ignore) != 0) {
            warnx(_("*** error reading %s ignore pattern"), get_deprule_desc(depkey));
        }
    }

    return false;
}

/* lambda for handle_inspections() below. */
static bool handle_inspections_cb(const char *key, const char *value, void *cb_data)
{
    uint64_t *tests = (uint64_t *) cb_data;
    bool onoff = true;

    if (!strcasecmp(value, RI_ON)) {
        onoff = false;
    } else if (strcasecmp(value, RI_OFF)) {
        warnx(_("*** flag must be 'on' or 'off'; ignoring '%s'"), value);
    }

    if (!process_inspection_flag(key, onoff, tests)) {
        err(RI_PROGRAM_ERROR, _("*** unknown inspections: `%s`"), key);
        return true;
    }

    return false;
}

/* Turn inspections on and off from the "inspections" config section. */
static inline void handle_inspections(struct rpminspect *ri, parser_plugin *p, parser_context *ctx)
{
    if (p->strdict_foreach(ctx, RI_INSPECTIONS, NULL, handle_inspections_cb, &ri->tests)) {
        warnx(_("*** malformed/unknown inspections section"));
    }

    return;
}

/* lambda for adding entries from the badfuncs_allowed configuration. */
static bool badfuncs_allowed_cb(const char *key, const char *value, void *cb_data)
{
    string_list_map_t **bad_functions_allowed = (string_list_map_t **) cb_data;

    add_string_list_map_entry(bad_functions_allowed, key, value);
    return false;
}

/* Process a desktop skip array and merge it in to desktop_skips_t */
static void process_desktop_skips(desktop_skips_t **desktop_skips, string_list_t *slist, int flag)
{
    string_entry_t *sentry = NULL;
    desktop_skips_t *ds = NULL;

    assert(desktop_skips != NULL);

    if (slist == NULL || TAILQ_EMPTY(slist)) {
        return;
    }

    TAILQ_FOREACH(sentry, slist, items) {
        HASH_FIND_STR(*desktop_skips, sentry->data, ds);

        if (ds == NULL) {
            ds = (desktop_skips_t *) calloc(1, sizeof(*ds));
            assert(ds != NULL);
            ds->path = strdup(sentry->data);
            assert(ds->path != NULL);
            ds->flags |= flag;
            HASH_ADD_KEYPTR(hh, *desktop_skips, ds->path, strlen(ds->path), ds);
        } else {
            ds->flags |= flag;
        }
    }

    return;
}

/*
 * Callback function for toml_walk() to read in the data from the
 * remedy override file.
 */
static inline void _remedy_walker(struct toml_node *node __attribute__((unused)), void *data __attribute__((unused)))
{
/*
    char *r = NULL;
    char *n = NULL;
    string_entry_t *entry = NULL;
    struct rpminspect *ri = (struct rpminspect *) data;
    static bool in_remedy = false;
*/

    /* sure, beginning of document, got it */
/*
    if (toml_type(node) == TOML_ROOT) {
        return;
    }
*/
    /* the name in this context is like 'key' in key=value */
/*
    n = toml_name(node);
*/

    /* note if we are in the [remedy] section */
/*
    if (toml_type(node) == TOML_TABLE && n != NULL && !strcmp(n, "remedy")) {
        in_remedy = true;
        free(n);
        return;
    }
*/

    /* gather all strings in the remedy section that match a known remedy */
/*
    if (in_remedy && toml_type(node) == TOML_STRING) {
        r = toml_value_as_string(node);

        if (r) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);
            entry->data = r;

            if (ri->remedy_overrides == NULL) {
                ri->remedy_overrides = (string_list_t *) calloc(1, sizeof(*(ri->remedy_overrides)));
                assert(ri->remedy_overrides != NULL);
                TAILQ_INIT(ri->remedy_overrides);
            }

            TAILQ_INSERT_TAIL(ri->remedy_overrides, entry, items);
        }

        if (!set_remedy(n, r)) {
            warnx("*** '%s' is not a valid remedy identifier", n);

            if (r) {
                TAILQ_REMOVE(ri->remedy_overrides, entry, items);
                free(entry->data);
                free(entry);
            }
        }
    }

    free(n);
*/
    return;
}

/*
 * Parse remedy TOML file and load override remedy strings.
 */
static void read_remedy(const char *remedyfile __attribute__((unused)), struct rpminspect *ri __attribute__((unused)))
{
//    toml::parse_result result;
//    toml::table tbl;
//    auto custom_remedy_strs;
//    string_entry_t *entry = NULL;
//
//    assert(ri != NULL);
//
//    /* no remedyfile defined in the config file, fine */
//    if (remedyfile == NULL) {
//        return;
//    }
//
//    /* parse the remedy file */
//    r = toml::parse(remedyfile);
//
//    if (r == 0) {
//        std::cerr << "toml::parse(): " << r.error() << "\n";
//        return;
//    }
//
//    /* handle the results */
//    tbl = std::move(result).table();
//
//    /* walk the results */
//    custom_remedy_strs = tbl["remedy"];
//
//    if (custom_remedy_strs->for_each([](auto&& remedy) {
//        if constexpr (toml::is_string<decltype(remedy)>) {
//            value() cons
//
//
//
///*
//        {
//            if constexpr (toml::is_number<decltype(el)>)
//                (*el)++;
//            else if constexpr (toml::is_string<decltype(el)>)
//                el = "five"sv;
//        });
//*/
//
//
//
//
//
//    /* gather all strings in the remedy section that match a known remedy */
//    if (in_remedy && toml_type(node) == TOML_STRING) {
//        r = toml_value_as_string(node);
//
//        if (r) {
//            entry = calloc(1, sizeof(*entry));
//            assert(entry != NULL);
//            entry->data = r;
//
//            if (ri->remedy_overrides == NULL) {
//                ri->remedy_overrides = (string_list_t *) calloc(1, sizeof(*(ri->remedy_overrides)));
//                assert(ri->remedy_overrides != NULL);
//                TAILQ_INIT(ri->remedy_overrides);
//            }
//
//            TAILQ_INSERT_TAIL(ri->remedy_overrides, entry, items);
//        }
//
//        if (!set_remedy(n, r)) {
//            warnx("*** '%s' is not a valid remedy identifier", n);
//
//            if (r) {
//                TAILQ_REMOVE(ri->remedy_overrides, entry, items);
//                free(entry->data);
//                free(entry);
//            }
//        }
//    }
//
//
//
//
//
//    }
//
//
//    toml_walk(root, _remedy_walker, ri);
//
    return;
}

/*
 * Read either the main configuration file or a configuration file
 * overlay (profile) and populate the struct rpminspect members.
 */
static void read_cfgfile(struct rpminspect *ri, const char *filename, const bool local)
{
    parser_plugin *p = NULL;
    parser_context *ctx = NULL;
    char *s = NULL;
    tabledict_cb_data annocheck_cb_data = { false, false, &ri->annocheck };
    string_list_t *slist = NULL;

    assert(ri != NULL);
    assert(filename != NULL);

    INIT_DEBUG_PRINT("filename=|%s|\n", filename);

    if (parse_agnostic(filename, &p, &ctx)) {
        warnx(_("*** ignoring malformed %s configuration file: %s"), COMMAND_NAME, filename);
        return;
    }

    /* Processing order doesn't matter, so match data/generic.yaml. */

    /* Read in common settings */
    strget(p, ctx, RI_COMMON, RI_WORKDIR, &ri->workdir);
    strget(p, ctx, RI_COMMON, RI_PROFILEDIR, &ri->profiledir);
    strget(p, ctx, RI_COMMON, RI_REMEDYFILE, &ri->remedyfile);

    if (ri->remedyfile != NULL) {
        /* remedy override strings, try to read in */
        read_remedy(ri->remedyfile, ri);
    }

    /* Read in some other basic settings */
    strget(p, ctx, RI_KOJI, RI_HUB, &ri->kojihub);
    strget(p, ctx, RI_KOJI, RI_DOWNLOAD_URSINE, &ri->kojiursine);
    strget(p, ctx, RI_KOJI, RI_DOWNLOAD_MBS, &ri->kojimbs);
    strget(p, ctx, RI_COMMANDS, RI_MSGUNFMT, &ri->commands.msgunfmt);
    strget(p, ctx, RI_COMMANDS, RI_DESKTOP_FILE_VALIDATE, &ri->commands.desktop_file_validate);
    strget(p, ctx, RI_COMMANDS, RI_ABIDIFF, &ri->commands.abidiff);
    strget(p, ctx, RI_COMMANDS, RI_KMIDIFF, &ri->commands.kmidiff);
    strget(p, ctx, RI_COMMANDS, RI_UDEVADM, &ri->commands.udevadm);
    strget(p, ctx, RI_VENDOR, RI_VENDOR_DATA_DIR, &ri->vendor_data_dir);
    array(p, ctx, RI_VENDOR, RI_LICENSEDB, &ri->licensedb);

    s = p->getstr(ctx, RI_ENVIRONMENT, RI_PRODUCT_RELEASE);

    if (s != NULL) {
        ri->have_environment = true;
        free(ri->product_release);
        ri->product_release = s;
        s = NULL;
    }

    s = p->getstr(ctx, RI_VENDOR, RI_FAVOR_RELEASE);

    if (s != NULL) {
        if (!strcasecmp(s, RI_OLDEST)) {
            ri->favor_release = FAVOR_OLDEST;
        } else if (!strcasecmp(s, RI_NEWEST)) {
            ri->favor_release = FAVOR_NEWEST;
        } else {
            ri->favor_release = FAVOR_NONE;
        }

        free(s);
    }

    handle_inspections(ri, p, ctx);
    tabledict(p, ctx, RI_PRODUCTS, NULL, &ri->products, false, false);
    array(p, ctx, RI_MACROFILES, NULL, &ri->macrofiles);
    array(p, ctx, RI_IGNORE, NULL, &ri->ignores);
    array(p, ctx, RI_SECURITY_PATH_PREFIX, NULL, &ri->security_path_prefix);
    array(p, ctx, RI_BADWORDS, NULL, &ri->badwords);
    strget(p, ctx, RI_METADATA, RI_VENDOR, &ri->vendor);
    array(p, ctx, RI_METADATA, RI_BUILDHOST_SUBDOMAIN, &ri->buildhost_subdomain);

#ifdef _HAVE_MODULARITYLABEL
    s = p->getstr(ctx, RI_MODULARITY, RI_STATIC_CONTEXT);

    if (s != NULL) {
        if (!strcasecmp(s, RI_REQUIRED)) {
            ri->modularity_static_context = STATIC_CONTEXT_REQUIRED;
        } else if (!strcasecmp(s, RI_FORBIDDEN)) {
            ri->modularity_static_context = STATIC_CONTEXT_FORBIDDEN;
        } else if (!strcasecmp(s, RI_RECOMMEND)) {
            ri->modularity_static_context = STATIC_CONTEXT_RECOMMEND;
        } else {
            ri->modularity_static_context = STATIC_CONTEXT_NULL;
            warnx(_("*** unknown modularity static context settiongs '%s'"), s);
        }

        free(s);
    }

    tabledict(p, ctx, RI_MODULARITY, RI_RELEASE_REGEXP, &ri->modularity_release, false, false);
#endif

    ADD_INCL_EXCL(elf);
    add_ignores(ri, p, ctx, RI_ELF);
    array(p, ctx, RI_EMPTYRPM, RI_EXPECTED_EMPTY, &ri->expected_empty_rpms);
    ADD_INCL_EXCL(manpage);
    add_ignores(ri, p, ctx, RI_MANPAGE);
    ADD_INCL_EXCL(xml);
    add_ignores(ri, p, ctx, RI_XML);

    strget(p, ctx, RI_DESKTOP, RI_DESKTOP_ENTRY_FILES_DIR, &ri->desktop_entry_files_dir);

    /* handle the desktop skips as two arrays and then merge them in to desktop_skips */
    array(p, ctx, RI_DESKTOP, RI_DESKTOP_SKIP_EXEC_CHECK, &slist);
    process_desktop_skips(&ri->desktop_skips, slist, SKIP_EXEC);
    list_free(slist, free);
    slist = NULL;

    array(p, ctx, RI_DESKTOP, RI_DESKTOP_SKIP_ICON_CHECK, &slist);
    process_desktop_skips(&ri->desktop_skips, slist, SKIP_ICON);
    list_free(slist, free);
    slist = NULL;

    add_ignores(ri, p, ctx, RI_DESKTOP);
    array(p, ctx, RI_CHANGEDFILES, RI_HEADER_FILE_EXTENSIONS, &ri->header_file_extensions);
    add_ignores(ri, p, ctx, RI_CHANGEDFILES);
    array(p, ctx, RI_ADDEDFILES, RI_FORBIDDEN_PATH_PREFIXES, &ri->forbidden_path_prefixes);
    array(p, ctx, RI_ADDEDFILES, RI_FORBIDDEN_PATH_SUFFIXES, &ri->forbidden_path_suffixes);
    array(p, ctx, RI_ADDEDFILES, RI_FORBIDDEN_DIRECTORIES, &ri->forbidden_directories);
    add_ignores(ri, p, ctx, RI_ADDEDFILES);
    add_ignores(ri, p, ctx, RI_MOVEDFILES);
    add_ignores(ri, p, ctx, RI_REMOVEDFILES);
    array(p, ctx, RI_OWNERSHIP, RI_BIN_PATHS, &ri->bin_paths);
    strget(p, ctx, RI_OWNERSHIP, RI_BIN_OWNER, &ri->bin_owner);
    strget(p, ctx, RI_OWNERSHIP, RI_BIN_GROUP, &ri->bin_group);
    array(p, ctx, RI_OWNERSHIP, RI_FORBIDDEN_OWNERS, &ri->forbidden_owners);
    array(p, ctx, RI_OWNERSHIP, RI_FORBIDDEN_GROUPS, &ri->forbidden_groups);
    add_ignores(ri, p, ctx, RI_OWNERSHIP);
    array(p, ctx, RI_SHELLSYNTAX, RI_SHELLS, &ri->shells);
    add_ignores(ri, p, ctx, RI_SHELLSYNTAX);

    s = p->getstr(ctx, RI_FILESIZE, RI_SIZE_THRESHOLD);

    if (s != NULL) {
        if (!strcasecmp(s, RI_INFO) || !strcasecmp(s, RI_INFO_ONLY0) || !strcasecmp(s, RI_INFO_ONLY1)) {
            ri->size_threshold = -1;
        } else {
            errno = 0;
            ri->size_threshold = strtol(s, 0, 10);

            if ((ri->size_threshold == LONG_MIN || ri->size_threshold == LONG_MAX) && errno == ERANGE) {
                warn("*** strtol");
                ri->size_threshold = 0;
            }
        }

        free(s);
    }

    add_ignores(ri, p, ctx, RI_FILESIZE);
    array(p, ctx, RI_LTO, RI_LTO_SYMBOL_NAME_PREFIXES, &ri->lto_symbol_name_prefixes);
    add_ignores(ri, p, ctx, RI_LTO);

    s = p->getstr(ctx, RI_SPECNAME, RI_MATCH);

    if (s != NULL) {
        if (!strcasecmp(s, RI_SUFFIX)) {
            ri->specmatch = MATCH_SUFFIX;
        } else if (!strcasecmp(s, RI_PREFIX)) {
            ri->specmatch = MATCH_PREFIX;
        } else if (!strcasecmp(s, RI_FULL)) {
            ri->specmatch = MATCH_FULL;
        } else {
            warnx(_("*** unknown specname match setting '%s', defaulting to 'full'"), s);
            ri->specmatch = MATCH_FULL;
        }

        free(s);
    }

    s = p->getstr(ctx, RI_SPECNAME, RI_PRIMARY);

    if (s != NULL) {
        if (!strcasecmp(s, RI_FILENAME)) {
            ri->specprimary = PRIMARY_FILENAME;
        } else if (!strcasecmp(s, RI_NAME)) {
            ri->specprimary = PRIMARY_NAME;
        } else {
            warnx(_("*** unknown specname primary setting '%s', defaulting to 'name'"), s);
            ri->specprimary = PRIMARY_NAME;
        }

        free(s);
    }

    add_ignores(ri, p, ctx, RI_SPECNAME);

    s = p->getstr(ctx, RI_ANNOCHECK, RI_FAILURE_SEVERITY);

    if (s != NULL) {
        ri->annocheck_failure_severity = getseverity(s, RESULT_NULL);

        if (ri->annocheck_failure_severity == RESULT_NULL) {
            warnx(_("*** invalid annocheck failure_reporting_level: %s, defaulting to %s"), s, strseverity(RESULT_VERIFY));
            ri->annocheck_failure_severity = RESULT_VERIFY;
        }

        free(s);
    }

    strget(p, ctx, RI_ANNOCHECK, RI_PROFILE, &ri->annocheck_profile);
    tabledict(p, ctx, RI_ANNOCHECK, RI_JOBS, &ri->annocheck, false, false);
    tabledict(p, ctx, RI_ANNOCHECK, RI_EXTRA_OPTS, &ri->annocheck, true, false);
    add_ignores(ri, p, ctx, RI_ANNOCHECK);

    /* Backward compatibility for annocheck jobs at top-level. */
    if (ri->annocheck == NULL) {
        p->strdict_foreach(ctx, RI_ANNOCHECK, NULL, annocheck_cb, &annocheck_cb_data);
    }

    tabledict(p, ctx, RI_JAVABYTECODE, NULL, &ri->jvm, false, true);
    add_ignores(ri, p, ctx, RI_JAVABYTECODE);
    tabledict(p, ctx, RI_PATHMIGRATION, RI_MIGRATED_PATHS, &ri->pathmigration, false, false);
    array(p, ctx, RI_PATHMIGRATION, RI_EXCLUDED_PATHS, &ri->pathmigration_excluded_paths);
    add_ignores(ri, p, ctx, RI_PATHMIGRATION);
    add_ignores(ri, p, ctx, RI_POLITICS);
    array(p, ctx, RI_FILES, RI_FORBIDDEN_PATHS, &ri->forbidden_paths);
    add_ignores(ri, p, ctx, RI_FILES);
    strget(p, ctx, RI_ABIDIFF, RI_SUPPRESSION_FILE, &ri->abidiff_suppression_file);
    strget(p, ctx, RI_ABIDIFF, RI_DEBUGINFO_PATH, &ri->abidiff_debuginfo_path);
    strget(p, ctx, RI_ABIDIFF, RI_EXTRA_ARGS, &ri->abidiff_extra_args);

    s = p->getstr(ctx, RI_ABIDIFF, RI_SECURITY_LEVEL_THRESHOLD);

    if (s != NULL) {
        errno = 0;
        ri->abi_security_threshold = strtol(s, 0, 10);

        if ((ri->abi_security_threshold == LONG_MIN ||
             ri->abi_security_threshold == LONG_MAX) && errno == ERANGE) {
            warn("*** strtol");
            ri->abi_security_threshold = DEFAULT_ABI_SECURITY_THRESHOLD;
        }

        free(s);
    }

    add_ignores(ri, p, ctx, RI_ABIDIFF);
    strget(p, ctx, RI_KMIDIFF, RI_SUPPRESSION_FILE, &ri->kmidiff_suppression_file);
    strget(p, ctx, RI_KMIDIFF, RI_DEBUGINFO_PATH, &ri->kmidiff_debuginfo_path);
    strget(p, ctx, RI_KMIDIFF, RI_EXTRA_ARGS, &ri->kmidiff_extra_args);
    array(p, ctx, RI_KMIDIFF, RI_KERNEL_FILENAMES, &ri->kernel_filenames);
    strget(p, ctx, RI_KMIDIFF, RI_KABI_DIR, &ri->kabi_dir);
    strget(p, ctx, RI_KMIDIFF, RI_KABI_FILENAME, &ri->kabi_filename);
    add_ignores(ri, p, ctx, RI_KMIDIFF);
    array(p, ctx, RI_PATCHES, RI_AUTOMACROS, &ri->automacros);
    array(p, ctx, RI_PATCHES, RI_IGNORE_LIST, &ri->patch_ignore_list);
    array(p, ctx, RI_BADFUNCS, RI_FORBIDDEN, &ri->bad_functions);

    /* ri->bad_functions_allowed is a string_list_map_t, not string_map_t. */
    if (p->strdict_foreach(ctx, RI_BADFUNCS, RI_ALLOWED, badfuncs_allowed_cb, &ri->bad_functions_allowed)) {
        warnx(_("*** malformed badfuncs->allowed section"));
    }

    /* Backward compatibility for badfuncs prior to "forbidden". */
    if (ri->bad_functions == NULL && ri->bad_functions_allowed == NULL) {
        array(p, ctx, RI_BADFUNCS, NULL, &ri->bad_functions);
    }

    add_ignores(ri, p, ctx, RI_BADFUNCS);
    array(p, ctx, RI_RUNPATH, RI_ALLOWED_PATHS, &ri->runpath_allowed_paths);
    array(p, ctx, RI_RUNPATH, RI_ALLOWED_ORIGIN_PATHS, &ri->runpath_allowed_origin_paths);
    array(p, ctx, RI_RUNPATH, RI_ORIGIN_PREFIX_TRIM, &ri->runpath_origin_prefix_trim);
    add_ignores(ri, p, ctx, RI_RUNPATH);
    add_ignores(ri, p, ctx, RI_TYPES);

    if (p->havesection(ctx, RI_UNICODE) && local) {
        warnx(_("*** ignoring 'unicode' section in %s; only allowed in system-wide configuration"), filename);
    } else {
        s = p->getstr(ctx, RI_UNICODE, RI_EXCLUDE);

        if (s != NULL && add_regex(s, &ri->unicode_exclude) != 0) {
            warnx(_("*** error reading unicode exclude regular expression: %s"), s);
        }

        free(s);

        array(p, ctx, RI_UNICODE, RI_EXCLUDED_MIME_TYPES, &ri->unicode_excluded_mime_types);
        array(p, ctx, RI_UNICODE, RI_FORBIDDEN_CODEPOINTS, &ri->unicode_forbidden_codepoints);
        add_ignores(ri, p, ctx, RI_UNICODE);
    }

    if (p->strdict_foreach(ctx, RI_RPMDEPS, RI_IGNORE, rpmdeps_cb, &ri->deprules_ignore)) {
        warnx(_("*** malformed rpmdeps->ignore section; skipping"));
    }

    add_ignores(ri, p, ctx, RI_VIRUS);
    add_ignores(ri, p, ctx, RI_CAPABILITIES);
    add_ignores(ri, p, ctx, RI_CONFIG);
    add_ignores(ri, p, ctx, RI_DOC);
    add_ignores(ri, p, ctx, RI_KMOD);
    add_ignores(ri, p, ctx, RI_PERMISSIONS);
    add_ignores(ri, p, ctx, RI_SYMLINKS);
    add_ignores(ri, p, ctx, RI_UPSTREAM);
    add_ignores(ri, p, ctx, RI_DEBUGINFO);
    strget(p, ctx, RI_DEBUGINFO, RI_DEBUGINFO_SECTIONS, &ri->debuginfo_sections);

    array(p, ctx, RI_UDEVRULES, RI_UDEV_RULES_DIRS, &ri->udev_rules_dirs);
    add_ignores(ri, p, ctx, RI_UDEVRULES);

    p->fini(ctx);
    return;
}

/*
 * Given a directory and a filename with an extension, try to look for
 * it under all support filename extensions.  If found, return the
 * full path to the config file to the caller.  The caller is
 * responsible for freeing this string.  NULL is return if no config
 * file is found.
 */
static char *find_cfgfile(const char *dir, const char *name)
{
    int i = 0;
    char *tmp = NULL;
    char *filename = NULL;

    assert(dir != NULL);
    assert(name != NULL);

    for (i = 0; CFG_FILENAME_EXTENSIONS[i] != NULL; i++) {
        xasprintf(&tmp, "%s/%s.%s", dir, name, CFG_FILENAME_EXTENSIONS[i]);
        filename = realpath(tmp, NULL);

        if (filename && !access(filename, F_OK|R_OK)) {
            free(tmp);
            return filename;
        }

        free(tmp);
        free(filename);
    }

    return NULL;
}

/*
 * Initialize the fileinfo list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_fileinfo(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;
    char *token = NULL;
    char *fnpart = NULL;
    fileinfo_field_t field = MODE;
    fileinfo_entry_t *fientry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->fileinfo) {
        return true;
    }

    /* the actual fileinfo file */
    if (ri->fileinfo_filename == NULL) {
        xasprintf(&ri->fileinfo_filename, "%s/%s/%s", ri->vendor_data_dir, FILEINFO_DIR, ri->product_release);
        assert(ri->fileinfo_filename != NULL);
    }

    contents = read_file(ri->fileinfo_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->fileinfo = (fileinfo_t *) calloc(1, sizeof(*(ri->fileinfo)));
    assert(ri->fileinfo != NULL);
    TAILQ_INIT(ri->fileinfo);

    /* add all the entries to the fileinfo list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* initialize a new list entry */
        fientry = (fileinfo_entry_t *) calloc(1, sizeof(*fientry));
        assert(fientry != NULL);

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* copy the field in to the correct struct member */
            if (field == MODE) {
                fientry->mode = parse_mode(token);
            } else if (field == OWNER) {
                fientry->owner = strdup(token);
            } else if (field == GROUP) {
                fientry->group = strdup(token);
            } else if (field == FILENAME) {
                fnpart = token;

                /* trim leading slashes since we compare to localpath later */
                while (*token != '/' && *token != '\0') {
                    token++;
                }

                if (*token == '\0') {
                    /* this is an invalid entry in the fileinfo list */
                    warnx(_("*** Invalid filename in the fileinfo list: %s"), fnpart);
                    warnx(_("*** From this invalid line:"));
                    warnx(_("***     %s"), line);

                    free(fientry->owner);
                    free(fientry->group);
                    free(fientry);
                    entry = NULL;
                } else {
                    fientry->filename = strdup(token);
                }

                break;     /* nothing should come after this field */
            }

            field++;
        }

        /* add the entry */
        if (fientry != NULL) {
            TAILQ_INSERT_TAIL(ri->fileinfo, fientry, items);
        }

        /* clean up */
        field = MODE;
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the caps list for the given product release.  If the
 * file cannot be found, return false.
 */
#ifdef _WITH_LIBCAP
bool init_caps(struct rpminspect *ri)
{
    char *line = NULL;
    char *token = NULL;
    const char *delim = NULL;
    char *end = NULL;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    caps_field_t field = PACKAGE;
    caps_filelist_t *files = NULL;
    caps_entry_t *centry = NULL;
    caps_filelist_entry_t *filelist_entry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->caps) {
        return true;
    }

    /* the actual caps list file */
    if (ri->caps_filename == NULL) {
        xasprintf(&ri->caps_filename, "%s/%s/%s", ri->vendor_data_dir, CAPABILITIES_DIR, ri->product_release);
        assert(ri->caps_filename != NULL);
    }

    contents = read_file(ri->caps_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->caps = (caps_t *) calloc(1, sizeof(*(ri->caps)));
    assert(ri->caps != NULL);
    TAILQ_INIT(ri->caps);

    /* add all the entries to the caps list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /*
         * the first two fields are space delimited, but then take the
         * 3rd field to the end of the line
         */
        delim = " \t";

        /* read the fields */
        while ((token = strsep(&line, delim)) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* take action on each field */
            if (field == PACKAGE) {
                /* this package may exist in the list */
                TAILQ_FOREACH(centry, ri->caps, items) {
                    if (!strcmp(centry->pkg, token)) {
                        files = centry->files;
                        break;
                    }
                }

                if (TAILQ_EMPTY(ri->caps) || files == NULL) {
                    /* new package for the list */
                    centry = (caps_entry_t *) calloc(1, sizeof(*centry));
                    assert(centry != NULL);
                    centry->pkg = strdup(token);
                    centry->files = (caps_filelist_t *) calloc(1, sizeof(*centry->files));
                    assert(centry->files != NULL);
                    TAILQ_INIT(centry->files);
                    TAILQ_INSERT_TAIL(ri->caps, centry, items);
                    files = centry->files;
                }

                filelist_entry = (caps_filelist_entry_t *) calloc(1, sizeof(*filelist_entry));
                assert(filelist_entry != NULL);
                field = FILEPATH;
            } else if (field == FILEPATH && filelist_entry->path == NULL) {
                filelist_entry->path = strdup(token);
                field = CAPABILITIES;

                /* reset to take the remaining part of the line as the 3rd field */
                delim = "\r\n";
            } else if (field == CAPABILITIES && filelist_entry->caps == NULL) {
                /* trim leading whitespace */
                while (isspace(*token) && *token != '\0') {
                    token++;
                }

                /* trim trailing whitespace */
                end = token;
                end += strlen(token);

                while (isspace(*end) && end != token) {
                    *end = '\0';
                    end--;
                }

                filelist_entry->caps = strdup(token);
                assert(filelist_entry->caps != NULL);
            } else {
                errx(RI_PROGRAM_ERROR, _("*** unexpected token `%s' seen in %s, cannot continue"), token, ri->caps_filename);
            }
        }

        /* add the entry */
        if (filelist_entry != NULL) {
            TAILQ_INSERT_TAIL(files, filelist_entry, items);
        }

        /* clean up */
        files = NULL;
        field = PACKAGE;
    }

    list_free(contents, free);

    return true;
}
#endif

/*
 * Initialize the rebaseable list for the given product release and
 * cache it.  Return the cached list.  If the file cannot be found,
 * return false.
 */
bool init_rebaseable(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->rebaseable) {
        return true;
    }

    /* the actual rebaseable list file */
    if (ri->rebaseable_filename == NULL) {
        xasprintf(&ri->rebaseable_filename, "%s/%s/%s", ri->vendor_data_dir, REBASEABLE_DIR, ri->product_release);
        assert(ri->rebaseable_filename != NULL);
    }

    contents = read_file(ri->rebaseable_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->rebaseable = (string_list_t *) calloc(1, sizeof(*(ri->rebaseable)));
    assert(ri->rebaseable != NULL);
    TAILQ_INIT(ri->rebaseable);

    /* add all the entries to the rebaseable list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* add the entry to the actual list */
        ri->rebaseable = list_add(ri->rebaseable, line);
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the politics list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_politics(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;
    char *token = NULL;
    politics_entry_t *pentry = NULL;
    politics_field_t field = PATTERN;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->politics) {
        return true;
    }

    /* the actual politics file */
    xasprintf(&ri->politics_filename, "%s/%s/%s", ri->vendor_data_dir, POLITICS_DIR, ri->product_release);
    assert(ri->politics_filename != NULL);
    contents = read_file(ri->politics_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->politics = (politics_list_t *) calloc(1, sizeof(*(ri->politics)));
    assert(ri->politics != NULL);
    TAILQ_INIT(ri->politics);

    /* add all the entries to the politics list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* initialize a new list entry */
        pentry = (politics_entry_t *) calloc(1, sizeof(*pentry));
        assert(pentry != NULL);

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            /* copy the field in to the correct struct member */
            if (field == PATTERN) {
                pentry->pattern = strdup(token);
            } else if (field == DIGEST) {
                pentry->digest = strdup(token);
            } else if (field == PERMISSION) {
                /*
                 * The permission column is either "allow" or "deny".
                 * The general assumption is that entries in this file
                 * will be for denial, so default to that and only do
                 * a case-insensitive search for 'allow' to allow the
                 * file. */
                if (!strcasecmp(token, "allow")) {
                    pentry->allowed = true;
                } else {
                    pentry->allowed = false;
                }

                break;     /* nothing should come after this field */
            }

            field++;
        }

        /* add the entry */
        if (pentry != NULL) {
            TAILQ_INSERT_TAIL(ri->politics, pentry, items);
        }

        /* clean up */
        field = PATTERN;
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the security list for the given product release.  If the
 * file cannot be found, return false.
 */
bool init_security(struct rpminspect *ri)
{
    int pos = 0;
    char *line = NULL;
    char *token = NULL;
    char *path = NULL;
    char *pkg = NULL;
    char *ver = NULL;
    char *rel = NULL;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    string_list_t *rules = NULL;
    string_entry_t *rentry = NULL;
    string_list_t *kv = NULL;
    string_entry_t *key = NULL;
    string_entry_t *value = NULL;
    secrule_type_t stype = SECRULE_NULL;
    severity_t severity = RESULT_NULL;
    security_entry_t *sentry = NULL;
    secrule_t *rule_entry = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->security_initialized) {
        return true;
    } else {
        ri->security = (security_list_t *) calloc(1, sizeof(*ri->security));
        assert(ri->security != NULL);
        TAILQ_INIT(ri->security);
        ri->security_initialized = true;
    }

    /* the actual security file */
    xasprintf(&ri->security_filename, "%s/%s/%s", ri->vendor_data_dir, SECURITY_DIR, ri->product_release);
    assert(ri->security_filename != NULL);
    contents = read_file(ri->security_filename);

    if (contents == NULL) {
        return false;
    }

    /* add all the entries to the caps list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* read the fields */
        while ((token = strsep(&line, " \t")) != NULL) {
            /* might be lots of space between fields */
            if (*token == '\0') {
                continue;
            }

            if (pos == 0) {
                path = strdup(token);
                assert(path != NULL);
            } else if (pos == 1) {
                pkg = strdup(token);
                assert(pkg != NULL);
            } else if (pos == 2) {
                ver = strdup(token);
                assert(ver != NULL);
            } else if (pos == 3) {
                rel = strdup(token);
                assert(rel != NULL);
            } else if (pos == 4) {
                rules = strsplit(token, ",");
            }

            pos++;
        }

        /* add the entry */
        if (path && pkg && ver && rel && rules) {
            /* allocate a new entry */
            sentry = (security_entry_t *) calloc(1, sizeof(*sentry));
            assert(sentry != NULL);

            /* the main values of a rule */
            sentry->path = strdup(path);
            assert(sentry->path != NULL);

            sentry->pkg = strdup(pkg);
            assert(sentry->pkg != NULL);

            sentry->ver = strdup(ver);
            assert(sentry->ver != NULL);

            sentry->rel = strdup(rel);
            assert(sentry->rel != NULL);

            /* the list of individual rules */
            TAILQ_FOREACH(rentry, rules, items) {
                /* split the RULE=ACTION setting */
                kv = strsplit(rentry->data, "=");

                /* we must have a rule and action */
                if (list_len(kv) != 2) {
                    warnx(_("*** invalid security rule: %s"), rentry->data);
                    list_free(kv, free);
                    continue;
                }

                key = TAILQ_FIRST(kv);
                value = TAILQ_LAST(kv, string_entry_s);

                /* find the rule */
                stype = get_secrule_type(key->data);

                if (stype == SECRULE_NULL) {
                    warnx(_("*** unknown security rule: %s"), key->data);
                    list_free(kv, free);
                    continue;
                }

                /* find the action */
                severity = get_secrule_severity(value->data);

                if (severity == RESULT_NULL) {
                    warnx(_("*** unknown security action: %s"), value->data);
                    list_free(kv, free);
                    continue;
                }

                /* add or replace the rule action */
                HASH_FIND_INT(sentry->rules, &stype, rule_entry);

                if (rule_entry == NULL) {
                    rule_entry = (secrule_t *) calloc(1, sizeof(*rule_entry));
                    assert(rule_entry != NULL);
                    rule_entry->type = stype;
                    rule_entry->severity = severity;
                    HASH_ADD_INT(sentry->rules, type, rule_entry);
                } else {
                    /* rule already exists, just replace the action */
                    rule_entry->severity = severity;
                }

                /* clean up */
                list_free(kv, free);
            }

            /* add new entry to the security rules list */
            TAILQ_INSERT_TAIL(ri->security, sentry, items);
        } else {
            warnx(_("*** malformed security line: %s"), line);
        }

        /* clean up */
        free(path);
        free(pkg);
        free(ver);
        free(rel);
        list_free(rules, free);
        pos = 0;
    }

    list_free(contents, free);

    return true;
}

/*
 * Initialize the icons list for the given product release and cache
 * it.  Return the cached list.  If the file cannot be found, return
 * false.
 */
bool init_icons(struct rpminspect *ri)
{
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *line = NULL;

    assert(ri != NULL);
    assert(ri->vendor_data_dir != NULL);
    assert(ri->product_release != NULL);

    /* already initialized */
    if (ri->icons) {
        return true;
    }

    /* the actual icons list file */
    xasprintf(&ri->icons_filename, "%s/%s/%s", ri->vendor_data_dir, ICONS_DIR, ri->product_release);
    assert(ri->icons_filename != NULL);
    contents = read_file(ri->icons_filename);

    if (contents == NULL) {
        return false;
    }

    /* initialize the list */
    ri->icons = (string_list_t *) calloc(1, sizeof(*(ri->icons)));
    assert(ri->icons != NULL);
    TAILQ_INIT(ri->icons);

    /* add all the entries to the icons list */
    TAILQ_FOREACH(entry, contents, items) {
        if (entry->data == NULL) {
            continue;
        }

        /* trim line ending characters */
        line = entry->data;
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (*line == '#' || *line == '\n' || *line == '\r' || !strcmp(line, "")) {
            continue;
        }

        /* add the entry to the actual list */
        ri->icons = list_add(ri->icons, line);
    }

    list_free(contents, free);

    return true;
}

/*
 * Memory initialization function for struct rpminspect.
 */
struct rpminspect *calloc_rpminspect(struct rpminspect *ri)
{
    if (ri != NULL) {
        return ri;
    }

    /* Only initialize if we were given NULL for ri */
    ri = (struct rpminspect *) calloc(1, sizeof(*ri));
    assert(ri != NULL);

    /* Initialize the struct before reading files */
    ri->workdir = strdup(DEFAULT_WORKDIR);
    ri->vendor_data_dir = strdup(VENDOR_DATA_DIR);
    ri->favor_release = FAVOR_NEWEST;
    ri->tests = ~0;
    ri->desktop_entry_files_dir = strdup(DESKTOP_ENTRY_FILES_DIR);
    ri->bin_paths = list_from_array(BIN_PATHS);
    ri->bin_owner = strdup(BIN_OWNER);
    ri->bin_group = strdup(BIN_GROUP);
    ri->shells = list_from_array(SHELLS);
    ri->specmatch = MATCH_FULL;
    ri->specprimary = PRIMARY_NAME;
    ri->abidiff_suppression_file = strdup(ABI_SUPPRESSION_FILE);
    ri->abidiff_debuginfo_path = strdup(DEBUG_PATH);
    ri->abi_security_threshold = DEFAULT_ABI_SECURITY_THRESHOLD;
    ri->kmidiff_suppression_file = strdup(ABI_SUPPRESSION_FILE);
    ri->kmidiff_debuginfo_path = strdup(DEBUG_PATH);
    ri->annocheck_failure_severity = RESULT_VERIFY;
    ri->size_threshold = -1;
    xasprintf(&ri->debuginfo_sections, "%s %s", ELF_SYMTAB, ELF_DEBUG_INFO);
    ri->udev_rules_dirs = list_from_array(UDEV_RULES_DIRS);

    /* Initialize commands */
    ri->commands.msgunfmt = strdup(MSGUNFMT_CMD);
    ri->commands.desktop_file_validate = strdup(DESKTOP_FILE_VALIDATE_CMD);
    ri->commands.abidiff = strdup(ABIDIFF_CMD);
    ri->commands.kmidiff = strdup(KMIDIFF_CMD);
#ifdef _WITH_ANNOCHECK
    ri->commands.annocheck = strdup(ANNOCHECK_CMD);
#endif
    ri->commands.udevadm = strdup(UDEVADM_CMD);

    /* Store full paths to all config files read */
    if (ri->cfgfiles == NULL) {
        ri->cfgfiles = (string_list_t *) calloc(1, sizeof(*ri->cfgfiles));
        assert(ri->cfgfiles != NULL);
        TAILQ_INIT(ri->cfgfiles);
    }

    return ri;
}

/*
 * Initialize a struct rpminspect.  Called by applications using
 * librpminspect before they began calling library functions.  If ri
 * passed in is NULL, the function will allocate and initialize a new
 * struct rpminspect.  The caller is responsible for freeing the
 * struct rpminspect.  Return 0 on success, -1 on failure.
 */
struct rpminspect *init_rpminspect(struct rpminspect *ri, const char *cfgfile, const char *profile)
{
    int i = 0;
    char cwd[PATH_MAX + 1];
    char *tmp = NULL;
    char *cf = NULL;
    char *bn = NULL;
    const char *kernelnames[] = KERNEL_FILENAMES;
    string_entry_t *cfg = NULL;

    /* initialize the default remedy strings */
    if (!remedy_initialized) {
        init_remedy_strings();
        remedy_initialized = true;
    }

    if (ri == NULL) {
        ri = calloc_rpminspect(ri);
    }

    /* Read in the config file */
    if (cfgfile) {
        cfg = (string_entry_t *) calloc(1, sizeof(*cfg));
        assert(cfg != NULL);
        cfg->data = realpath(cfgfile, NULL);

        /* In case we have a missing configuration file */
        if ((cfg->data == NULL) || (access(cfg->data, F_OK|R_OK) == -1)) {
            free(cfg->data);
            free(cfg);
            errx(RI_PROGRAM_ERROR, _("*** missing configuration file `%s'"), cfgfile);
        }

        /* Read the main configuration file to get things started */
        read_cfgfile(ri, cfg->data, false);

        /* Store this config file as one we read in */
        if (!list_contains(ri->cfgfiles, cfg->data)) {
            TAILQ_INSERT_TAIL(ri->cfgfiles, cfg, items);
        } else {
            free(cfg->data);
            free(cfg);
        }
    }

    /* Look for and autoload a product release profile if we have one */
    if (ri->product_release) {
        xasprintf(&tmp, "%s/%s", ri->profiledir, PRODUCT_RELEASE_CFGFILE_SUBDIR);
        assert(tmp != NULL);
        cf = find_cfgfile(tmp, ri->product_release);
        free(tmp);

        if (cf) {
            read_cfgfile(ri, cf, false);
            free(cf);
        }
    }

    /* If a profile is specified, read an overlay config file */
    if (profile) {
        cf = find_cfgfile(ri->profiledir, profile);

        if (cf) {
            read_cfgfile(ri, cf, false);
            free(cf);
        } else {
            errx(RI_MISSING_PROFILE, _("*** unable to find profile '%s'"), profile);
        }
    }

    /* get current dir */
    memset(cwd, '\0', sizeof(cwd));

    if (getcwd(cwd, PATH_MAX) == NULL) {
        err(RI_PROGRAM_ERROR, "*** getcwd");
    }

    /* optional config file from current directory */
    cf = find_cfgfile(cwd, COMMAND_NAME);

    if (cf) {
        /* save a copy of the local rpminspect config file for diagnostics */
        bn = basename(cf);
        assert(bn != NULL);
        ri->localcfg = strdup(bn);
        assert(ri->localcfg != NULL);
        ri->locallines = read_file(cf);

        /* read the file only if it's not empty */
        if (ri->locallines) {
            read_cfgfile(ri, cf, true);
        }

        free(cf);
    }

    /* Initialize some lists if we did not get any config file data */
    if (ri->kernel_filenames == NULL) {
        for(i = 0; kernelnames[i] != NULL; i++) {
            ri->kernel_filenames = list_add(ri->kernel_filenames, kernelnames[i]);
        }
    }

    /* the rest of the members are used at runtime */
    ri->threshold = RESULT_VERIFY;
    ri->worst_result = RESULT_OK;
    ri->suppress = RESULT_NULL;

    if (ri->peers == NULL) {
        ri->peers = init_peers();
    }

    return ri;
}
