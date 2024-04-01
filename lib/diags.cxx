/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <assert.h>
#include <zlib.h>
#include <magic.h>
#include <clamav.h>
#include <libxml/globals.h>
#include <json.h>
#include <curl/curl.h>
#include <archive.h>
#include <yaml.h>
#include <xmlrpc-c/client.h>
#include <openssl/crypto.h>

#ifdef _WITH_LIBANNOCHECK
#include <libannocheck.h>
#endif

#include "rpminspect.h"

/* For older versions of OpenSSL */
#ifndef OPENSSL_VERSION
#define OPENSSL_VERSION 0
#endif

/*
 * Gather versions of dependent libraries and programs and add the
 * information as strings to a list and return the list.  Caller must
 * free the list.
 */
string_list_t *gather_diags(struct rpminspect *ri, const char *progname, const char *progver)
{
    string_list_t *list = NULL;
    string_list_t *details = NULL;
    char *tmp = NULL;
    char *ver = NULL;
    string_entry_t *entry = NULL;
    unsigned int major = 0;
    unsigned int minor = 0;
    unsigned int update = 0;
    int exitcode = 0;

    assert(ri != NULL);
    assert(progname != NULL);
    assert(progver != NULL);

    /* initialize a new list */
    list = (string_list_t *) calloc(1, sizeof(*list));
    assert(list != NULL);
    TAILQ_INIT(list);

    /* start by adding info about ourself */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "%s version %s", progname, progver);
    TAILQ_INSERT_TAIL(list, entry, items);

    /*
     * LIBRARIES
     * All of these dependent libraries provide some sort of version function.
     */

    /* zlib */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "zlib version %s", zlibVersion());
    TAILQ_INSERT_TAIL(list, entry, items);

#ifdef _HAVE_MAGIC_VERSION
    /* libmagic */
    /* older versions of libmagic lack this function */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libmagic version %d", magic_version());
    TAILQ_INSERT_TAIL(list, entry, items);
#endif

    /* libclamav */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libclamav version %s", cl_retver());
    TAILQ_INSERT_TAIL(list, entry, items);

    /* librpm */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "librpm version %s", RPMVERSION);
    TAILQ_INSERT_TAIL(list, entry, items);

    /* libxml */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libxml version %s", xmlParserVersion);
    TAILQ_INSERT_TAIL(list, entry, items);

    /* json-c */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "json-c version %s", json_c_version());
    TAILQ_INSERT_TAIL(list, entry, items);

    /* libcurl */
    tmp = strreplace(curl_version(), "libcurl/", NULL);     /* get detailed version info, strip lib name */
    details = strsplit(tmp, " ");
    free(tmp);

    entry = TAILQ_FIRST(details);                           /* first entry is libcurl version */
    TAILQ_REMOVE(details, entry, items);
    ver = strdup(entry->data);
    free(entry->data);
    free(entry);

    tmp = list_to_string(details, " ");                     /* rejoin remaining details */
    list_free(details, free);

    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libcurl version %s (%s)", ver, tmp);
    TAILQ_INSERT_TAIL(list, entry, items);

    free(ver);
    free(tmp);

    /* libarchive */
#ifdef _HAVE_ARCHIVE_VERSION_DETAILS
    tmp = strreplace(archive_version_details(), "libarchive ", NULL);
    details = strsplit(tmp, " ");
    free(tmp);

    entry = TAILQ_FIRST(details);
    TAILQ_REMOVE(details, entry, items);
    ver = strdup(entry->data);
    free(entry->data);
    free(entry);

    tmp = list_to_string(details, " ");
    list_free(details, free);

    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libarchive version %s (%s)", ver, tmp);
    TAILQ_INSERT_TAIL(list, entry, items);

    free(ver);
    free(tmp);
#elif _HAVE_ARCHIVE_VERSION_STRING
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "%s", archive_version_string());
    TAILQ_INSERT_TAIL(list, entry, items);
#endif

    /* libyaml */
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libyaml version %s", yaml_get_version_string());
    TAILQ_INSERT_TAIL(list, entry, items);

#ifndef NO_OPENSSL_VERSION_FUNCTION
    /* openssl */
    tmp = strreplace(OpenSSL_version(OPENSSL_VERSION), "OpenSSL ", "OpenSSL version ");
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "%s", tmp);
    TAILQ_INSERT_TAIL(list, entry, items);
    free(tmp);
#endif

    /* xmlrpc-c */
    xmlrpc_client_version(&major, &minor, &update);
    entry = (string_entry_t *) calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "xmlrpc-c version %u.%u.%u", major, minor, update);
    TAILQ_INSERT_TAIL(list, entry, items);

#ifdef _WITH_LIBANNOCHECK
    /* libannocheck */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    xasprintf(&entry->data, "libannocheck version %u", libannocheck_get_version());
    TAILQ_INSERT_TAIL(list, entry, items);
#endif

    /*
     * COMMANDS
     * External commands rpminspect runs.  We capture version info.
     */

    /* msgunfmt */
    tmp = run_cmd(&exitcode, ri->worksubdir, ri->commands.msgunfmt, "--version", NULL);

    if (exitcode == 0) {
        details = strsplit(tmp, "\n");
        free(tmp);

        if (details) {
            entry = TAILQ_FIRST(details);

            if (entry && entry->data) {
                ver = strdup(entry->data);
            } else {
                ver = NULL;
            }

            list_free(details, free);
        }

        if (ver == NULL) {
            ver = realpath(ri->commands.msgunfmt, NULL);
            assert(ver != NULL);
        }

        entry = (string_entry_t *) calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xasprintf(&entry->data, "%s", ver);
        TAILQ_INSERT_TAIL(list, entry, items);

        free(ver);
    }

#ifdef _WITH_ANNOCHECK
    /* annocheck */
    tmp = run_cmd(&exitcode, ri->worksubdir, ri->commands.annocheck, "--version", NULL);

    if (exitcode == 0) {
        details = strsplit(tmp, "\n");
        free(tmp);

        if (details) {
            entry = TAILQ_FIRST(details);

            if (entry && entry->data) {
                ver = strreplace(entry->data, ": Version ", " version ");
            } else {
                ver = NULL;
            }

            list_free(details, free);
        }

        if (ver == NULL) {
            ver = realpath(ri->commands.annocheck, NULL);
            assert(ver != NULL);
        }

        entry = (string_entry_t *) calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xasprintf(&entry->data, "%s", ver);
        TAILQ_INSERT_TAIL(list, entry, items);

        free(ver);
    }
#endif

    /* abidiff */
    tmp = run_cmd(&exitcode, ri->worksubdir, ri->commands.abidiff, "--version", NULL);

    if (exitcode == 0) {
        details = strsplit(tmp, "\n");
        free(tmp);

        if (details) {
            entry = TAILQ_FIRST(details);

            if (entry && entry->data) {
                ver = strreplace(entry->data, ": ", " version ");
            } else {
                ver = NULL;
            }

            list_free(details, free);
        }

        if (ver == NULL) {
            ver = realpath(ri->commands.abidiff, NULL);
            assert(ver != NULL);
        }

        entry = (string_entry_t *) calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xasprintf(&entry->data, "%s", ver);
        TAILQ_INSERT_TAIL(list, entry, items);

        free(ver);
    }

    /* kmidiff */
    tmp = run_cmd(&exitcode, ri->worksubdir, ri->commands.kmidiff, "--version", NULL);

    if (exitcode == 0) {
        details = strsplit(tmp, "\n");
        free(tmp);

        if (details) {
            entry = TAILQ_FIRST(details);

            if (entry && entry->data) {
                ver = strreplace(entry->data, ": ", " version ");
            } else {
                ver = NULL;
            }

            list_free(details, free);
        }

        if (ver == NULL) {
            ver = realpath(ri->commands.kmidiff, NULL);
            assert(ver != NULL);
        }

        entry = (string_entry_t *) calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xasprintf(&entry->data, "%s", ver);
        TAILQ_INSERT_TAIL(list, entry, items);

        free(ver);
    }

    /* udevadm */
    tmp = run_cmd(&exitcode, ri->worksubdir, ri->commands.udevadm, "--version", NULL);

    if (exitcode == 0) {
        details = strsplit(tmp, "\n");
        free(tmp);

        if (details) {
            entry = TAILQ_FIRST(details);

            if (entry && entry->data) {
                ver = strdup(entry->data);
            } else {
                ver = NULL;
            }

            list_free(details, free);
        }

        if (ver == NULL) {
            ver = realpath(ri->commands.udevadm, NULL);
            assert(ver != NULL);
        }

        entry = (string_entry_t *) calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xasprintf(&entry->data, "udevadm version %s", ver);
        TAILQ_INSERT_TAIL(list, entry, items);

        free(ver);
    }

    return list;
}
