/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stddef.h>
#include "rpminspect.h"

/*
 * Ensure the array of output formats is only defined once.
 */

struct format formats[] = {
    { FORMAT_TEXT,    "text",    &output_text },
    { FORMAT_JSON,    "json",    &output_json },
    { FORMAT_XUNIT,   "xunit",   &output_xunit },
    { FORMAT_SUMMARY, "summary", &output_summary },
    { -1, NULL, NULL }
};

const char *format_desc(unsigned int format)
{
    switch (format) {
        case FORMAT_TEXT:
            return _("Detailed results suitable for the console and piping through paging programs.");
        case FORMAT_JSON:
            return _("Results organized as a JSON data structure suitable for reading by web applications and other frontend tools.");
        case FORMAT_XUNIT:
            return _("Results organized as an XUnit data structure suitable for use with Jenkins and other XUnit-enabled services.");
        case FORMAT_SUMMARY:
            return _("Results summarized with one result per line, suitable for console viewing with a paging program.");
        default:
            return NULL;
    }
}
