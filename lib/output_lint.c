/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

/*
 * Output a results_t in an rpmlint-style format.  Results not
 * normally shown by rpmlint are suppressed in this output mode.
 */
void output_lint(const results_t *results, const char *dest, __attribute__((unused)) const severity_t threshold, const severity_t suppress)
{
    results_entry_t *result = NULL;
    int r = 0;
    int count = 0;
    int len = 0;
    bool displayed_header = false;
    bool first = true;
    FILE *fp = NULL;
    const char *header = NULL;
    char *msg = NULL;
    size_t width = tty_width();

    /* output the results */
    TAILQ_FOREACH(result, results, items) {



    PRCO = ('REQUIRES', 'PROVIDES', 'CONFLICTS', 'OBSOLETES',
            'RECOMMENDS', 'SUGGESTS', 'ENHANCES', 'SUPPLEMENTS')

    # {fname : (size, mode, mtime, flags, dev, inode,                           
    #           nlink, state, vflags, user, group, digest)}                     
    __FILEIDX = [['S', 0],
                 ['M', 1],
                 ['5', 11],
                 ['D', 4],
                 ['N', 6],
                 ['L', 7],
                 ['V', 8],
                 ['U', 9],
                 ['G', 10],
                 ['F', 3],
                 ['T', 2]]

    DEPFORMAT = '%-12s%s %s %s %s'
    FORMAT = '%-12s%s'

    ADDED = 'added'
    REMOVED = 'removed'



        /* section header */
        if (header == NULL || strcmp(header, result->header)) {
            header = result->header;
            displayed_header = false;
            count = 1;
        }

        /* Ignore suppressed results */
        if (suppressed_results(results, header, suppress)) {
            continue;
        }

        /* ensure we have an output */
        if (fp == NULL) {
            /* default to stdout unless a filename was specified */
            if (dest == NULL) {
                fp = stdout;
            } else {
                fp = fopen(dest, "w");

                if (fp == NULL) {
                    warn(_("*** error opening %s for writing"), dest);
                    return;
                }
            }
        }

        /* display the next section header */
        if (!displayed_header) {
            if (first) {
                first = false;
            } else {
                fprintf(fp, "\n");
            }

            len = strlen(header) + 1;
            fprintf(fp, "%s:\n", header);

            for (r = 0; r < len; r++) {
                fprintf(fp, "-");
            }

            fprintf(fp, "\n");
            displayed_header = true;
        }

        if (!strcmp(header, NAME_DIAGNOSTICS) || (result->severity >= suppress)) {
            if (result->msg != NULL) {
                xasprintf(&msg, "%d) %s\n", count++, result->msg);

                if (width) {
                    printwrap(msg, width, 0, fp);
                } else {
                    fprintf(fp, "%s", msg);
                }

                fprintf(fp, "\n");
                free(msg);
            }

            fprintf(fp, _("Result: %s\n"), strseverity(result->severity));

            if (result->waiverauth > NULL_WAIVERAUTH) {
                fprintf(fp, _("Waiver Authorization: %s\n\n"), strwaiverauth(result->waiverauth));
            }

            if (result->details != NULL) {
                fprintf(fp, _("Details:\n%s\n\n"), result->details);
            }

            if (result->remedy != NULL) {
                xasprintf(&msg, _("Suggested Remedy:\n%s"), result->remedy);

                if (width) {
                    printwrap(msg, width, 0, fp);
                } else {
                   fprintf(fp, "%s", msg);
                }

                free(msg);
            }

            fprintf(fp, "\n");
        }
    }

    /* tidy up and return */
    if (fp != NULL) {
        r = fflush(fp);
        assert(r == 0);

        if (dest != NULL) {
            r = fclose(fp);
            assert(r == 0);
        }
    }

    return;
}
