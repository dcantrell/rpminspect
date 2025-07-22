/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <err.h>
#include <curl/curl.h>
#include "rpminspect.h"

/* Globals used by programs linking with the library */
volatile sig_atomic_t terminal_resized = 0;

/* Local global variables */
static size_t total_width = 0;
static size_t half_width = 0;
static size_t bar_width = 0;
static curl_off_t progress_displayed = 0;
static size_t progress_msg_len = 0;

/*
 * Called by either the download helper or the progress bar callback
 * on SIGWINCH.  Sets the line up for the progress bar.  NULL input
 * means reposition an in-progress progress bar.
 */
static void setup_progress_bar(const char *src)
{
    char *archive = NULL;
    char *vmsg = NULL;

    /* terminal width and progress bar width */
    if (total_width == 0) {
        total_width = tty_width();
        half_width = round(total_width / 2);
        bar_width = half_width - 2;       /* account for '[' and ']' */
    }

    progress_displayed = 0;

    /* generate the verbose message string */
    if (src != NULL) {
        /* the basename of the source URL, which is the file name */
        archive = rindex(src, PATH_SEP) + 1;
        assert(archive != NULL);

        /* we need to shorten the package basename if too wide */
        if ((strlen(archive) + 5) > bar_width) {
            archive = strshorten(archive, bar_width - 5);
            assert(archive != NULL);
            xasprintf(&vmsg, "=> %s ", archive);
            assert(vmsg != NULL);
            free(archive);
        } else {
            xasprintf(&vmsg, "=> %s ", archive);
        }

        progress_msg_len = strlen(vmsg);
    }

    /* display the progress bar and position the cursor */
    /*
     * Because I am very likely to forget these escape sequences,
     * here's a brief explanation.  These are originate from the VT100
     * and then became ANSI escape sequences, so you can search for
     * both terms online and probably find the information you want.
     * Here are direction movement ones:
     *
     *    Esc[nA      Move the cursor up n lines
     *    Esc[nB      Move the cursor down n lines
     *    Esc[nC      Move the cursor right n columns
     *    Esc[nD      Move the cursor left n columns
     *
     * Within printf(3), we can't say "Esc" for escape, so we spell
     * that as \033 to use the octal code (see ascii(7) for more
     * information).  The values for n are computed and then are
     * substituted in to the format string making this extremely
     * difficult to read.  Good luck decoding.
     */
    if (vmsg != NULL) {
        /* new progress bar */
        printf("%s\033[%zuC[\033[%zuC]\033[%zuD", vmsg, bar_width - progress_msg_len, bar_width, bar_width + 1);
        free(vmsg);
    } else {
        /* reposition due to terminal resize */
        printf("\033[%" CURL_FORMAT_CURL_OFF_T "D[\033[%zuC]\033[%zuD", progress_msg_len + progress_displayed, bar_width, bar_width + 1);
    }

    fflush(stdout);
    return;
}

/*
 * libcurl progress callback function
 * The caller needs to set up the terminal for displaying the progress
 * bar.  The total width needs to be in the global total_width
 * variable.  And the caller needs to position the cursor so this
 * callback can start printing hash marks.
 */
static int download_progress(__attribute__((unused)) void *p, curl_off_t dltotal, curl_off_t dlnow, __attribute__((unused)) curl_off_t ultotal, __attribute__((unused)) curl_off_t ulnow)
{
    curl_off_t percentage = 0;
    curl_off_t hashes = 0;
    curl_off_t i = 0;

    /* compute percentage downloaded */
    if (dltotal > 0) {
        percentage = (100L*dlnow + dltotal/2) / dltotal;
    }

    /*
     * adjust the progress bar if the terminal has resized
     */
    if (terminal_resized == 1) {
        /* reposition the progress bar */
        total_width = 0;
        setup_progress_bar(NULL);

        /* reset for the next change */
        terminal_resized = 0;
    }

    /*
     * now determine how many hash marks represent that percentage in
     * our progress bar
     */
    hashes = (percentage / 100.0) * bar_width;

    /*
     * display any new hash marks to indicate progress and update our
     * displayed total
     */
    if (hashes != progress_displayed) {
        for (i = 0; i < (hashes - progress_displayed); i++) {
            printf("#");
            fflush(stdout);
        }

        progress_displayed = hashes;
    }

    return 0;
}

#if LIBCURL_VERSION_NUM < 0x072000
static int legacy_download_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
    return download_progress(p, (curl_off_t) dltotal, (curl_off_t) dlnow, (curl_off_t) ultotal, (curl_off_t) ulnow);
}
#endif

/*
 * Download helper for libcurl
 */
void curl_get_file(const bool verbose, const char *src, const char *dst)
{
    FILE *fp = NULL;
    char *archive = NULL;
    CURL *c = NULL;
    CURLcode cc;

    assert(src != NULL);
    assert(dst != NULL);

    DEBUG_PRINT("src=|%s|\ndst=|%s|\n", src, dst);

    /* initialize curl */
    c = curl_easy_init();

    if (!c) {
        warn("*** curl_easy_init");
        return;
    }

    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_MAXREDIRS, 10L);

    if (verbose) {
        if (isatty(STDOUT_FILENO) == 1) {
#if LIBCURL_VERSION_NUM >= 0x072000
            curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, download_progress);
#else
            curl_easy_setopt(c, CURLOPT_PROGRESSFUNCTION, legacy_download_progress);
#endif
            curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
            setup_progress_bar(src);
        } else {
            archive = rindex(src, PATH_SEP) + 1;
            assert(archive != NULL);
            printf(">>> %s", archive);
        }
    }

    /* perform the download */
    fp = fopen(dst, "wb");

    if (fp == NULL) {
        err(RI_PROGRAM_ERROR, "*** fopen");
    }

    curl_easy_setopt(c, CURLOPT_URL, src);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(c, CURLOPT_FAILONERROR, true);
#ifdef CURLOPT_TCP_FASTOPEN /* not available on all versions of libcurl (e.g., <= 7.29) */
    curl_easy_setopt(c, CURLOPT_TCP_FASTOPEN, 1);
#endif
    cc = curl_easy_perform(c);

    if (verbose) {
        printf("\n");
        fflush(stdout);
    }

    if (fclose(fp) != 0) {
        err(RI_PROGRAM_ERROR, "*** fclose");
    }

    /* remove output file if there was a download error (e.g., 404) */
    if (cc != CURLE_OK) {
        if (unlink(dst)) {
            warn("*** unlink");
        }
    }

    curl_easy_cleanup(c);

    return;
}

curl_off_t curl_get_size(const char *src)
{
    curl_off_t r = 0;
    CURL *c = NULL;
    CURLcode cc;
#ifndef _HAVE_NEWER_CURLINFO
    double len = 0;
#endif

    assert(src != NULL);

    /* initialize curl */
    if (!(c = curl_easy_init())) {
        warn("*** curl_easy_init");
        return 0;
    }

    /* get the size */
    curl_easy_setopt(c, CURLOPT_URL, src);
    curl_easy_setopt(c, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(c, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_MAXREDIRS, 10L);
#ifdef CURLOPT_TCP_FASTOPEN /* not available on all versions of libcurl (e.g., <= 7.29) */
    curl_easy_setopt(c, CURLOPT_TCP_FASTOPEN, 1);
#endif

    cc = curl_easy_perform(c);

    if (cc != CURLE_OK) {
        warnx(_("*** unable to read %s"), src);
        return 0;
    }

#ifdef _HAVE_NEWER_CURLINFO
    curl_easy_getinfo(c, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &r);
#else
    curl_easy_getinfo(c, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &len);
    r = (curl_off_t) len;
#endif

    curl_easy_cleanup(c);
    return r;
}

bool is_remote_rpm(const char *url)
{
    CURL *c = NULL;
    CURLcode r = -1;

    assert(url != NULL);

    if (!strsuffix(url, RPM_FILENAME_EXTENSION)) {
        return false;
    }

    c = curl_easy_init();

    if (c) {
        curl_easy_setopt(c, CURLOPT_URL, url);
        curl_easy_setopt(c, CURLOPT_NOBODY, 1);
        r = curl_easy_perform(c);
        curl_easy_cleanup(c);
    }

    return (r == CURLE_OK);
}
