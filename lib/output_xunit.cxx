/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Output a results_t in XUnit format.  This can be consumed by Jenkins or
 * other services that can read in XUnit data (e.g., GitHub).
 */
void output_xunit(const results_t *results, const char *dest, const severity_t threshold, const severity_t suppress)
{
    results_entry_t *result = NULL;
    int r = 0;
    int count = 0;
    int total = 0;
    int failures = 0;
    FILE *fp = NULL;
    const char *header = NULL;
    char *msg = NULL;
    char *rawcdata = NULL;
    char *cdata = NULL;

    /* count up total test cases and total failures */
    TAILQ_FOREACH(result, results, items) {
        if (header == NULL || strcmp(header, result->header)) {
            total++;
        }

        header = result->header;
    }

    /* output the results */
    TAILQ_FOREACH(result, results, items) {
        /* Ignore suppressed results */
        if (suppressed_results(results, result->header, suppress)) {
            continue;
        }

        /* default to stdout unless a filename was specified */
        if (fp == NULL) {
            if (dest == NULL) {
                fp = stdout;
            } else {
                fp = fopen(dest, "w");
            }

            if (fp == NULL) {
                warn(_("*** opening %s for writing"), dest);
                return;
            }

            header = NULL;
            fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            fprintf(fp, "<testsuite tests=\"%d\" failures=\"%d\" errors=\"0\" skipped=\"0\">\n", total, failures);
        }

        if (header == NULL || strcmp(header, result->header)) {
            if (header != NULL) {
                fprintf(fp, "    </testcase>\n");
            }

            fprintf(fp, "    <testcase name=\"/%s\" classname=\"rpminspect\">\n", result->header);
            header = result->header;
            count = 1;
        }

        /* prepare the system out message */
        if (result->msg != NULL) {
            if (result->severity >= threshold) {
                fprintf(fp, "        <failure message=\"%s\">%s</failure>\n", result->msg, inspection_header_to_desc(result->header));
            }

            xasprintf(&msg, "%d) %s\n\n", count++, result->msg);
            assert(msg != NULL);
        }

        xasprintf(&rawcdata, _("Result: %s\n"), strseverity(result->severity));
        assert(rawcdata != NULL);

        if (msg) {
            msg = strappend(msg, rawcdata, NULL);
        } else {
            msg = strdup(rawcdata);
        }

        assert(msg != NULL);
        free(rawcdata);

        if (result->waiverauth > NULL_WAIVERAUTH) {
            xasprintf(&rawcdata, _("Waiver Authorization: %s\n\n"), strwaiverauth(result->waiverauth));
            assert(rawcdata != NULL);

            if (msg) {
                msg = strappend(msg, rawcdata, NULL);
            } else {
                msg = strdup(rawcdata);
            }

            assert(msg != NULL);
            free(rawcdata);
        }

        if (result->details != NULL) {
            xasprintf(&rawcdata, _("Details:\n%s\n\n"), result->details);
            assert(rawcdata != NULL);
            msg = strappend(msg, rawcdata, NULL);
            assert(msg != NULL);
            free(rawcdata);
        }

        if (result->remedy != NULL) {
            xasprintf(&rawcdata, _("Suggested Remedy:\n%s"), result->remedy);
            assert(rawcdata != NULL);
            msg = strappend(msg, rawcdata, NULL);
            assert(msg != NULL);
            free(rawcdata);
        }

        /* escape the string for XML CDATA use */
        cdata = strxmlescape(msg);
        assert(cdata != NULL);
        fprintf(fp, "        <system-out><![CDATA[%s]]></system-out>\n", cdata);
        free(cdata);
        free(msg);
        cdata = NULL;
        msg = NULL;
    }

    /* tidy up and return */
    if (fp != NULL) {
        if (header != NULL) {
            fprintf(fp, "    </testcase>\n");
        }

        fprintf(fp, "</testsuite>\n");
        r = fflush(fp);
        assert(r == 0);

        if (dest != NULL) {
            r = fclose(fp);
            assert(r == 0);
        }
    }

    return;
}
