/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <dirent.h>
#include <sys/types.h>
#include <clamav.h>
#include "rpminspect.h"

static bool clamav_ready = false;
static struct cl_engine *engine = NULL;
static unsigned int sigs = 0;
static struct result_params params;

static bool virus_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    cl_error_t r = 0;
#ifndef CL_SCAN_STDOPT
    struct cl_scan_options opts;
#endif
    const char *virus = NULL;

    /* only check regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* initialize clamav if we need to */
    if (!clamav_ready) {
        /* create clamav engine */
        engine = cl_engine_new();

        if (engine == NULL) {
            errx(RI_PROGRAM_ERROR, _("*** cl_engine_new returned NULL, check clamav library"));
        }

        /* load clamav databases */
        r = cl_load(cl_retdbdir(), engine, &sigs, CL_DB_STDOPT);

        if (r != CL_SUCCESS) {
            cl_engine_free(engine);
            errx(RI_PROGRAM_ERROR, "*** cl_load: %s", cl_strerror(r));
        }

        /* compile engine */
        r = cl_engine_compile(engine);

        if (r != CL_SUCCESS) {
            cl_engine_free(engine);
            errx(RI_PROGRAM_ERROR, "*** cl_engine_compile: %s", cl_strerror(r));
        }

        /* remember to not do all this again */
        clamav_ready = true;
    }

#ifndef CL_SCAN_STDOPT
    /* set up the clamav scan options */
    memset(&opts, 0, sizeof(opts));
    opts.general = CL_SCAN_GENERAL_ALLMATCHES | CL_SCAN_GENERAL_COLLECT_METADATA;
    opts.parse = ~0;

    /* disable broken ELF detection */
    opts.parse &= ~CL_SCAN_HEURISTIC_BROKEN;

    /* disable max limit detection (filesize, etc) */
    opts.parse &= ~CL_SCAN_HEURISTIC_EXCEEDS_MAX;

    /* scan the file */
    r = cl_scanfile(file->fullpath, &virus, NULL, engine, &opts);
#else
    /* scan the file */
    r = cl_scanfile(file->fullpath, &virus, NULL, engine, CL_SCAN_STDOPT);
#endif

    if (r == CL_VIRUS) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_VIRUS);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            if (params.severity == RESULT_INFO) {
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
            } else {
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.verb = VERB_FAILED;
                result = false;
            }

            params.arch = get_rpm_header_arch(file->rpm_header);
            params.file = file->localpath;
            params.remedy = get_remedy(REMEDY_VIRUS);
            xasprintf(&params.msg, _("Virus detected in %s in the %s package on %s: %s"), file->localpath, headerGetString(file->rpm_header, RPMTAG_NAME), params.arch, virus);
            add_result(ri, &params);
            free(params.msg);
        }
    } else if (r != CL_CLEAN) {
        warnx("*** cl_scanfile(%s): %s", file->localpath, cl_strerror(r));
    }

    return result;
}

bool inspect_virus(struct rpminspect *ri)
{
    char *dbver = NULL;
    const char *dbpath = NULL;
    DIR *d = NULL;
    struct dirent *de = NULL;
    char *cvdpath = NULL;
    struct cl_cvd *cvd = NULL;
    bool result = false;
    int r = 0;

    /* initialize clamav */
    r = cl_init(CL_INIT_DEFAULT);

    if (r != CL_SUCCESS) {
        warnx("*** cl_init: %s", cl_strerror(r));
        return false;
    }

    /* set up result parameters */
    init_result_params(&params);

    /* display version information about clamav */
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_VIRUS;
    xasprintf(&params.msg, _("clamav version information"));

    dbpath = cl_retdbdir();
    assert(dbpath != NULL);

    xasprintf(&params.details, _("clamav version %s"), cl_retver());

    d = opendir(dbpath);

    if (d == NULL) {
        err(EXIT_FAILURE, _("*** missing %s"), dbpath);
    }

    errno = 0;

    while ((de = readdir(d)) != NULL) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..") || (!strsuffix(de->d_name, ".cvd") && !strsuffix(de->d_name, ".cld"))) {
            continue;
        }

        xasprintf(&cvdpath, "%s/%s", dbpath, de->d_name);
        assert(cvdpath != NULL);
        cvd = cl_cvdhead(cvdpath);

        if (cvd == NULL) {
            free(cvdpath);

            if (closedir(d) == -1) {
                warn("*** closedir");
            }

            return false;
        }

        xasprintf(&dbver, _("%s version %u (%s)"), cvdpath, cvd->version, cvd->time);
        assert(dbver != NULL);

        params.details = strappend(params.details, "\n", dbver, NULL);
        assert(params.details != NULL);

        cl_cvdfree(cvd);
        free(cvdpath);
        free(dbver);
    }

    if (errno != 0) {
        free(params.details);
        err(EXIT_FAILURE, "*** readdir");
    }

    if (closedir(d) == -1) {
        warn("*** closedir");
    }

    params.verb = VERB_OK;
    params.noun = NULL;
    params.file = NULL;
    params.arch = NULL;
    add_result(ri, &params);
    free(params.msg);
    params.msg = NULL;
    free(params.details);
    params.details = NULL;

    /* run the virus check on each file */
    params.header = NAME_VIRUS;
    params.noun = _("virus or malware in ${FILE} on ${ARCH}");
    result = foreach_peer_file(ri, NAME_VIRUS, virus_driver);

    /* hope the result is always this */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_VIRUS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    /* clean up */
    if (engine) {
        cl_engine_free(engine);
    }

    return result;
}
