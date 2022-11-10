/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <yaml.h>

#include "rpminspect.h"

/* Globals */
static bool static_context = false;

/*
 * Called by nftw() to find and read /data/static_context from modulemd.txt
 */
static int read_modulemd(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    int r = 0;
    char *bn = NULL;
    yaml_parser_t parser;
    yaml_token_t token;
    char *value = NULL;
    int in_data = 0;
    FILE *fp = NULL;

    if (tflag != FTW_F) {
        return 0;
    }

    bn = basename(fpath);
    static_context = false;

    /* try to read this file */
    if (!strcmp(bn, MODULEMD_FILENAME)) {
        /* prepare a YAML parser */
        if (!yaml_parser_initialize(&parser)) {
            warn("yaml_parser_initializer");
        }

        /* open the modulemd file */
        if ((fp = fopen(fpath, "r")) == NULL) {
            err(RI_PROGRAM_ERROR, "fopen");
        }

        /* tell the YAML parser to read the modulemd file */
        yaml_parser_set_input_file(&parser, fp);

        /* Loop over the YAML file looking for /data/static_context */
        do {
            if (yaml_parser_scan(&parser, &token) == 0) {
                warnx(_("ignoring malformed module metadata file: %s"), fpath);
                return -1;
            }

            value = (char *) token.data.scalar.value;

            switch (token.type) {
                case YAML_SCALAR_TOKEN:
                    if ((in_data == 0) && !strcmp(value, "data")) {
                        in_data++;
                    } else if ((in_data == 1) && !strcmp(value, "static_context")) {
                        in_data++;
                    } else if (in_data == 2) {
                        if (!strcasecmp(value, "true")) {
                            static_context = true;
                        }
                    }

                    break;
                case YAML_BLOCK_END_TOKEN:
                    in_data = 0;
                    break;
                default:
                    break;
            }

            if (token.type != YAML_STREAM_END_TOKEN) {
                yaml_token_delete(&token);
            }

            if (static_context) {
                r = 1;
                break;
            }
        } while (token.type != YAML_STREAM_END_TOKEN);

        /* destroy the YAML parser, close the input file */
        yaml_parser_delete(&parser);

        if (fclose(fp) != 0) {
            err(RI_PROGRAM_ERROR, "fclose");
        }
    }

    return r;
}

/*
 * Given a directory and a build subdirectory, construct a new full
 * path and walk that tree looking for the modulemd.txt file and then
 * reading /data/static_context from it.
 */
static bool get_static_context(const char *subdir, const char *build)
{
    char *path = NULL;

    assert(subdir != NULL);
    assert(build != NULL);

    /* create the build path */
    path = joinpath(subdir, build, NULL);
    assert(path != NULL);

    /* find the modulemd.txt file and read /data/static_context */
    static_context = false;

    if (nftw(path, read_modulemd, FOPEN_MAX, FTW_PHYS) == -1) {
        warn("nftw");
        free(path);
        return false;
    }

    /* cleanup */
    free(path);

    return static_context;
}

/*
 * Read the /data/static_context value from the modulemd.txt files and
 * validate them against the rules.
 */
static bool check_static_context(struct rpminspect *ri)
{
    bool r = true;
    bool bsc = false;
    bool asc = false;
    struct result_params params;

    assert(ri != NULL);

    /* Set up the result parameters */
    init_result_params(&params);
    params.header = NAME_MODULARITY;
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;

    /* get the after build static_context first */
    asc = get_static_context(ri->worksubdir, AFTER_SUBDIR);

    if (ri->before) {
        /* comparing builds, verify after build is correct, but report changes */
        bsc = get_static_context(ri->worksubdir, BEFORE_SUBDIR);

        if (bsc == asc) {
            if (ri->modularity_static_context == STATIC_CONTEXT_FORBIDDEN) {
                xasprintf(&params.msg, _("The /data/static_context value in %s matches the value in %s, but the product release rules forbid the presence of /data/static_context in the module metadata."), ri->before, ri->after);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
                r = false;
            } else if (ri->modularity_static_context == STATIC_CONTEXT_REQUIRED) {
                xasprintf(&params.msg, _("The /data/static_context value in %s matches the value in %s, and the product release rules require the presence of /data/static_context in the module metadata."), ri->before, ri->after);
            } else if (ri->modularity_static_context == STATIC_CONTEXT_RECOMMEND) {
                xasprintf(&params.msg, _("The /data/static_context value in %s matches the value in %s, and the product release rules recommend the presence of /data/static_context in the module metadata."), ri->before, ri->after);
            } else if (ri->modularity_static_context == STATIC_CONTEXT_NULL) {
                xasprintf(&params.msg, _("The /data/static_context value in %s matches the value in %s, but the product release rules do not have a setting for /data/static_context in the module metadata."), ri->before, ri->after);
            }
        } else if (bsc != asc) {
            if (asc && ri->modularity_static_context == STATIC_CONTEXT_FORBIDDEN) {
                xasprintf(&params.msg, _("The /data/static_context value in %s was %s and became %s in %s, but the product release rules forbid the presence of /data/static_context in the module metadata."), ri->before, bsc ? _("true") : _("false"), asc ? _("true") : _("false"), ri->after);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
                r = false;
            } else if (!asc && ri->modularity_static_context == STATIC_CONTEXT_REQUIRED) {
                xasprintf(&params.msg, _("The /data/static_context value in %s was %s and became %s in %s, but the product release rules require the presence of /data/static_context in the module metadata."), ri->before, bsc ? _("true") : _("false"), asc ? _("true") : _("false"), ri->after);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
                r = false;
            } else if (!asc && ri->modularity_static_context == STATIC_CONTEXT_RECOMMEND) {
                xasprintf(&params.msg, _("The /data/static_context value in %s was %s and became %s in %s, and the product release rules recommend the presence of /data/static_context in the module metadata."), ri->before, bsc ? _("true") : _("false"), asc ? _("true") : _("false"), ri->after);
            } else if (ri->modularity_static_context == STATIC_CONTEXT_NULL) {
                xasprintf(&params.msg, _("The /data/static_context value in %s was %s and became %s in %s, but the product release rules do not have a setting for /data/static_context in the module metadata."), ri->before, bsc ? _("true") : _("false"), asc ? _("true") : _("false"), ri->after);
            }
        }
    } else {
        if (asc && ri->modularity_static_context == STATIC_CONTEXT_FORBIDDEN) {
            xasprintf(&params.msg, _("The /data/static_context value in %s is %s, but the product release rules forbid the presence of /data/static_context in the module metadata."), ri->after, asc ? _("true") : _("false"));
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
            r = false;
        } else if (!asc && ri->modularity_static_context == STATIC_CONTEXT_REQUIRED) {
            xasprintf(&params.msg, _("The /data/static_context value in %s is %s, but the product release rules require the presence of /data/static_context in the module metadata."), ri->after, asc ? _("true") : _("false"));
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_MODULARITY_STATIC_CONTEXT;
            r = false;
        } else if (!asc && ri->modularity_static_context == STATIC_CONTEXT_RECOMMEND) {
            xasprintf(&params.msg, _("The /data/static_context value in %s is %s, and the product release rules recommend the presence of /data/static_context in the module metadata."), ri->after, asc ? _("true") : _("false"));
        } else if (ri->modularity_static_context == STATIC_CONTEXT_NULL) {
            xasprintf(&params.msg, _("The /data/static_context value in %s is %s, but the product release rules do not have a setting for /data/static_context in the module metadata."), ri->after, asc ? _("true") : _("false"));
        }
    }

    /* report */
    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
    }

    return r;
}

static bool modularity_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    rpmTagType tt;
    rpmTagVal tv;
    const char *modularitylabel = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_BAD;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_MODULARITY;
    params.remedy = REMEDY_MODULARITY;

    /* Build the message we'll use for errors */
    xasprintf(&params.msg, _("Package \"%s\" is part of a module but lacks the '%%{modularitylabel}' header tag."), headerGetString(file->rpm_header, RPMTAG_NAME));

    /* Find how to find the header */
    tv = rpmTagGetValue("modularitylabel");
    if (tv == -1) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    tt = rpmTagGetTagType(tv);
    if (tt == RPM_NULL_TYPE) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /* Get the tag from the header */
    modularitylabel = headerGetString(file->rpm_header, tv);

    if (modularitylabel == NULL) {
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    free(params.msg);
    return result;
}

/*
 * Main driver for the 'modularity' inspection.
 */
bool inspect_modularity(struct rpminspect *ri)
{
    bool tag_result = false;
    bool static_context_result = false;
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    if (ri->buildtype != KOJI_BUILD_MODULE) {
        init_result_params(&params);
        xasprintf(&params.msg, _("Inspection skipped because this build's type is not `module'."));
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_MODULARITY;
        add_result(ri, &params);
        free(params.msg);
        return true;
    }

    /* check each RPM for the modularitylabel tag */
    tag_result = foreach_peer_file(ri, NAME_MODULARITY, modularity_driver);

    /* check static context against static context rule */
    static_context_result = check_static_context(ri);

    /* final check of all results */
    result = tag_result && static_context_result;

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_MODULARITY;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
