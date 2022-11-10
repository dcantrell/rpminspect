/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <wordexp.h>
#include <regex.h>
#include <zlib.h>
#include <magic.h>
#include <clamav.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmmacro.h>
#include "rpminspect.h"

void sigabrt_handler(__attribute__ ((unused)) int i)
{
    rpmFreeRpmrc();
    return;
}

void sigwinch_handler(__attribute__ ((unused)) int i)
{
    terminal_resized = 1;
    return;
}

static void usage(void)
{
    printf(_("Compare package builds for policy compliance and consistency.\n\n"));
    printf(_("Usage: %s [OPTIONS] [before build] [after build]\n"), COMMAND_NAME);
    printf(_("Options:\n"));
    printf(_("  -c FILE, --config=FILE      Configuration file to use\n"));
    printf(_("  -p NAME, --profile=NAME     Configuration profile to use\n"));
    printf(_("  -T LIST, --tests=LIST       List of tests to run\n"));
    printf(_("                                (default: ALL)\n"));
    printf(_("  -E LIST, --exclude=LIST     List of tests to exclude\n"));
    printf(_("                                (default: none)\n"));
    printf(_("  -a LIST, --arches=LIST      List of architectures to check\n"));
    printf(_("  -r STR, --release=STR       Product release string\n"));
    printf(_("  -n, --no-rebase             Disable build rebase detection\n"));
    printf(_("  -b TYPE, --build-type=TYPE  Set Koji build type to TYPE\n"));
    printf(_("  -o FILE, --output=FILE      Write results to FILE\n"));
    printf(_("                                (default: stdout)\n"));
    printf(_("  -F TYPE, --format=TYPE      Format output results as TYPE\n"));
    printf(_("                                (default: text)\n"));
    printf(_("  -t TAG, --threshold=TAG     Result threshold triggering exit\n"));
    printf(_("                              failure (default: VERIFY)\n"));
    printf(_("  -s TAG, --suppress=TAG      Results suppression threshold\n"));
    printf(_("                                (default: off, report everything)\n"));
    printf(_("  -l, --list                  List available tests and formats\n"));
    printf(_("  -w PATH, --workdir=PATH     Temporary directory to use\n"));
    printf(_("                                (default: %s)\n"), DEFAULT_WORKDIR);
    printf(_("  -f, --fetch-only            Fetch builds only, do not perform inspections\n"));
    printf(_("                                (implies -k)\n"));
    printf(_("  -k, --keep                  Do not remove the comparison working files\n"));
    printf(_("  -d, --debug                 Debugging mode output\n"));
    printf(_("  -D, --dump-config           Dump configuration settings (in YAML format)\n"));
    printf(_("  -v, --verbose               Verbose inspection output\n"));
    printf(_("                              when finished, display full path\n"));
    printf(_("  -?, --help                  Display usage information\n"));
    printf(_("  -V, --version               Display program version\n"));
    printf(_("\nSee the rpminspect(1) man page for more information.\n"));

    return;
}

/*
 * Match the candidate product string to a product string rule.
 * Caller must free the returned string.
 */
static char *match_product(string_map_t *products, const char *candidate)
{
    char *ret = NULL;
    bool matched = false;
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;
    regex_t product_regex;
    char reg_error[BUFSIZ];
    regmatch_t matches[1];
    int result;

    assert(candidate != NULL);

    if (products == NULL) {
        return strdup(candidate);
    }

    /* Try to see if a product mapping matches our strings */
    HASH_ITER(hh, products, hentry, tmp_hentry) {
        /* build a regex for this product release string */
        result = regcomp(&product_regex, hentry->value, REG_EXTENDED);

        if (result != 0) {
            regerror(result, &product_regex, reg_error, sizeof(reg_error));
            warnx(_("*** unable to compile product release regular expression: %s"), reg_error);
            return NULL;
        }

        /* now try to match the candidate */
        result = regexec(&product_regex, candidate, 1, matches, 0);

        if (result != 0) {
            regfree(&product_regex);
            continue;
        }

        if (matches[0].rm_so > -1) {
            matched = true;
            ret = strdup(hentry->key);
        }

        regfree(&product_regex);

        if (matched) {
            break;
        }
    }

    return ret;
}

/*
 * Get the product release string by grabbing a possible dist tag from
 * the Release value.  Dist tags begin with '.' and go to the end of the
 * Release value.  Trim any trailing '/' characters in case the user is
 * specifying a build from a local path.
 */
static char *get_product_release(string_map_t *products, const favor_release_t favor_release, const char *before, const char *after)
{
    int c = -1;
    size_t i = 0;
    char *r = NULL;
    char *before_candidate = NULL;
    char *after_candidate = NULL;
    char *before_product = NULL;
    char *after_product = NULL;
    bool matched = false;

    assert(after != NULL);

    /* skip up to the dist tag */
    while (*after != '.' && *after != '\0') {
        after++;
    }

    if (!after) {
        warnx(_("*** Product release for after build (%s) is empty"), after);
        return NULL;
    }

    /*
     * Get the character after the last occurrence of a period. This should
     * tell us what release flag the product is.
     */
    after_candidate = strdup(after);

    if (after_candidate == NULL) {
        warnx(_("*** Product release for after build (%s) is empty"), after);
        return NULL;
    }

    /*
     * Trim any trailing slashes in case the user is specifying builds from
     * local paths
     */
    after_candidate[strcspn(after_candidate, "/")] = 0;

    if (before) {
        /* skip up to the dist tag */
        while (*before != '.' && *before != '\0') {
            before++;
        }

        if (!before) {
            warnx(_("*** Product release for before build (%s) is empty"), before);
            free(after_candidate);
            return NULL;
        }

        before_candidate = strdup(before);

        if (before_candidate == NULL) {
            warnx(_("*** Product release for before build (%s) is empty"), before);
            free(after_candidate);
            return NULL;
        }

        /*
         * Trim any trailing slashes in case the user is specifying builds from
         * local paths
         */
        before_candidate[strcspn(before_candidate, "/")] = 0;

        /*
         * If builds are different and we have no products hash table, fail
         */
        after_product = match_product(products, after_candidate);
        before_product = match_product(products, before_candidate);

        if (before_product && after_product) {
            c = strcmp(before_product, after_product);
        }

        if (c) {
            matched = false;
        } else if (!c) {
            matched = true;
        }
    } else {
        after_product = match_product(products, after_candidate);
        matched = true;
    }

    free(before_candidate);
    free(after_candidate);

    /* Handle release favoring */
    if (!matched) {
        if (before_product && after_product && favor_release != FAVOR_NONE) {
            c = strverscmp(before_product, after_product);
            matched = true;

            if (favor_release == FAVOR_OLDEST && c <= 0) {
                free(after_product);
                after_product = before_product;
                before_product = NULL;
            } else if (favor_release == FAVOR_NEWEST && c >= 0) {
                free(after_product);
                after_product = before_product;
                before_product = NULL;
            }
        } else if (before_product == NULL && after_product) {
            /* use after product since we got nothing from the before */
            matched = true;
        } else if (before_product && after_product == NULL) {
            /* use before product since we got nothing from the after */
            after_product = before_product;
            before_product = NULL;
            matched = true;
        }
    }

    if (!matched) {
        warnx(_("*** Unable to determine product release for %s and %s"), before, after);
        warnx(_("*** See the 'favor_release' setting in the rpminspect configuration file."));
        after_product = NULL;
    }

    free(before_product);

    /* trim the leading period */
    i = 0;

    while (after_product[i] == '.' && after_product[i] != '\0') {
        i++;
    }

    if (i < strlen(after_product)) {
        r = strdup(after_product + i);
        assert(r != NULL);
    }

    free(after_product);

    return r;
}

/*
 * Used to ensure the user only specifies the -T or -E option.
 */
static void check_inspection_options(const bool inspection_opt)
{
    if (inspection_opt) {
        warnx(_("*** The -T and -E options are mutually exclusive"));
        errx(RI_PROGRAM_ERROR, _("*** See `%s --help` for more information."), COMMAND_NAME);
    }

    return;
}

/*
 * Used in the -T and -E option processing to report any unknown
 * test names provided.  Exit if true.
 */
static void check_found(const bool found, const char *inspection)
{
    assert(inspection != NULL);

    if (!found) {
        warnx(_("*** Unknown inspection: `%s`"), inspection);
        errx(RI_PROGRAM_ERROR, _("*** See `%s --help` for more information."), COMMAND_NAME);
    }

    return;
}

int main(int argc, char **argv)
{
    struct sigaction abrt;
    struct sigaction winch;
    int c = 0;
    int i = 0;
    int j = 0;
    int idx = 0;
    int ret = RI_SUCCESS;
    wordexp_t expand;
    struct stat sb;
    char *short_options = "c:p:T:E:a:r:nb:o:F:lw:t:s:fkdDv\?V";
    struct option long_options[] = {
        { "config", required_argument, 0, 'c' },
        { "profile", required_argument, 0, 'p' },
        { "tests", required_argument, 0, 'T' },
        { "exclude", required_argument, 0, 'E' },
        { "arches", required_argument, 0, 'a' },
        { "release", required_argument, 0, 'r' },
        { "no-rebase", no_argument, 0, 'n' },
        { "build-type", required_argument, 0, 'b' },
        { "list", no_argument, 0, 'l' },
        { "output", required_argument, 0, 'o' },
        { "format", required_argument, 0, 'F' },
        { "workdir", required_argument, 0, 'w' },
        { "threshold", required_argument, 0, 't' },
        { "suppress", required_argument, 0, 's' },
        { "fetch-only", no_argument, 0, 'f' },
        { "keep", no_argument, 0, 'k' },
        { "debug", no_argument, 0, 'd' },
        { "dump-config", no_argument, 0, 'D' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, '?' },
        { "version", no_argument, 0, 'V' },
        { 0, 0, 0, 0 }
    };
    char *cfgfile = NULL;
    char *tmp_cfgfile = NULL;
    char *profile = NULL;
    bool initialized = false;
    char *archopt = NULL;
    char *walk = NULL;
    char *token = NULL;
    char *cwd = NULL;
    char *r = NULL;
    char *output = NULL;
    char *release = NULL;
    bool rebase_detection = true;
    koji_build_type_t buildtype = KOJI_BUILD_NULL;
    char *threshold = NULL;
    char *suppress = NULL;
    int formatidx = -1;
    bool fetch_only = false;
    bool keep = false;
    bool list = false;
    bool verbose = false;
    bool dump_config = false;
    int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    bool found = false;
    char *inspection = NULL;
    const char *desc = NULL;
    char *insoptarg = NULL;
    char *tmp = NULL;
    bool inspection_opt = false;
    bool exclude = false;
    size_t width = tty_width();
    string_list_t *valid_arches = NULL;
    string_entry_t *arch = NULL;
    rpmpeer_entry_t *peer = NULL;
    const char *after_rel = NULL;
    const char *before_rel = NULL;
    char *hsz = NULL;
    struct result_params params;
    size_t cmdlen = 0;
    char *tail = NULL;
    bool ires = false;
    string_list_t *diags = NULL;
    struct rpminspect *ri = NULL;

    /* Be friendly to "rpminspect ... 2>&1 | tee" use case */
    setlinebuf(stdout);

    /* SIGABRT handler since we use abort() in some failure cases */
    abrt.sa_handler = sigabrt_handler;
    sigemptyset(&abrt.sa_mask);
    abrt.sa_flags = 0;
    sigaction(SIGABRT, &abrt, NULL);

    /* SIGWINCH handler to capture terminal resizes */
    winch.sa_handler = sigwinch_handler;
    sigemptyset(&winch.sa_mask);
    winch.sa_flags = 0;
    sigaction(SIGWINCH, &winch, NULL);

    /* Set up the i18n environment */
    setlocale(LC_ALL, "");
    bindtextdomain("rpminspect", "/usr/share/locale/");
    textdomain("rpminspect");

    /* parse command line options */
    while (1) {
        c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'c':
                /* Capture user specified config file */
                cfgfile = strdup(optarg);
                break;
            case 'p':
                /* Configuration profile to use */
                profile = strdup(optarg);
                break;
            case 'T':
                /* Inspections to enable */
                check_inspection_options(inspection_opt);
                insoptarg = strdup(optarg);
                exclude = false;
                inspection_opt = true;
                break;
            case 'E':
                /* Inspections to disable */
                check_inspection_options(inspection_opt);
                insoptarg = strdup(optarg);
                exclude = true;
                inspection_opt = true;
                break;
            case 'a':
                archopt = strdup(optarg);
                break;
            case 'r':
                release = strdup(optarg);
                break;
            case 'n':
                rebase_detection = false;
                break;
            case 'b':
                /* validate the specified build type */
                for (i = 0; buildtypes[i].type != KOJI_BUILD_NULL; i++) {
                    if (!strcasecmp(buildtypes[i].name, optarg)) {
                        if (!buildtypes[i].supported) {
                            errx(RI_PROGRAM_ERROR, _("*** Unsupported build type: `%s`."), optarg);
                        }

                        buildtype = buildtypes[i].type;
                        break;
                    }
                }

                if (buildtype == KOJI_BUILD_NULL) {
                    errx(RI_PROGRAM_ERROR, _("*** Invalid build type: `%s`."), optarg);
                }

                break;
            case 'o':
                output = strdup(optarg);
                break;
            case 'F':
                /* validate the specified output format */
                for (i = 0; formats[i].type != -1; i++) {
                    if (!strcasecmp(formats[i].name, optarg)) {
                        formatidx = formats[i].type;
                        break;
                    }
                }

                if (formatidx < 0) {
                    errx(RI_PROGRAM_ERROR, _("*** Invalid output format: `%s`."), optarg);
                }

                break;
            case 'l':
                list = true;
                break;
            case 'w':
                /* allow for ~ expansion and other shell stuff */
                j = wordexp(optarg, &expand, 0);

                if (j != 0 || expand.we_wordc != 1) {
                    errx(RI_PROGRAM_ERROR, _("*** Unable to expand workdir: `%s`"), optarg);
                }

                if (stat(expand.we_wordv[0], &sb) == 0) {
                    if (cwd) {
                        /* we may have been called with multiple -w arguments, take the last one */
                        free(cwd);
                        cwd = NULL;
                    }

                    cwd = realpath(expand.we_wordv[0], NULL);
                    assert(cwd != NULL);
                }

                wordfree(&expand);
                break;
            case 't':
                threshold = strdup(optarg);
                break;
            case 's':
                suppress = strdup(optarg);
                break;
            case 'f':
                fetch_only = true;        /* -f implies -k */
                /* fall through */
            case 'k':
                keep = true;
                break;
            case 'd':
                set_debug_mode(true);
                break;
            case 'D':
                dump_config = true;
                break;
            case 'v':
                verbose = true;
                break;
            case '?':
                usage();
                exit(0);
            case 'V':
                printf(_("%s version %s\n"), COMMAND_NAME, PACKAGE_VERSION);
                exit(0);
            default:
                errx(RI_PROGRAM_ERROR, _("?? getopt returned character code 0%o ??"), c);
        }
    }

    /* list inspections and formats and exit if asked to */
    if (list) {
        /* list the available build types */
        printf(_("Available build types:\n"));

        for (i = 0; buildtypes[i].type != KOJI_BUILD_NULL; i++) {
            if (!buildtypes[i].supported) {
                continue;
            }

            if (i > 0 && verbose) {
                printf("\n");
            }

            printf("    %s\n", buildtypes[i].name);
            desc = buildtype_desc(buildtypes[i].type);

            if (desc != NULL && verbose) {
                printwrap(desc, width, 8, stdout);
                printf("\n");
            }
        }

        /* list the formats available */
        printf(_("\nAvailable output formats:\n"));

        for (i = 0; formats[i].type != -1; i++) {
            if (i > 0 && verbose) {
                printf("\n");
            }

            printf("    %s\n", formats[i].name);
            desc = format_desc(formats[i].type);

            if (desc != NULL && verbose) {
                printwrap(desc, width, 8, stdout);
                printf("\n");
            }
        }

        /* list the inspections available */
        printf(_("\nAvailable inspections:\n"));

        for (i = 0; inspections[i].name != NULL; i++) {
            if (i > 0 && verbose) {
                printf("\n");
            }

            printf("    %s\n", inspections[i].name);
            desc = inspection_desc(inspections[i].flag);

            if (desc != NULL && verbose) {
                printwrap(desc, width, 8, stdout);
                printf("\n");
            }
        }

        exit(RI_SUCCESS);
    }

    /* Set up the main program data structure */
    ri = calloc_rpminspect(ri);
    ri->progname = strdup(argv[0]);
    ri->verbose = verbose;
    ri->rebase_detection = rebase_detection;

    /*
     * Find an appropriate configuration file. This involves:
     *
     *  - Using the user-passed value and sanity-checking it,
     *  - Using the global default if it exists, or
     *  - Telling the user they need to install a required dependency.
     *
     * This loop also handles reading in multiple configuration files
     * for overrides.
     */
    xasprintf(&tmp_cfgfile, "%s/%s", CFGFILE_DIR, CFGFILE);

    if (cfgfile == NULL && (access(tmp_cfgfile, F_OK|R_OK) == 0)) {
        /* /usr/share/rpminspect/rpminspect.yaml exists */
        ri = init_rpminspect(ri, tmp_cfgfile, profile);
        initialized = true;

        if (ri == NULL) {
            errx(RI_PROGRAM_ERROR, _("Failed to read configuration file %s"), tmp_cfgfile);
        }
    } else if (access(cfgfile, F_OK|R_OK) == 0) {
        /* -c configuration file if it exists */
        ri = init_rpminspect(ri, cfgfile, profile);
        initialized = true;

        if (ri == NULL) {
            errx(RI_PROGRAM_ERROR, _("Failed to read configuration file %s"), cfgfile);
        }
    }

    free(tmp_cfgfile);
    free(cfgfile);

    /*
     * Failsafe to try and load a config file from the current
     * directory if we have not been able to load any already.  If we
     * did initialize, then that process would have also loaded this
     * file from the current directory.
     */
    if (!initialized && access(CFGFILE, F_OK|R_OK) == 0) {
        ri = init_rpminspect(ri, CFGFILE, profile);
        initialized = true;

        if (ri == NULL) {
            errx(RI_PROGRAM_ERROR, _("Failed to read configuration file %s"), CFGFILE);
        }
    }

    if (!initialized) {
        errx(RI_PROGRAM_ERROR, _("Please specify a configuration file using '-c' or supply ./%s"), CFGFILE);
    }

    free(profile);

    /* Product release specified on the command line overrides config file */
    if (release) {
        free(ri->product_release);
        ri->product_release = release;
    }

    /* Koji build type may have been specified */
    ri->buildtype = buildtype;

    /* Reporting threshold and suppression levels */
    ri->threshold = getseverity(threshold, RESULT_VERIFY);
    ri->suppress = getseverity(suppress, RESULT_NULL);
    free(threshold);
    free(suppress);

    /*
     * any inspection selections on the command line can override
     * selections made via the config files
     */
    if (inspection_opt) {
        if (exclude) {
            ri->tests = ~0;
        } else {
            ri->tests = 0;
        }

        tmp = insoptarg;

        while ((inspection = strsep(&insoptarg, ",")) != NULL) {
            found = process_inspection_flag(inspection, exclude, &ri->tests);
            check_found(found, inspection);
        }

        free(tmp);
    }

    /* Handle user-specified working directory */
    if (cwd == NULL && fetch_only) {
        /* no workdir specified, but fetch only requested, default to cwd */
        cwd = getcwd(NULL, 0);
        assert(cwd != NULL);
    }

    if (cwd != NULL) {
        /* the user specified a working directory */
        free(ri->workdir);
        ri->workdir = cwd;
    }

    /* Display the configuration settings for this run */
    if (dump_config) {
        dump_cfg(ri);
    }

    /*
     * We should exactly one more argument (single build) or two arguments
     * (a before and after build).  Except for fetch-only we can take a
     * list of builds.
     */
    if (!fetch_only) {
        if (optind == (argc - 1)) {
            /* only a single build specified */
            ri->after = strdup(argv[optind]);
        } else if ((optind + 1) == (argc - 1)) {
            /* we got a before and after build */
            ri->before = strdup(argv[optind]);
            ri->after = strdup(argv[optind + 1]);
        } else {
            free_rpminspect(ri);

            if (dump_config) {
                return RI_SUCCESS;
            } else {
                warnx(_("*** Invalid before and after build specification."));
                errx(RI_PROGRAM_ERROR, _("*** See `%s --help` for more information."), COMMAND_NAME);
            }
        }
    }

    /* initialize librpm, we'll be using it */
    if (init_librpm(ri) != RPMRC_OK) {
        errx(RI_PROGRAM_ERROR, _("*** unable to read RPM configuration"));
    }

    /* load macros for librpm */
    load_macros(ri);

    /* if an architecture list is specified, validate it */
    if (archopt) {
        /* get a list of valid architectures */
        valid_arches = get_all_arches(ri);
        assert(valid_arches != NULL);

        /* collect the specified architectures */
        walk = archopt;

        while ((token = strsep(&walk, ",")) != NULL) {
            /* if the architecture is invalid, report it and exit */
            found = false;

            TAILQ_FOREACH(arch, valid_arches, items) {
                if (!strcmp(token, arch->data)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                free_rpminspect(ri);
                rpmFreeMacros(NULL);
                rpmFreeRpmrc();
                warnx(_("*** Unsupported architecture specified: `%s`"), token);
                errx(RI_PROGRAM_ERROR, _("*** See `%s --help` for more information."), COMMAND_NAME);
            }

            /* architecture is valid, save it */
            ri->arches = list_add(ri->arches, token);
        }

        /* clean up */
        free(archopt);
        list_free(valid_arches, free);
    }

    /* create the working directory */
    if (mkdirp(ri->workdir, mode)) {
        free_rpminspect(ri);
        rpmFreeMacros(NULL);
        rpmFreeRpmrc();
        errx(RI_PROGRAM_ERROR, _("*** Unable to create directory %s"), ri->workdir);
    }

    /* validate and gather the builds specified */
    if (fetch_only) {
        /* iterate over each specified build and fetch it */
        for (i = optind; i < argc; i++) {
            ri->after = strdup(argv[i]);
            assert(ri->after != NULL);
            j = gather_builds(ri, true);

            if (j) {
                free_rpminspect(ri);
                rpmFreeMacros(NULL);
                rpmFreeRpmrc();

                if (j > 0) {
                    errx(j, "%s", strexitcode(j));
                } else {
                    exit(j);
                }
            }

            free(ri->after);
            ri->after = NULL;
            free(ri->worksubdir);
            ri->worksubdir = NULL;
        }

        free_rpminspect(ri);
        rpmFreeMacros(NULL);
        rpmFreeRpmrc();
        return RI_SUCCESS;
    } else {
        j = gather_builds(ri, false);

        if (j) {
            free_rpminspect(ri);
            rpmFreeMacros(NULL);
            rpmFreeRpmrc();

            if (j > 0) {
                errx(j, "%s", strexitcode(j));
            } else {
                exit(j);
            }
        }
    }

    /* general information in the results */
    init_result_params(&params);
    params.severity = RESULT_DIAG;
    params.header = NAME_DIAGNOSTICS;

    /* gather version information for dependent programs and libraries */
    diags = gather_diags(ri, COMMAND_NAME, PACKAGE_VERSION);

    /* add version information to the results output */
    xasprintf(&params.msg, _("Version information for libraries and programs used by %s as well as storage requirements.  This result is for informational and diagnostic purposes only."), COMMAND_NAME);
    params.details = list_to_string(diags, "\n");
    params.details = strappend(params.details, "\n\n", NULL);

    /* add disk space requirements */
    hsz = human_size(ri->download_size);
    assert(hsz != NULL);
    xasprintf(&tmp, _("Space required to download artifacts: %lu bytes (%s)\n"), ri->download_size, hsz);
    assert(tmp != NULL);
    params.details = strappend(params.details, tmp, NULL);
    free(tmp);
    free(hsz);

    hsz = human_size(ri->unpacked_size);
    assert(hsz != NULL);
    xasprintf(&tmp, _("Space required to unpack artifacts: %lu bytes (%s)\n"), ri->unpacked_size, hsz);
    assert(tmp != NULL);
    params.details = strappend(params.details, tmp, NULL);
    free(tmp);
    free(hsz);

    add_result_entry(&ri->results, &params);
    free(params.msg);
    free(params.details);
    list_free(diags, free);

    /* add command line information to the results output */
    xasprintf(&params.msg, _("Command line arguments used to invoke %s."), COMMAND_NAME);

    for (i = 0; i < argc; i++) {
        cmdlen += strlen(argv[i]) + 1;
    }

    params.details = calloc(1, cmdlen + 1);
    assert(params.details != NULL);
    tail = params.details;

    for (i = 0; i < argc; i++) {
        if (i != 0) {
            tail = stpcpy(tail, " ");
        }

        tail = stpcpy(tail, argv[i]);
    }

    add_result_entry(&ri->results, &params);
    free(params.msg);
    free(params.details);

    /* make sure the worst result is set before running inspections */
    ri->worst_result = params.severity;

    /* perform the selected inspections */
    if (!fetch_only) {
        /* Determine product release unless the user specified one. */
        if (ri->product_release == NULL) {
            if (ri->peers == NULL || TAILQ_EMPTY(ri->peers)) {
                free_rpminspect(ri);
                rpmFreeMacros(NULL);
                rpmFreeRpmrc();
                errx(RI_PROGRAM_ERROR, _("*** No peers, ensure packages exist for specified architecture(s)."));
            }

            /* try to find a before and after peer */
            TAILQ_FOREACH(peer, ri->peers, items) {
                after_rel = headerGetString(peer->after_hdr, RPMTAG_RELEASE);

                if (ri->before) {
                    before_rel = headerGetString(peer->before_hdr, RPMTAG_RELEASE);
                }

                if (before_rel && after_rel) {
                    break;
                }
            }

            /* if we got here with no before and after release values, bad */
            if ((ri->before && before_rel == NULL) && after_rel == NULL) {
                free_rpminspect(ri);
                rpmFreeMacros(NULL);
                rpmFreeRpmrc();
                errx(RI_PROGRAM_ERROR, _("unable to find a set of peer packages between the before and after builds"));
            }

            /* get the product release */
            if (ri->product_release == NULL) {
                ri->product_release = get_product_release(ri->products, ri->favor_release, before_rel, after_rel);
            }

            DEBUG_PRINT("product_release=%s\n", ri->product_release);

            if (ri->product_release == NULL) {
                free_rpminspect(ri);
                rpmFreeMacros(NULL);
                rpmFreeRpmrc();
                errx(RI_PROGRAM_ERROR, _("*** Unable to determine product release or none specified (-r)."));
            }
        }

        for (i = 0; inspections[i].name != NULL; i++) {
            /* test not selected by user */
            if (!(ri->tests & inspections[i].flag)) {
                /*
                 * tell the user this inspection is skipped when in
                 * verbose mode
                 */
                if (verbose) {
                    xasprintf(&r, _("Skipping %s inspection..."), inspections[i].name);
                    assert(r != NULL);
                    printf("%-36s", r);
                    free(r);

                    printf("%5s\n", _("skip"));
                }

                /* add a skipped result for this inspection */
                init_result_params(&params);
                params.header = inspections[i].name;
                params.severity = RESULT_SKIP;
                params.verb = VERB_SKIP;
                add_result(ri, &params);

                /* next inspection */
                continue;
            }

            /* inspection requires before/after builds and we have one */
            if (ri->before == NULL && !inspections[i].single_build) {
                continue;
            }

            if (verbose) {
                xasprintf(&r, _("Running %s inspection..."), inspections[i].name);
                assert(r != NULL);
                printf("%-36s", r);
                free(r);
            }

            ires = inspections[i].driver(ri);

            if (verbose) {
                printf("%5s\n", ires ? _("pass") : _("FAIL"));
            }
        }

        if (verbose) {
            printf("\n");
        }

        /* output the results */
        if (formatidx == -1) {
            formatidx = 0;                 /* default to 'text' output */
        }

        if (ri->results != NULL) {
            formats[formatidx].driver(ri->results, output, ri->threshold, ri->suppress);
        }

        free(output);
    }

    /* Set exit code based on result threshold */
    if (ri->worst_result >= ri->threshold) {
        ret = RI_INSPECTION_FAILURE;
    }

    /* Clean up */
    if (!fetch_only) {
        if (keep) {
            printf(_("\nKeeping working directory: %s\n"), ri->worksubdir);
        } else {
            /* remove the working directories we can */
            (void) rmtree(ri->worksubdir, true, false);
        }
    }

    free_rpminspect(ri);
    rpmFreeMacros(NULL);
    rpmFreeRpmrc();

    return ret;
}
