/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <json.h>

#include "rpminspect.h"

/*
 * Output a results_t in JSON format.
 */
void output_json(const results_t *results, const char *dest, __attribute__((unused)) const severity_t threshold, const severity_t suppress)
{
    results_entry_t *result = NULL;
    int r = 0;
    int len = 0;
    FILE *fp = NULL;
    const char *header = NULL;
    const char* json_string = NULL;
    struct json_object *j = NULL;
    struct json_object *ji = NULL;
    struct json_object *jr = NULL;
    int flags = JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY;
    struct json_object_iter iter;

    assert(results != NULL);

    /* output the results */
    TAILQ_FOREACH(result, results, items) {
        /* Ignore suppressed results */
        if (suppressed_results(results, result->header, suppress)) {
            continue;
        }

        /*
         * The main results object.  Each inspection will be an array
         * with the results contained as array elements.
         */
        if (j == NULL) {
            j = json_object_new_object();
        }

        /* if we have a new inspection set, create a new object */
        if (header == NULL || strcmp(header, result->header)) {
            /* add previous inspection if there is one */
            if (ji != NULL) {
                json_object_object_add(j, header, json_object_get(ji));
            }

            /* new inspection begins now */
            header = result->header;
            ji = json_object_new_array();
        }

        /* create the object for this result */
        jr = json_object_new_object();
        json_object_object_add(jr, "result", json_object_new_string(strseverity(result->severity)));

        if (result->waiverauth > NULL_WAIVERAUTH) {
            json_object_object_add(jr, "waiver authorization", json_object_new_string(strwaiverauth(result->waiverauth)));
        }

        if (result->msg != NULL) {
            json_object_object_add(jr, "message", json_object_new_string(result->msg));
        }

        if (result->details != NULL) {
            json_object_object_add(jr, "details", json_object_new_string(result->details));
        }

        if (result->remedy != NULL) {
            json_object_object_add(jr, "remedy", json_object_new_string(result->remedy));
        }

        /* add this result data to the inspection array */
        json_object_array_add(ji, jr);
    }

    /* add the final inspection */
    if (header != NULL && j != NULL) {
        json_object_object_add(j, header, json_object_get(ji));
    }

    /* output results */
    if (j != NULL) {
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

        /* write out the results */
        json_string = json_object_to_json_string_ext(j, flags);
        if (json_string == NULL) {
            errx(RI_PROGRAM_ERROR, "*** failed to stringify object to json format");
        }
        /* write out the results */
        fprintf(fp, "%s\n", json_string);

        /* tidy up and return */
        r = fflush(fp);
        assert(r == 0);

        if (dest != NULL) {
            r = fclose(fp);
            assert(r == 0);
        }

        /* clean up the json library memory allocation */
        json_object_object_foreachC(j, iter) {
            if (json_object_get_type(iter.val) == json_type_array) {
                len = json_object_array_length(iter.val);

                for (r = 0; r < len; r++) {
                    json_object_array_put_idx(iter.val, r, NULL);
                }
            }

            json_object_put(iter.val);
        }

        json_object_put(j);
    }

    return;
}
