/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "rpminspect.h"

/*
 * Helper function for printwrap().  Handles printing the specified word with
 * the optional indentation and a space for the next word.
 */
static int printword(const char *word, const size_t width, const unsigned int indent, const size_t lw, bool first, FILE *dest)
{
    int ret = 0;

    if (word && ((strlen(word) + lw) >= width)) {
        fprintf(dest, "\n");
        first = true;

        if (indent > 0) {
            ret = fprintf(dest, "%*c", indent, ' ');
        } else {
            ret = 0;
        }
    } else {
        ret = lw;
    }

    /* print a space between words except at the start of a line */
    ret += fprintf(dest, "%s%s", first ? "" : " ", word);

    return ret;
}

/*
 * Returns true if s starts with prefix.
 */
bool strprefix(const char *s, const char *prefix)
{
    size_t plen;

    if (s == NULL || prefix == NULL) {
        return false;
    }

    plen = strlen(prefix);

    if (plen > strlen(s)) {
        return false;
    }

    if (!strncmp(s, prefix, plen)) {
        return true;
    } else {
        return false;
    }
}

/*
 * Test if the string STR ends up with the string SUFFIX.
 * Both strings have to be NULL terminated.
 */
bool strsuffix(const char *s, const char *suffix)
{
    int sl, xl;

    if (s == NULL || suffix == NULL) {
        return false;
    }

    sl = strlen(s);
    xl = strlen(suffix);

    if ((sl >= xl) && !strcmp(s + sl - xl, suffix)) {
        return true;
    } else {
        return false;
    }
}

/*
 * Simple stdout line wrapping function.  All this does is split strings
 * across a given column width and an optional indentation.  Nothing
 * fancy.  It's so not fancy that the input string must be preformatted.
 * It also splits on single spaces assuming that's the word boundary.  So
 * don't preformat your strings with two spaces after sentences and stuff
 * like that unless you weird looking word wrapped output.
 */
int printwrap(const char *s, const size_t width, const unsigned int indent, FILE *dest)
{
    int lines = 0;
    bool first = false;
    bool begin = false;
    size_t lw = 0;
    size_t lastlw = 0;
    char *start = NULL;
    char *str = NULL;
    char *word = NULL;

    assert(s != NULL);

    /*
     * A zero width means we're not on a tty or we're on a tty we
     * don't understand.  Just output the string and call it 1 line.
     */
    if (width == 0) {
        fprintf(dest, "%s", s);
        return 1;
    }

    start = str = strdup(s);

    while (*str != '\0') {
        if (lw == 0 && !begin) {
            if (indent > 0) {
                lw += fprintf(dest, "%*c", indent, ' ');
            }

            word = str;
            first = true;
            begin = true;
        } else if (isspace(*str)) {
            *str = '\0';
            lw = printword(word, width, indent, lw, first, dest);

            if (lw < lastlw) {
                lines++;
            }

            lastlw = lw;
            str++;
            word = str;
            first = false;
            begin = false;
        } else {
            str++;
        }
    }

    /* print the last word */
    lw = printword(word, width, indent, lw, first, dest);

    if (lw < lastlw) {
        lines++;
    }

    fprintf(dest, "\n");
    free(start);
    return lines;
}

/* Check whether two strings match, ignoring instances of version-release */
bool versioned_match(const char *str1, Header h1, const char *str2, Header h2)
{
    const char *version1;
    const char *version2;
    const char *release1;
    const char *release2;
    char *vr1;
    char *vr2;
    size_t vr1_len;
    size_t vr2_len;
    const char *nextversion1;
    const char *nextversion2;
    bool is_old_version;
    bool result;

    assert(str1);
    assert(str2);
    assert(h1);
    assert(h2);

    version1 = headerGetString(h1, RPMTAG_VERSION);
    version2 = headerGetString(h2, RPMTAG_VERSION);
    release1 = headerGetString(h1, RPMTAG_RELEASE);
    release2 = headerGetString(h2, RPMTAG_RELEASE);
    xasprintf(&vr1, "%s-%s", version1, release1);
    xasprintf(&vr2, "%s-%s", version2, release2);

    vr1_len = strlen(vr1);
    vr2_len = strlen(vr2);

    while (*str1 != '\0') {
        /* Look for the next instance of a version in the first string */
        nextversion1 = strstr(str1, vr1);

        /* No version? Just compare the two strings as-is */
        if (nextversion1 == NULL) {
            result = (strcmp(str1, str2) == 0);
            goto cleanup;
        }

        /* Look for a version in the second string */
        is_old_version = false;
        nextversion2 = strstr(str2, vr2);

        if (nextversion2 == NULL) {
            /* We didn't find the new version, see if it still matches the old version */
            nextversion2 = strstr(str2, vr1);

            /* If we didn't find anything, no match */
            if (nextversion2 == NULL) {
                result = false;
                goto cleanup;
            }

            is_old_version = true;
        }

        /* Check that the portions of the string before the version match */
        if (((nextversion1 - str1) != (nextversion2 - str2)) || (strncmp(str1, str2, nextversion1 - str1) != 0)) {
            result = false;
            goto cleanup;
        }

        /* Advance the pointers past the VR strings and continue */
        str1 = nextversion1 + vr1_len;

        if (is_old_version) {
            str2 = nextversion2 + vr1_len;
        } else {
            str2 = nextversion2 + vr2_len;
        }
    }

    /* End of str1, it's a match if we're also at the end of str2 */
    result = (*str2 == '\0');

cleanup:
    free(vr1);
    free(vr2);

    return result;
}

/*
 * Given a severity_t, return a string representing the value.
 */
char *strseverity(const severity_t severity)
{
    switch (severity) {
        case RESULT_NULL:
            return _("NULL");
        case RESULT_OK:
            return _("OK");
        case RESULT_INFO:
            return _("INFO");
        case RESULT_VERIFY:
            return _("VERIFY");
        case RESULT_BAD:
            return _("BAD");
        case RESULT_SKIP:
            return _("SKIP");
        case RESULT_DIAG:
            return _("DIAGNOSTICS");
        default:
            return _("UnKnOwN");
    }

    return NULL;
}

/*
 * Given a severity string, return a severity_t matching it.
 * Or return the default.
 */
severity_t getseverity(const char *name, const severity_t default_s)
{
    severity_t s = default_s;

    if (name == NULL) {
        return s;
    }

    if (!strcasecmp(name, _("NULL"))) {
        s = RESULT_NULL;
    } else if (!strcasecmp(name, _("OK"))) {
        s = RESULT_OK;
    } else if (!strcasecmp(name, _("INFO"))) {
        s = RESULT_INFO;
    } else if (!strcasecmp(name, _("VERIFY"))) {
        s = RESULT_VERIFY;
    } else if (!strcasecmp(name, _("BAD"))) {
        s = RESULT_BAD;
    } else if (!strcasecmp(name, _("SKIP"))) {
        s = RESULT_SKIP;
    } else if (!strcasecmp(name, _("DIAGNOSTICS"))) {
        s = RESULT_DIAG;
    }

    return s;
}

/*
 * Given a type of waiver authorization, return a string representing it.
 */
char *strwaiverauth(const waiverauth_t waiverauth)
{
    switch (waiverauth) {
        case NOT_WAIVABLE:
            return _("Not Waivable");
        case WAIVABLE_BY_ANYONE:
            return _("Anyone");
        case WAIVABLE_BY_SECURITY:
            return _("Security");
        default:
            return _("UnKnOwN");
    }

    return NULL;
}

/**
 * @brief Given a string s, find the substring "find", and return a
 * newly allocated string with "find" replaced with "replace".  If
 * "replace" is NULL, this function will delete "find" from the string
 * s and return the new string.  The caller is responsible for freeing
 * the returned string.
 *
 * @param s The string to replace substrings in.
 * @param find The substring to find in s.
 * @param replace The substring to replace find with.  If NULL, find
 * is removed from s in the result.
 * @return Newly allocated string with modifications made; caller must
 * free this result.
 */
char *strreplace(const char *s, const char *find, const char *replace)
{
    const char *find_start;
    size_t find_len;
    const char *remainder;
    char *tmp;
    char *result = NULL;

    if (s == NULL) {
        return NULL;
    }

    assert(find != NULL);

    find_len = strlen(find);
    remainder = s;

    while (*remainder != '\0') {
        find_start = strstr(remainder, find);

        /*
         * No more instances of find, concat the rest of the string on to the
         * result and finish
         */
        if (find_start == NULL) {
            xasprintf(&tmp, "%s%s", result ? result : "", remainder);
            free(result);
            result = tmp;

            break;
        } else {
            /*
             * Print the string up to the start of the match, then the
             * "replace" string.  Reset remainder to the end of the match.
             */
            if (replace) {
                xasprintf(&tmp, "%s%.*s%s", result ? result : "", (int) (find_start - remainder), remainder, replace);
            } else {
                xasprintf(&tmp, "%s%.*s", result ? result : "", (int) (find_start - remainder), remainder);
            }

            free(result);
            result = tmp;

            remainder = find_start + find_len;
        }
    }

    /*
     * Result could be NULL at this point if the input was an empty string.
     * In that case, just allocate a new empty string
     */
    if (result == NULL) {
        result = strdup(s);
        assert(result != NULL);
    }

    return result;
}

/**
 * @brief A variant on the idea of strreplace() that just escapes XML
 * special characters so the input string can be used as a CDATA
 * value.  The special characters are <, >, ", ', and &.  The caller
 * is responsible for freeing the returned string.
 *
 * @param s The string to escape XML special characters in.
 * @return Newly allocated string with modifications made; caller must
 * free this result.
 */
char *strxmlescape(const char *s)
{
    size_t len = 0;
    const char *walk = s;
    char *tmp = NULL;
    char *result = NULL;

    if (s == NULL) {
        return NULL;
    }

    /* allocate a buffer for the new string */
    len = BUFSIZ;
    result = calloc(1, len);
    assert(result != NULL);

    /* go through the string to build the new string */
    tmp = result;

    while (*walk != '\0') {
        /* escape special XML characters, otherwise copy as-is */
        if (*walk == '<') {
            tmp = stpcpy(tmp, "&lt;");
        } else if (*walk == '>') {
            tmp = stpcpy(tmp, "&gt;");
        } else if (*walk == '&') {
            tmp = stpcpy(tmp, "&amp;");
        } else if (*walk == '"') {
            tmp = stpcpy(tmp, "&quot;");
        } else if (*walk == '\'') {
            tmp = stpcpy(tmp, "&apos;");
        } else {
            tmp = stpncpy(tmp, walk, 1);
        }

        /* grow the buffer if we need more space */
        if ((len - strlen(result)) <= 8) {
            /* grow the result string */
            len += BUFSIZ;
            result = realloc(result, len);
            assert(result != NULL);

            /* reset the tmp pointer to the end of the new string */
            tmp = result;
            tmp += strlen(result) + 1;
        }

        /* next character */
        walk++;
    }

    /* shrink down the buffer to just the size we need */
    result = realloc(result, strlen(result) + 1);
    assert(result != NULL);

    return result;
}

/*
 * Append one or more strings to dest and return the result.  All
 * items to append must be of type 'char *'.  Terminate the list with
 * NULL.  Memory is allocated or reallocated and the first argument is
 * modified.  Caller is responsible for freeing memory.
 */
char *strappend(char *dest, ...)
{
    va_list sl;
    char *s = NULL;

    va_start(sl, dest);

    while ((s = va_arg(sl, char *)) != NULL) {
        if (dest == NULL) {
            dest = strdup(s);
            assert(dest != NULL);
        } else {
            dest = realloc(dest, strlen(dest) + strlen(s) + 1);
            assert(dest != NULL);
            dest = strcat(dest, s);
        }
    }

    va_end(sl);

    return dest;
}

/*
 * Split given string on delimiter.  Put each substring in a
 * string_list_t as a separate entry, return the list.  Caller
 * must free the list.
 */
string_list_t *strsplit(const char *s, const char *delim)
{
    char *walk = NULL;
    char *walkp = NULL;
    char *token = NULL;
    string_list_t *list = NULL;

    if (s == NULL) {
        return NULL;
    }

    /* given a string but no delim, just make a single entry list */
    if (s && (delim == NULL || !strcmp(delim, "") || !strcmp(s, delim))) {
        list = list_add(list, s);
        return list;
    }

    walk = strdup(s);
    assert(walk != NULL);
    walkp = walk;

    /* split the string and build the list */
    while ((token = strsep(&walk, delim)) != NULL) {
        if (!strcmp(token, "")) {
            continue;
        }

        list = list_add(list, token);
    }

    free(walkp);
    return list;
}

/**
 * @brief Return a string description of the file type given its
 *        stat(2) mode.
 *
 * Return a string describing the file type from the struct stat
 * st_mode.  Do not free the string returned.
 *
 * @param mode The file mode from stat(2).
 * @return String describing the file type; do not free.
 */
const char *strtype(const mode_t mode)
{
    if (S_ISREG(mode)) {
        return _("regular file");
    } else if (S_ISDIR(mode)) {
        return _("directory");
    } else if (S_ISCHR(mode)) {
        return _("character device");
    } else if (S_ISBLK(mode)) {
        return _("block device");
    } else if (S_ISFIFO(mode)) {
        return _("FIFO (named pipe)");
    } else if (S_ISLNK(mode)) {
        return _("symbolic link");
    } else if (S_ISSOCK(mode)) {
        return _("socket");
    } else {
        return _("UNKNOWN");
    }
}

/*
 * Given a string s and a width, shorten the string to be no longer
 * than width.  The center of the string is replaced with "..." and
 * the left and right parts of the string contain the maximum number
 * of characters allowed to represent the string and still be no wider
 * than width.  Caller must free the returned string.
 */
char *strshorten(const char *s, size_t width)
{
    char *r = NULL;
    char *tail = NULL;
    size_t input_len = 0;
    size_t half_width = 0;
    size_t left_width = 0;
    size_t right_width = 0;

    /* the empty case */
    if (width == 0 || s == NULL) {
        return r;
    }

    /* the simple case */
    input_len = strlen(s);

    if (input_len <= width) {
        return strdup(s);
    }

    /* allocate the buffer for the shortened string */
    r = calloc(1, width + 1);
    assert(r != NULL);
    tail = r;

    /* compute width of each half */
    half_width = (width - 3) / 2;
    left_width = right_width = half_width;

    if (((half_width * 2) + 3) != width) {
        left_width++;
    }

    /* create the shortened string */
    tail = stpncpy(tail, s, left_width);
    tail = stpcpy(tail, "...");
    s = s + input_len - right_width;
    (void) stpcpy(tail, s);

    return r;
}

const char *strexitcode(int exitcode)
{
    if (exitcode == RI_SUCCESS) {
        return _("Success.");
    } else if (exitcode == RI_INSPECTION_FAILURE) {
        return _("One or more inspections failed.");
    } else if (exitcode == RI_PROGRAM_ERROR) {
        return _("Program error.");
    } else if (exitcode == RI_MISSING_PROFILE) {
        return _("The specified profile is not found.");
    } else if (exitcode == RI_INSUFFICIENT_SPACE) {
        return _("Insufficient disk space in the working directory.");
    }

    return "";
}

/*
 * Trims leading and trailing whitespace from a dynamically allocated
 * string.  NOTE: The string is left allocated as is but shifted to
 * the beginning and unused chars are \0 at the end.
 */
char *strtrim(char *s)
{
    char *r = NULL;
    size_t i = 0;

    if (s == NULL) {
        return NULL;
    }

    /* the beginning of the string */
    r = s;

    /* remove leading whitespace */
    while (isspace(*s) && *s != '\0') {
        s++;
    }

    if (s == NULL) {
        return NULL;
    }

    /* shift everything to the head of the string */
    while (s[i] != '\0') {
        r[i] = s[i];
        i++;
    }

    r[i] = '\0';

    /* go to the end and remove trailing whitespace */
    i = strlen(s);

    while (isspace(s[i])) {
        s[i] = '\0';
        i--;
    }

    if (s == NULL) {
        return NULL;
    }

    return r;
}
