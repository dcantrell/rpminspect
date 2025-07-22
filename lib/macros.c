/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <ctype.h>
#include <regex.h>
#include <assert.h>
#include <err.h>
#include <rpm/rpmfileutil.h>
#include <rpm/rpmmacro.h>
#include "queue.h"
#include "rpminspect.h"

/**
 * Initialize macros for use by librpm calls.  This function should
 * only be called once for the lifetime of the program run.  The
 * caller needs to also call rpmFreeMacros() during cleanup as well.
 * This function can be safely called multiple times as it is guarded
 * by the macros_loaded boolean.
 */
void load_macros(struct rpminspect *ri)
{
    char *mf = NULL;
    char *macropath = NULL;

    assert(ri != NULL);

    if (ri->macros_loaded) {
        return;
    }

    if (ri->macrofiles) {
        macropath = list_to_string(ri->macrofiles, ":");
        mf = rpmGetPath(macropath, NULL);
        rpmInitMacros(rpmGlobalMacroContext, mf);
        free(macropath);
        free(mf);
    }

    ri->macros_loaded = true;
    return;
}
