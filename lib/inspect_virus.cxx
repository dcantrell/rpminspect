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
#include "parallel.h"

static struct cl_engine *engine = NULL;
#ifndef CL_SCAN_STDOPT
struct cl_scan_options clamav_opts;
#endif

static parallel_t *col = NULL;

/* these variables have different values in each child */
static unsigned child_no = 0;
static unsigned file_no = (unsigned)-1;
static int virus_countdown = 4000;
static int write_fd;

static bool virus_driver(struct rpminspect *ri __attribute__((unused)), rpmfile_entry_t *file)
{
    int r = 0;
    const char *virus = NULL;

    /* cyclically count from 0 to NCHILDREN-1 */
    file_no++;

    if (file_no == col->max_pids) {
        file_no = 0;
    }

    /* handle only every Nth file */
    if (file_no != child_no) {
        return true;
    }

    /* only check regular files */
    if (!S_ISREG(file->st_mode)) {
        return true;
    }

    /* scan the file */
#ifndef CL_SCAN_STDOPT
    r = cl_scanfile(file->fullpath, &virus, NULL, engine, &clamav_opts);
#else
    r = cl_scanfile(file->fullpath, &virus, NULL, engine, CL_SCAN_STDOPT);
#endif
#if 0 /* debug: uncomment for error injection - test that virus detection indeed works */
    if (child_no == 0 && file_no == 0 && virus_countdown == 4000) {
        virus = "b0g0virus";
        r = CL_VIRUS;
    }
#endif

    if (r != CL_CLEAN && r != CL_VIRUS) {
        /* unexpected failure, bail out */
        errx(EXIT_FAILURE, "*** cl_scanfile(%s): %s", file->localpath, cl_strerror(r));
    }

    if (r == CL_VIRUS) {
        if (!virus || !virus[0]) {
            /* "nameless" virus? probably clamav bug, and our code would break on such: bail out */
            errx(EXIT_FAILURE, "*** cl_scanfile(%s): virus with no name???", file->localpath);
        }

        /* Cap the number of reported infections.
         * Receiving buffer has sanity limit, and we'd abort if it is exceeded.
         * If we see thousands of "infected" files, we probably aren't
         * interested in every one of them anyway.
         */
        if (virus_countdown != 0) {
            virus_countdown--;
            full_write(write_fd, virus, strlen(virus) + 1);
            full_write(write_fd, &file, sizeof(file));
        }
    }

    return true;
}

bool inspect_virus(struct rpminspect *ri)
{
    char *dbver = NULL;
    const char *dbpath = NULL;
    DIR *d = NULL;
    struct dirent *de = NULL;
    char *cvdpath = NULL;
    struct cl_cvd *cvd = NULL;
    int r = 0;
    struct result_params params;
    unsigned int loaded_signatures; /* unused, exists to make cl_load() happy */

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

    /* initialize clamav engine */
    engine = cl_engine_new();

    if (engine == NULL) {
        errx(RI_PROGRAM_ERROR, _("*** cl_engine_new returned NULL, check clamav library"));
    }

    /* load clamav databases */
    r = cl_load(dbpath, engine, &loaded_signatures, CL_DB_STDOPT);

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

#ifndef CL_SCAN_STDOPT
    /* set up the clamav scan options */
    memset(&clamav_opts, 0, sizeof(clamav_opts));
    clamav_opts.general = CL_SCAN_GENERAL_ALLMATCHES | CL_SCAN_GENERAL_COLLECT_METADATA;
    clamav_opts.parse = ~0;

    /* disable broken ELF detection */
    clamav_opts.parse &= ~CL_SCAN_HEURISTIC_BROKEN;
    /* disable max limit detection (filesize, etc) */
    clamav_opts.parse &= ~CL_SCAN_HEURISTIC_EXCEEDS_MAX;
#endif

    params.verb = VERB_OK;
    params.noun = NULL;
    params.file = NULL;
    params.arch = NULL;
    add_result(ri, &params);
    free(params.msg);
    params.msg = NULL;
    free(params.details);
    params.details = NULL;
    params.header = NAME_VIRUS;
    params.noun = _("virus or malware in ${FILE} on ${ARCH}");

    /* fork $NCPUS children */
    fflush(NULL);
    col = new_parallel(0); /* 0: will have one child per CPU */

    for (child_no = 0; child_no < col->max_pids; child_no++) {
        pid_t pid;
        int pipefd[2];

        if (pipe(pipefd)) {
            err(EXIT_FAILURE, "pipe"); /* fatal */
        }

        int rnd = rand();
        pid = fork();

        if (pid < 0) {
            err(EXIT_FAILURE, "fork"); /* fatal */
        }

        if (pid == 0) {
            /* child */
            close(pipefd[0]);
            write_fd = pipefd[1];

            /* "If you’re using libclamav with a forking daemon you
             * should call srand() inside a forked child before making
             * any calls to the libclamav functions" - clamav docs
             */
            srand(child_no ^ rnd);

            /* run the virus check on each Nth file, then exit */
            foreach_peer_file(ri, NAME_VIRUS, virus_driver);
            _exit(0);
        }

        /* parent */
        /* insert the child into collector */
        close(pipefd[1]);
        insert_new_pid_and_fd(col, pid, pipefd[0]);
    } /* forking N children */

    /* Let all children run, collecting their outputs.
     * When any one of them finish, process its output.
     * Repeat until all of them exit.
     */
    bool result = true;
    parallel_slot_t *slot;
    while ((slot = collect_one(col)) != NULL) {
        int r = slot->exit_status;

        if (!WIFEXITED(r)) {
            errx(EXIT_FAILURE, "cl_scanfile() killed by signal %u", WTERMSIG(r));
        }

        if (WEXITSTATUS(r) != 0) {
            errx(EXIT_FAILURE, "cl_scanfile() exited with %u", WEXITSTATUS(r));
        }

        char *output = slot->output;

        if (output) {
            /* consume all pairs of ("virusname",rpmfile_entry_t pointer) in output */
            while (output[0]) {
                rpmfile_entry_t *file;
                const char *virus = output;

                output += strlen(virus) + 1;
                memcpy(&file, output, sizeof(file)); /* copy unaligned bytes */
                output += sizeof(file);

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
            } /* while (there is unprocessed output from this child) */

            free(slot->output);
            slot->output = NULL; /* avoid double-free in delete_parallel() */
        }
    } /* while (waiting for a child to finish) */

    delete_parallel(col, /*signal:*/ 0);
    col = NULL;

    /* hope the result is always this */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_VIRUS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    /* clean up */
    cl_engine_free(engine);
    engine = NULL;

    return result;
}
