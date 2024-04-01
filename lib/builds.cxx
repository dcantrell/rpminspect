/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file builds.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief Collect builds from either a local or remote (Koji) source.
 * @copyright LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <libgen.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <rpm/rpmlib.h>
#include "parser.h"
#include "rpminspect.h"

/* Local global variables */
static struct rpminspect *workri = NULL;
static int whichbuild = BEFORE_BUILD;
static bool fetch_only = false;
static int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

/* This array holds strings that map to the whichbuild index value. */
static char *build_desc[] = { "before", "after" };

/* Local prototypes */
static void set_worksubdir(struct rpminspect *, workdir_t, const struct koji_build *, const struct koji_task *);
static void get_rpm_info(const char *);
static void prune_local(const int);
static int copytree(const char *, const struct stat *, int, struct FTW *);
static int download_build(struct rpminspect *, const struct koji_build *);
static int download_task(struct rpminspect *, struct koji_task *);
static int download_rpm(struct rpminspect *ri, const char *rpm);

/*
 * Set the working subdirectory for this particular run based on whether
 * this is a remote build or a local build.
 */
static void set_worksubdir(struct rpminspect *ri, workdir_t wd, const struct koji_build *build, const struct koji_task *task)
{
    assert(ri != NULL);
    assert(wd != NULL_WORKDIR);

    if (ri->worksubdir != NULL) {
        return;
    }

    if (fetch_only) {
        if (build != NULL) {
            xasprintf(&ri->worksubdir, "%s/%s", ri->workdir, build->nvr);
        } else if (task != NULL) {
            xasprintf(&ri->worksubdir, "%s/scratch-%d", ri->workdir, task->id);
        } else {
            ri->worksubdir = strdup(ri->workdir);
            assert(ri->worksubdir);
        }

        if (mkdirp(ri->worksubdir, mode)) {
            err(RI_PROGRAM_ERROR, _("*** unable to create download directory %s"), ri->worksubdir);
        }
    } else {
        if (wd == LOCAL_WORKDIR) {
            xasprintf(&ri->worksubdir, "%s/local.XXXXXX", ri->workdir);
        } else if (wd == TASK_WORKDIR) {
            assert(task != NULL);
            xasprintf(&ri->worksubdir, "%s/scratch-%d.XXXXXX", ri->workdir, task->id);
        } else if (wd == BUILD_WORKDIR) {
            assert(build != NULL);
            xasprintf(&ri->worksubdir, "%s/%s-%s.XXXXXX", ri->workdir, build->name, build->version);
        } else {
            errx(RI_PROGRAM_ERROR, _("*** unknown working directory type: %d"), wd);
        }

        if (mkdtemp(ri->worksubdir) == NULL) {
            err(RI_PROGRAM_ERROR, "*** mkdtemp");
        }
    }

    return;
}

/*
 * Collect package peer information.
 */
static void get_rpm_info(const char *pkg)
{
    Header h;

    assert(pkg != NULL);
    h = get_rpm_header(workri, pkg);

    if (h == NULL) {
        return;
    }

    add_peer(&workri->peers, workri->deprules_ignore, whichbuild, fetch_only, pkg, h);
    return;
}

/*
 * Walk a local build tree and prune empty arch subdirectories.
 */
static void prune_local(const int whichbuild)
{
    char *lpath = NULL;
    char *apath = NULL;
    DIR *d = NULL;
    struct dirent *de = NULL;

    if (fetch_only) {
        lpath = strdup(workri->worksubdir);
    } else {
        xasprintf(&lpath, "%s/%s", workri->worksubdir, build_desc[whichbuild]);
    }

    if ((d = opendir(lpath)) == NULL) {
        free(lpath);
        return;
    }

    while ((de = readdir(d)) != NULL) {
        xasprintf(&apath, "%s/%s", lpath, de->d_name);
        (void) rmdir(apath);
        free(apath);
    }

    if (closedir(d) == -1) {
        warn("*** closedir");
        return;
    }

    free(lpath);

    return;
}

/*
 * Used to recursively copy a build tree over to the working directory.
 */
static int copytree(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    static int toptrim = 0;
    char *bufpath = NULL;
    Header h;
    const char *arch = NULL;
    int ret = 0;

    /*
     * On our first call, take the length of the main directory that
     * we will work relative to for this rescursive copy.
     */
    if (ftwbuf->level == 0 && S_ISDIR(sb->st_mode)) {
        toptrim = strlen(fpath) + 1;
        return 0;
    }

    /* Ignore unreadable things */
    if (tflag == FTW_DNR || tflag == FTW_NS || tflag == FTW_SLN) {
        warnx(_("*** unable to read %s, skipping"), fpath);
        return 0;
    }

    /* Copy file or create a directory */
    xasprintf(&bufpath, "%s/%s/%s", workri->worksubdir, build_desc[whichbuild], ((char *) fpath) + toptrim);

    if (S_ISDIR(sb->st_mode)) {
        if (mkdirp(bufpath, mode)) {
            ret = -1;
        }
    } else if (S_ISREG(sb->st_mode) || S_ISLNK(sb->st_mode)) {
        h = get_rpm_header(workri, fpath);

        if (h) {
            /* filter out RPMs from excluded architectures */
            arch = get_rpm_header_arch(h);

            if (!allowed_arch(workri, arch)) {
                free(bufpath);
                return 0;
            }
        }

        if (copyfile(fpath, bufpath, true, false)) {
            warn("*** copyfile");
            ret = -1;
        }

        /* Gather the RPM header for packages */
        get_rpm_info(bufpath);
    } else {
        warnx(_("*** unknown directory member encountered: %s"), fpath);
        ret = -1;
    }

    free(bufpath);

    return ret;
}

/* lambda for adding filtered entries. */
static bool filter_cb(const char *entry, void *cb_data)
{
    string_list_t *filter = (string_list_t **) cb_data;
    list_add(filter, entry);
    return false;
}

/* Display insufficient space error. */
static void report_insufficient_space(const unsigned long int avail, const unsigned long int need, const char *workdir, const char *type)
{
    char *availh = NULL;
    char *needh = NULL;

    assert(workdir != NULL);
    assert(type != NULL);

    availh = human_size(avail);
    needh = human_size(need);

    warnx(_("There is not enough available space to download the requested %s.\n"), type);
    warnx(_("    Need %s in %s, have %s.\n"), needh, workdir, availh);
    warnx(_("See the `-w' option for specifying an alternate working directory.\n"));

    free(availh);
    free(needh);
    return;
}

/* Display warning indicating the required space is unknown. */
static void report_unknown_space(const char *workdir, const char *type)
{
    assert(workdir != NULL);
    assert(type != NULL);

    warnx(_("Unable to determine the required space to download the requested %s.\n"), type);
    warnx(_("Ensure %s has sufficient space available."), workdir);

    return;
}

/*
 * Given a remote artifact specification in a Koji build, download it
 * to our working directory.
 */
static int download_build(struct rpminspect *ri, const struct koji_build *build)
{
    unsigned long int avail = 0;
    size_t total_width = 0;
    koji_buildlist_entry_t *buildentry = NULL;
    koji_rpmlist_entry_t *rpm = NULL;
    const char *downloading = NULL;
    bool displayed = false;
    char *verbose_msg = NULL;
    size_t mlen = 0;
    char *mend = NULL;
    int i = 0;
    char *src = NULL;
    char *srcfmt = NULL;
    char *dst = NULL;
    char *pkg = NULL;
    parser_plugin *p = &yaml_parser;
    parser_context *ctx = NULL;
    string_list_t *filter = NULL;

    assert(build != NULL);
    assert(build->builds != NULL);

    if (build->total_size == 0) {
        return -1;
    }

    /* Check to see that there's enough disk space available */
    avail = get_available_space(workri->workdir);

    if (avail > 0 && avail < build->total_size) {
        report_insufficient_space(avail, build->total_size, workri->workdir, _("build"));
        rmtree(workri->worksubdir, true, false);
        return RI_INSUFFICIENT_SPACE;
    } else if (avail == 0) {
        report_unknown_space(workri->workdir, _("build"));
    } else {
        ri->download_size += build->total_size;
    }

    /* set the working subdirectory */
    set_worksubdir(workri, BUILD_WORKDIR, build, NULL);

    /* Iterate over list of builds, each with a list of packages */
    TAILQ_FOREACH(buildentry, build->builds, builditems) {
        if (TAILQ_EMPTY(buildentry->rpms)) {
            /*
             * not sure how you could have an empty package list,
             * but let's not underestimate what jobs people can
             * submit to koji
             */
            continue;
        }

        /* Download status output */
        if (ri->verbose && ((ri->buildtype == KOJI_BUILD_MODULE && !displayed) || ri->buildtype != KOJI_BUILD_MODULE)) {
            /* the progress bar will need the terminal width */
            total_width = tty_width();

            /* generate our downloading message */
            if (ri->buildtype == KOJI_BUILD_MODULE) {
                downloading = _("Downloading module");
                displayed = true;
            } else {
                downloading = _("Downloading RPM build");
            }

            xasprintf(&verbose_msg, "%s %s-%s-%s", downloading, build->package_name, build->version, build->release);
            assert(verbose_msg != NULL);

            /* truncate the message to one line if it is too long */
            mlen = strlen(verbose_msg);

            if (mlen > total_width) {
                mend = verbose_msg + total_width - 3;

                for (i = 0; i < 3; i++) {
                    if (*mend == '\0') {
                        break;
                    }

                    *mend++ = '.';
                }

                *mend = '\0';
            }

            /* display the message */
            printf("%s\n", verbose_msg);
            free(verbose_msg);
        }

        /* Download module metadata at the top level */
        if (ri->buildtype == KOJI_BUILD_MODULE) {
            /* Create destination directory */
            if (fetch_only) {
                xasprintf(&dst, "%s/files", workri->worksubdir);
            } else {
                xasprintf(&dst, "%s/%s/files", workri->worksubdir, build_desc[whichbuild]);
            }

            if (mkdirp(dst, mode)) {
                return -1;
            }

            /* Get the main metadata file */
            dst = strappend(dst, "/", MODULEMD_FILENAME, NULL);
            xasprintf(&src, "%s/packages/%s/%s/%s/files/module/%s", workri->kojimbs, build->package_name, build->version, build->release, MODULEMD_FILENAME);
            curl_get_file(workri->verbose, src, dst);
            free(src);

            /* Get the list of artifacts to filter if we don't have it */
            if (filter == NULL) {
                if (p->parse_file(&ctx, dst)) {
                    warnx(_("*** ignoring malformed module metadata file: %s"), dst);
                    return -1;
                }

                /* Initialize a string list. */
                filter = (string_list_t *) calloc(1, sizeof(*filter));
                assert(filter != NULL);
                TAILQ_INIT(filter);

                if (p->strarray_foreach(ctx, "filter", "rpms", filter_cb, filter)) {
                    warnx(_("*** malformed rpm filters in file: %s"), dst);
                    free(filter);
                    return -1;
                }

                p->fini(ctx);
            }

            /* Need to use this in the next loop */
            free(dst);
        }

        /* Iterate over the list of packages for this build */
        TAILQ_FOREACH(rpm, buildentry->rpms, items) {
            /* skip arches the user wishes to exclude */
            if (!allowed_arch(ri, rpm->arch)) {
                continue;
            }

            /* create the destination directory */
            if (fetch_only) {
                xasprintf(&dst, "%s/%s", workri->worksubdir, rpm->arch);
            } else {
                xasprintf(&dst, "%s/%s/%s", workri->worksubdir, build_desc[whichbuild], rpm->arch);
            }

            if (mkdirp(dst, mode)) {
                return -1;
            }

            free(dst);

            /* for modules, get the per-arch module metadata */
            if (workri->buildtype == KOJI_BUILD_MODULE) {
                if (fetch_only) {
                    xasprintf(&dst, "%s/%s/%s", workri->worksubdir, rpm->arch, rpm->arch, MODULEMD_ARCH_FILENAME);
                } else {
                    xasprintf(&dst, "%s/%s/%s/%s", workri->worksubdir, build_desc[whichbuild], rpm->arch, rpm->arch, MODULEMD_ARCH_FILENAME);
                }

                /* only download this file if we have not already gotten it */
                if (access(dst, F_OK|R_OK)) {
                    xasprintf(&src, "%s/packages/%s/%s/%s/files/module/%s", workri->kojimbs, build->package_name, build->version, build->release, rpm->arch, MODULEMD_ARCH_FILENAME);
                    curl_get_file(workri->verbose, src, dst);
                    free(src);
                }

                free(dst);
            }

            /* for module builds, filter out packages */
            if (workri->buildtype == KOJI_BUILD_MODULE && filter != NULL) {
                if (list_contains(filter, rpm->name)) {
                    continue;
                }
            }

            /* build path strings */
            xasprintf(&pkg, "%s-%s-%s.%s.rpm", rpm->name, rpm->version, rpm->release, rpm->arch);

            if (fetch_only) {
                xasprintf(&dst, "%s/%s/%s", workri->worksubdir, rpm->arch, pkg);
            } else {
                xasprintf(&dst, "%s/%s/%s/%s", workri->worksubdir, build_desc[whichbuild], rpm->arch, pkg);
            }

            if (build->volume_name == NULL || !strcmp(build->volume_name, "DEFAULT")) {
                srcfmt = "%s/%s/%s/%s/%s/%s/%s";
            } else {
                srcfmt = "%s/vol/%s/packages/%s/%s/%s/%s/%s";
            }

            /* construct the source download url */
            xasprintf(&src, srcfmt,
                      (workri->buildtype == KOJI_BUILD_MODULE) ? workri->kojimbs : workri->kojiursine,
                      (build->volume_name == NULL || !strcmp(build->volume_name, "DEFAULT")) ? "packages" : build->volume_name,
                      (buildentry->package_name == NULL) ? build->name : buildentry->package_name,
                      (buildentry->version == NULL) ? build->version : buildentry->version,
                      (buildentry->release == NULL) ? build->release : buildentry->release,
                      rpm->arch,
                      pkg);

            /* download the package */
            curl_get_file(workri->verbose, src, dst);

            /* gather the RPM header */
            get_rpm_info(dst);

            /* start over */
            free(src);
            free(dst);
            free(pkg);
        }

        list_free(filter, free);
        filter = NULL;
    }

    return RI_SUCCESS;
}

/*
 * Given a remote artifact specification in a Koji task, download it
 * to our working directory.
 */
static int download_task(struct rpminspect *ri, struct koji_task *task)
{
    unsigned long int avail = 0;
    size_t len;
    char *pkg = NULL;
    char *src = NULL;
    char *dst = NULL;
    char *tail = NULL;
    koji_task_entry_t *descendent = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(task != NULL);
    assert(task->descendents != NULL);

    /* compute total size of all files to download for the task */
    TAILQ_FOREACH(descendent, task->descendents, items) {
        /* skip if we have nothing */
        if (TAILQ_EMPTY(descendent->srpms) && TAILQ_EMPTY(descendent->rpms)) {
            continue;
        }

        /* size of SRPMs */
        if (allowed_arch(workri, "src")) {
            TAILQ_FOREACH(entry, descendent->srpms, items) {
                xasprintf(&src, "%s/work/%s", workri->kojiursine, entry->data);
                task->total_size += curl_get_size(src);
                free(src);
            }
        }

        /* size of RPMs */
        TAILQ_FOREACH(entry, descendent->rpms, items) {
            /* skip arches the user wishes to exclude */
            if (!allowed_arch(ri, descendent->task->arch)) {
                continue;
            }

            xasprintf(&src, "%s/work/%s", workri->kojiursine, entry->data);
            task->total_size += curl_get_size(src);
            free(src);
        }
    }

    /* zero size means a read error */
    if (task->total_size == 0) {
        return -1;
    }

    /* Check to see that there's enough disk space available */
    avail = get_available_space(workri->workdir);

    if (avail > 0 && avail < task->total_size) {
        report_insufficient_space(avail, task->total_size, workri->workdir, _("task"));
        rmtree(workri->worksubdir, true, false);
        return RI_INSUFFICIENT_SPACE;
    } else if (avail == 0) {
        report_unknown_space(workri->workdir, _("task"));
    } else {
        ri->download_size += task->total_size;
    }

    /* set working subdirectory */
    set_worksubdir(workri, TASK_WORKDIR, NULL, task);

    /* download the task */
    TAILQ_FOREACH(descendent, task->descendents, items) {
        /* skip if we have nothing */
        if (TAILQ_EMPTY(descendent->srpms) && TAILQ_EMPTY(descendent->rpms)) {
            continue;
        }

        /* create the destination directory */
        if (fetch_only) {
            xasprintf(&dst, "%s/%s", workri->worksubdir, descendent->task->arch);
        } else {
            xasprintf(&dst, "%s/%s/%s", workri->worksubdir, build_desc[whichbuild], descendent->task->arch);
        }

        if (mkdirp(dst, mode)) {
            free(dst);
            return -1;
        }

        free(dst);

        /* download SRPMs */
        if (allowed_arch(ri, "src")) {
            TAILQ_FOREACH(entry, descendent->srpms, items) {
                pkg = basename(entry->data);

                if (fetch_only) {
                    xasprintf(&dst, "%s/src", workri->worksubdir);
                } else {
                    xasprintf(&dst, "%s/%s/src", workri->worksubdir, build_desc[whichbuild]);
                }

                if (mkdirp(dst, mode)) {
                    free(dst);
                    return -1;
                }

                len = strlen(dst);
                dst = (char *) realloc(dst, len + strlen(pkg) + 2);
                tail = dst + len;
                tail = stpcpy(tail, "/");
                (void) stpcpy(tail, pkg);
                assert(dst != NULL);

                xasprintf(&src, "%s/work/%s", workri->kojiursine, entry->data);
                curl_get_file(workri->verbose, src, dst);

                /* gather the RPM header */
                get_rpm_info(dst);

                free(dst);
                free(src);
            }
        }

        TAILQ_FOREACH(entry, descendent->rpms, items) {
            /* skip arches the user wishes to exclude */
            if (!allowed_arch(ri, descendent->task->arch)) {
                continue;
            }

            pkg = basename(entry->data);

            if (fetch_only) {
                xasprintf(&dst, "%s/%s/%s", workri->worksubdir, descendent->task->arch, pkg);
            } else {
                xasprintf(&dst, "%s/%s/%s/%s", workri->worksubdir, build_desc[whichbuild], descendent->task->arch, pkg);
            }

            xasprintf(&src, "%s/work/%s", workri->kojiursine, entry->data);
            curl_get_file(workri->verbose, src, dst);

            /* gather the RPM header */
            get_rpm_info(dst);

            free(dst);
            free(src);
        }
    }

    return RI_SUCCESS;
}

/*
 * Given a remote RPM, download it to our working directory.
 */
static int download_rpm(struct rpminspect *ri, const char *rpm)
{
    unsigned long int avail = 0;
    unsigned long int rpmsize = 0;
    char *pkg = NULL;
    char *dstdir = NULL;
    char *dst = NULL;

    assert(rpm != NULL);

    /* Check to see that there's enough disk space available */
    rpmsize = curl_get_size(rpm);

    if (rpmsize == 0) {
        return -1;
    }

    avail = get_available_space(workri->workdir);

    if (avail > 0 && avail < rpmsize) {
        report_insufficient_space(avail, rpmsize, workri->workdir, _("RPM"));
        return RI_INSUFFICIENT_SPACE;
    } else if (avail == 0) {
        report_unknown_space(workri->workdir, _("RPM"));
    } else {
        ri->download_size += rpmsize;
    }

    /* create the destination directory */
    if (fetch_only) {
        dstdir = strdup(workri->worksubdir);
    } else {
        xasprintf(&dstdir, "%s/%s", workri->worksubdir, build_desc[whichbuild]);
    }

    if (mkdirp(dstdir, mode)) {
        free(dstdir);
        return -1;
    }

    /* the RPM filename */
    pkg = strdup(rpm);
    assert(pkg != NULL);

    /* set working subdirectory */
    set_worksubdir(workri, LOCAL_WORKDIR, NULL, NULL);

    /* download the package */
    xasprintf(&dst, "%s/%s", dstdir, basename(pkg));
    curl_get_file(workri->verbose, rpm, dst);

    /* gather the RPM header */
    get_rpm_info(dst);

    /* clean up */
    free(pkg);
    free(dst);
    free(dstdir);

    return RI_SUCCESS;
}

/* Returns true if the string specifies a task ID, which is just an int */
static bool is_task_id(const char *id)
{
    int i = 0;

    assert(id != NULL);

    while (id[i] != '\0') {
        if (!isdigit(id[i])) {
            return false;
        }

        i++;
    }

    return true;
}

/*
 * Try to see if a given koji_task can be represented as a koji_build.
 * If so, convert it and return the resulting koji_build.  If it
 * can't, return NULL.
 */
static struct koji_build *get_koji_task_as_build(const struct koji_task *task)
{
    struct koji_build *build = NULL;
    size_t i = 0;
    char *srpm = NULL;
    char *nvr = NULL;
    const char *srpmext = "." SRPM_ARCH_NAME RPM_FILENAME_EXTENSION;
    koji_task_entry_t *descendent = NULL;
    string_list_t *srpmlist = NULL;
    string_entry_t *entry = NULL;

    assert(task != NULL);

    /* unable to determine candidate NVR without a SRPM */
    if (!allowed_arch(workri, "src")) {
        return NULL;
    }

    /* we need exactly one SRPM */
    TAILQ_FOREACH(descendent, task->descendents, items) {
        if (!TAILQ_EMPTY(descendent->srpms)) {
            srpmlist = descendent->srpms;
            i += list_len(descendent->srpms);
        }
    }

    if (i != 1) {
        return NULL;
    }

    /* guess the NVR for the build */
    entry = TAILQ_FIRST(srpmlist);

    if (entry == NULL || entry->data == NULL) {
        return NULL;
    }

    srpm = strdup(entry->data);
    assert(srpm != NULL);

    if (strrchr(srpm, '/')) {
        nvr = strrchr(srpm, '/');
        nvr++;
    } else {
        nvr = srpm;
    }

    if (nvr == NULL) {
        free(srpm);
        return NULL;
    }

    i = strlen(nvr);

    if (strsuffix(nvr, srpmext)) {
        i -= strlen(srpmext);
        nvr[i] = '\0';
    }

    /* try to get the build */
    build = get_koji_build(workri, nvr);
    free(srpm);

    return build;
}

/* Gather local build */
static int gather_local_build(struct rpminspect *ri, const char *build)
{
    char * modulemd_path = NULL;

    assert(build != NULL);

    if (fetch_only) {
        warnx(_("*** `%s' already exists in %s"), build, workri->workdir);
        return -1;
    }

    set_worksubdir(workri, LOCAL_WORKDIR, NULL, NULL);

    /*
     * Some inspections need to know which kind of build are testing. Remote
     * builds know this from the metadata provided by Koji. For local builds, we
     * need to tell it by the presence of the modulemd file.
     */
    modulemd_path = strdup(build);
    modulemd_path = strappend(modulemd_path, "/files/", MODULEMD_FILENAME, NULL);

    if (access(modulemd_path, F_OK) == 0){
        ri->buildtype = KOJI_BUILD_MODULE;
    } else{
        ri->buildtype = KOJI_BUILD_RPM;
    }
    free(modulemd_path);

    /* copy after tree */
    if (nftw(build, copytree, FOPEN_MAX, FTW_PHYS) == -1) {
        warn("*** nftw");
        return -1;
    }

    /* clean up */
    prune_local(whichbuild);

    return 0;
}

static int _gather_build_types(struct rpminspect *ri)
{
    int r = 0;
    bool gathered = false;
    struct koji_build *build = NULL;
    struct koji_build *innerbuild = NULL;
    struct koji_task *task = NULL;
    const char *spec = NULL;

    assert(ri != NULL);

    if (whichbuild == AFTER_BUILD) {
        spec = ri->after;
    } else if (whichbuild == BEFORE_BUILD) {
        spec = ri->before;
    }

    /* try for a local Koji build or local RPM */
    if (!gathered && (is_local_build(ri->workdir, spec, fetch_only) || is_local_rpm(ri, spec))) {
        if (gather_local_build(ri, spec) == -1) {
            warnx(_("*** unable to find local build: %s"), spec);
            return -1;
        } else {
            gathered = true;
        }
    }

    /* try for remote RPM */
    if (!gathered && is_remote_rpm(spec)) {
        r = download_rpm(ri, spec);

        if (r != RI_SUCCESS) {
            warnx(_("*** unable to download RPM: %s"), spec);
            return r;
        } else {
            gathered = true;
        }
    }

    /* try for a Koji task identifier */
    if (!gathered && is_task_id(spec)) {
        task = get_koji_task(ri, spec);

        if (task != NULL) {
            innerbuild = get_koji_task_as_build(task);

            if (innerbuild) {
                r = download_build(ri, innerbuild);
                free_koji_build(innerbuild);

                if (r != RI_SUCCESS) {
                    warnx(_("*** unable to find build %s in Koji hub %s"), spec, ri->kojihub);
                    free_koji_task(task);
                    return r;
                } else {
                    gathered = true;
                }

                free_koji_task(task);
            } else {
                r = download_task(ri, task);

                if (r != RI_SUCCESS) {
                    warnx(_("*** unable to find task %s in Koji hub %s"), spec, ri->kojihub);
                    free_koji_task(task);
                    return r;
                } else {
                    gathered = true;
                }

                free_koji_task(task);
            }
        }
    }

    /* try for a Koji build */
    if (!gathered) {
        build = get_koji_build(ri, spec);

        if (build != NULL) {
            r = download_build(ri, build);
            free_koji_build(build);

            if (r != RI_SUCCESS) {
                warnx(_("*** unable to find build %s in Koji hub %s"), spec, ri->kojihub);
                return r;
            } else {
                gathered = true;
            }
        }
    }

    /* still not found a build and we're this far? */
    if (!gathered) {
        warnx(_("*** unable to find build %s in Koji hub %s"), spec, ri->kojihub);
        return -1;
    }

    return 0;
}

/**
 * @brief Collects specified builds in to the working directory.
 *
 * For each build that is not NULL, determine if it is local or remote
 * and collect it in the appropriate manner.  For local builds, that
 * is just copying files.  For remote builds, **libcurl** is called to
 * download files.  Files are downloaded to each build's subdirectory
 * in the program working directory.  This function gathers both
 * before and after builds if specified at run time.
 *
 * @param ri The main program data structure; contains the before and
 *        after build specifications from the command line.
 * @param fo True if '-f' (fetch only) specified, false otherwise.
 * @return 0 on success, non-zero on failure (program exit code).
 */
int gather_builds(struct rpminspect *ri, bool fo)
{
    int r = 0;

    assert(ri != NULL);
    assert(ri->after != NULL);

    workri = ri;
    fetch_only = fo;

    /* process after first so the temp directory gets the NV of that pkg */
    if (ri->after != NULL) {
        whichbuild = AFTER_BUILD;
        r = _gather_build_types(ri);

        if (r) {
            return r;
        }
    }

    /* did we get a before build specified? */
    if (ri->before == NULL) {
        return extract_peers(ri, fo);
    }

    whichbuild = BEFORE_BUILD;
    r = _gather_build_types(ri);

    if (r) {
        return r;
    }

    /*
     * init the arches list if the user did not specify it (we have
     * builds now)
     */
    init_arches(ri);

    /*
     * extract the RPMs
     */
    return extract_peers(ri, fo);
}
