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

/* File inspection callback to track files being scanned */
static cl_error_t file_inspection_callback(int fd __attribute__((unused)),
                                           const char *type __attribute__((unused)),
                                           const char **pathnames,
                                           size_t parent_file_size __attribute__((unused)),
                                           const char *file_name,
                                           size_t file_size __attribute__((unused)),
                                           const char *file_buffer __attribute__((unused)),
                                           uint32_t level,
                                           uint32_t layer_attributes __attribute__((unused)),
                                           void *context)
{
    virus_scan_context_t *ctx = (virus_scan_context_t *) context;
    unsigned int i = 0;

    /* free previous file name */
    if (ctx->current) {
        free(ctx->current);
        ctx->current = NULL;
    }

    /* free previous pathnames */
    if (ctx->pathnames) {
        for (i = 0; i < ctx->level; i++) {
            free(ctx->pathnames[i]);
            ctx->pathnames[i] = NULL;
        }

        free(ctx->pathnames);
        ctx->pathnames = NULL;
    }

    ctx->level = level;

    /* copy current file name */
    if (file_name) {
        ctx->current = strdup(file_name);
        assert(ctx->current != NULL);
    }

    /* copy pathnames */
    if (pathnames && level > 0) {
        ctx->pathnames = calloc(level, sizeof(*(ctx->pathnames)));

        for (i = 0; i < level; i++) {
            if (pathnames[i]) {
                ctx->pathnames[i] = strdup(pathnames[i]);
                assert(ctx->pathnames[i] != NULL);
            }
        }
    }

    return CL_CLEAN;
}

/* Virus found callback to record the file path when a virus is detected */
static void virus_found_callback(int fd __attribute__((unused)),
                                 const char *virname __attribute__((unused)),
                                 void *context)
{
    virus_scan_context_t *ctx = (virus_scan_context_t *) context;
    unsigned int i = 0;

    /* build the full path including pathnames */
    if (ctx->virus_path) {
        free(ctx->virus_path);
        ctx->virus_path = NULL;
    }

    /* construct path: pathname1/pathname2/.../current */
    if (ctx->pathnames && ctx->level > 0) {
        ctx->virus_path = strdup(ctx->pathnames[0]);
        assert(ctx->virus_path != NULL);

        for (i = 1; i < ctx->level; i++) {
            if (ctx->pathnames[i]) {
                ctx->virus_path = joindelim('/', ctx->virus_path, ctx->pathnames[i], NULL);
            }
        }

        if (ctx->current) {
            ctx->virus_path = joindelim('/', ctx->virus_path, ctx->current, NULL);
        }
    } else if (ctx->current) {
        ctx->virus_path = strdup(ctx->current);
        assert(ctx->virus_path != NULL);
    }

    return;
}

static bool virus_driver(struct rpminspect *ri __attribute__((unused)), rpmfile_entry_t *file)
{
    int r = 0;
    unsigned int i = 0;
    const char *virus = NULL;
    const char *infected_file = "";
    virus_scan_context_t ctx;

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

    /* Initialize scan context */
    memset(&ctx, 0, sizeof(ctx));

    /* scan the file */
#ifndef CL_SCAN_STDOPT
    r = cl_scanfile_callback(file->fullpath, &virus, NULL, engine, &clamav_opts, &ctx);
#else
    r = cl_scanfile_callback(file->fullpath, &virus, NULL, engine, CL_SCAN_STDOPT, &ctx);
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

            /* Get the infected file path from context */
            if (ctx.virus_path && ctx.virus_path[0]) {
                infected_file = ctx.virus_path;
            }

            full_write(write_fd, virus, strlen(virus) + 1);
            full_write(write_fd, &file, sizeof(file));
            full_write(write_fd, infected_file, strlen(infected_file) + 1);
        }
    }

    /* Clean up context */
    if (ctx.current) {
        free(ctx.current);
    }

    if (ctx.pathnames) {
        for (i = 0; i < ctx.level; i++) {
            free(ctx.pathnames[i]);
        }

        free(ctx.pathnames);
    }

    if (ctx.virus_path) {
        free(ctx.virus_path);
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
            free(params.msg);
            free(params.details);

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

    /* scan large files, but dump error as warning if this doesn't work */
    r = cl_engine_set_num(engine, CL_ENGINE_MAX_FILESIZE, 0);

    if (r != CL_SUCCESS) {
        warnx("*** cl_engine_set_num: %s", cl_strerror(r));
    }

    r = cl_engine_set_num(engine, CL_ENGINE_MAX_SCANSIZE, 0);

    if (r != CL_SUCCESS) {
        warnx("*** cl_engine_set_num: %s", cl_strerror(r));
    }

    /* load clamav databases */
    r = cl_load(dbpath, engine, &loaded_signatures, CL_DB_STDOPT);

    if (r != CL_SUCCESS) {
        cl_engine_free(engine);
        errx(RI_PROGRAM_ERROR, "*** cl_load: %s", cl_strerror(r));
    }

    /* set up callbacks for tracking infected files within archives */
    cl_engine_set_clcb_file_inspection(engine, file_inspection_callback);
    cl_engine_set_clcb_virus_found(engine, virus_found_callback);

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
            if (close(pipefd[0]) == -1) {
                warn("*** close");
            }

            if (close(pipefd[1]) == -1) {
                warn("*** close");
            }

            err(EXIT_FAILURE, "fork"); /* fatal */
        }

        if (pid == 0) {
            /* child */
            if (close(pipefd[0]) == -1) {
                warn("*** close");
            }

            write_fd = pipefd[1];

            /* "If you’re using libclamav with a forking daemon you
             * should call srand() inside a forked child before making
             * any calls to the libclamav functions" - clamav docs
             */
            srand(child_no ^ rnd);

            /* run the virus check on each Nth file, then exit */
            foreach_peer_file(ri, NAME_VIRUS, virus_driver);

            if (close(pipefd[1]) == -1) {
                warn("*** close");
            }

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
            /* consume all triples of ("virusname",rpmfile_entry_t pointer,"infected_file") in output */
            while (output[0]) {
                rpmfile_entry_t *file;
                const char *virus = output;
                const char *infected_file = NULL;
                char *nevra = NULL;

                output += strlen(virus) + 1;
                memcpy(&file, output, sizeof(file)); /* copy unaligned bytes */
                output += sizeof(file);
                infected_file = output;
                output += strlen(infected_file) + 1;

                params.severity = get_secrule_result_severity(ri, file, SECRULE_VIRUS);

                if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                    nevra = get_nevra(file->rpm_header);
                    assert(nevra != NULL);

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
                    params.remedy = REMEDY_VIRUS;

                    if (headerIsSource(file->rpm_header)) {
                        if (infected_file && infected_file[0]) {
                            xasprintf(&params.msg, _("Virus detected in %s in the %s source package: %s (infected file: %s)"), file->localpath, nevra, virus, infected_file);
                        } else {
                            xasprintf(&params.msg, _("Virus detected in %s in the %s source package: %s"), file->localpath, nevra, virus);
                        }
                    } else {
                        if (infected_file && infected_file[0]) {
                            xasprintf(&params.msg, _("Virus detected in %s in the %s package: %s (infected file: %s)"), file->localpath, nevra, virus, infected_file);
                        } else {
                            xasprintf(&params.msg, _("Virus detected in %s in the %s package: %s"), file->localpath, nevra, virus);
                        }
                    }

                    add_result(ri, &params);
                    free(params.msg);
                    free(nevra);
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
