/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <regex.h>
#include <rpm/header.h>
#include "queue.h"
#include "rpminspect.h"

/* For reporting */
static const char *specfile = NULL;
static char *pkg_evr = NULL;
static char *pkg_vr = NULL;
static uint64_t pkg_epoch = 0;

/*
 * Given a package name, return true if this is a valid subpackage in
 * the current build.
 */
static bool is_subpackage(rpmpeer_t *peers, const char *package)
{
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;

    assert(peers != NULL);
    assert(package != NULL);

    TAILQ_FOREACH(peer, peers, items) {
        name = headerGetString(peer->after_hdr, RPMTAG_NAME);
        assert(name != NULL);

        if (!strcmp(name, package)) {
            return true;
        }
    }

    return false;
}

/*
 * Given a requirement string, remove any %{_isa} substring in it.
 * Returns a newly allocated string.  The %{_isa} substrings are all
 * defined in the macros files found in /usr/lib/rpm/platform/ and are
 * of the form "(text-32)" or "(text-64)".  Rather than list them all
 * out here, this function just uses a regular expression to find a
 * substring matching "\([a-zA-Z0-9]+-(32|64)\)" and then removes it.
 * That will catch things like "(aarch-64)" and "(x86-32)" but it
 * would also match "(SuperPickle47-64)" if that ever showed up.  I'm
 * going to choose to take that risk to avoid maintaining a static
 * list of %{_isa} substring values.
 */
static char *remove_isa_substring(char *requirement)
{
    char *r = NULL;
    char *tmp = NULL;
    char *s = requirement;
    regex_t re;
    regmatch_t pmatch[1];
    size_t nmatch = sizeof(pmatch) / sizeof(pmatch[0]);

    /* give me something to work with */
    if (requirement == NULL) {
        return NULL;
    }

    /* isa substring regex pattern */
    if (regcomp(&re, "\\([a-zA-Z0-9]+-(32|64)\\)", REG_EXTENDED | REG_NEWLINE) != 0) {
        warn("*** regcomp");
        r = strdup(requirement);
        return r;
    }

    /* scan the requirement string and remove all isa substrings */
    while (regexec(&re, s, nmatch, pmatch, 0) != REG_NOMATCH) {
        if (r) {
            tmp = strreplace(r, s + pmatch[0].rm_so, NULL);
            assert(tmp != NULL);
            free(r);
            r = tmp;
        } else {
            r = strreplace(requirement, s + pmatch[0].rm_so, NULL);
        }

        s += pmatch[0].rm_eo;
    }

    /* no isa substrings found, return a dupe of requirement */
    if (r == NULL) {
        r = strdup(requirement);
    }

    /* clean up */
    regfree(&re);

    return r;
}

/*
 * Scan all dependencies and look for version values containing
 * unexpanded macros.  Anything found is reported as a failure.
 */
static bool have_unexpanded_macros(struct rpminspect *ri, const char *name, const char *arch, deprule_list_t *deprules)
{
    bool result = true;
    deprule_entry_t *entry = NULL;
    char *near = NULL;
    char *far = NULL;
    char *noun = NULL;
    const char *desc = NULL;
    char *r = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(name != NULL);
    assert(name != NULL);

    if (deprules == NULL) {
        return true;
    }

    /* initialize result parameters */
    init_result_params(&params);
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_RPMDEPS;
    params.remedy = get_remedy(REMEDY_RPMDEPS_MACROS);
    params.file = specfile;

    /* check all dependencies */
    TAILQ_FOREACH(entry, deprules, items) {
        if (entry->version == NULL) {
            continue;
        }

        /* find the macro beginning substring */
        near = far = strstr(entry->version, "%{");

        /* starting from there, find the macro ending substring */
        if (near != NULL) {
            far = strstr(entry->version, "}");
        }

        /* we have at least one macro, report */
        if (near != NULL && far != NULL) {
            desc = get_deprule_desc(entry->type);
            r = strdeprule(entry);

            xasprintf(&params.msg, _("Invalid looking %s dependency in the %s package on %s: %s"), desc, name, arch, r);
            params.severity = RESULT_BAD;
            params.verb = VERB_FAILED;
            xasprintf(&noun, _("'${FILE}' in %s on ${ARCH}"), name);
            params.file = r;
            params.noun = noun;
            params.arch = arch;
            add_result(ri, &params);
            free(params.msg);
            free(noun);
            free(r);

            result = false;
        }
    }

    return result;
}

/*
 * Given a list of multiple providers of a dynamic provides, check for
 * explicit requires and return a list of them.
 */
static string_list_t *get_explicit_requires(struct rpminspect *ri, const deprule_entry_t *dep)
{
    string_list_t *requires = NULL;
    string_entry_t *entry = NULL;
    deprule_entry_t *req = NULL;
    rpmpeer_entry_t *peer = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);
    assert(dep != NULL);

    if (dep->providers == NULL || TAILQ_EMPTY(dep->providers)) {
        return NULL;
    }

    if (list_len(dep->providers) <= 1) {
        return NULL;
    }

    /*
     * given a list of providers, check for any Requires on any other
     * packages for those providers
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* skip anything with no dependencies */
        if (peer->after_deprules == NULL || TAILQ_EMPTY(peer->after_deprules)) {
            continue;
        }

        /* check for explicit Requires of multiple providers */
        TAILQ_FOREACH(req, peer->after_deprules, items) {
            /* skip the deprule we were called with or non-Requires */
            if (req == dep || req->type != TYPE_REQUIRES) {
                continue;
            }

            TAILQ_FOREACH(entry, dep->providers, items) {
                if (!strcmp(entry->data, req->requirement)) {
                    requires = list_add(requires, entry->data);
                }
            }
        }
    }

    return requires;
}

/*
 * Verify the after build subpackages all carry explicit Requires:
 * dependencies for autogenerated shared library dependencies.  Also
 * make sure there are not multiple packages providing the same shared
 * library dependency.
 */
static bool check_explicit_lib_deps(struct rpminspect *ri, Header h, deprule_list_t *after_deps)
{
    bool result = true;
    const char *name = NULL;
    const char *arch = NULL;
    const char *srpm = NULL;
    const char *peer_srpm = NULL;
    rpmpeer_entry_t *peer = NULL;
    rpmpeer_entry_t *potential_prov = NULL;
    deprule_entry_t *verify = NULL;
    deprule_entry_t *req = NULL;
    deprule_entry_t *prov = NULL;
    char *isareq = NULL;
    char *isaprov = NULL;
    string_list_t *explicit_requires = NULL;
    char *multiples = NULL;
    char *requires = NULL;
    char *r = NULL;
    char *vr = NULL;
    char *evr = NULL;
    char *noun = NULL;
    const char *pn = NULL;
    const char *pv = NULL;
    const char *pr = NULL;
    uint64_t epoch = 0;
    char *rulestr = NULL;
    bool found = false;
    bool collect = false;
    const char *tn = NULL;
    string_entry_t *entry = NULL;
    string_list_t *transitive = NULL;
    struct result_params params;

    assert(ri != NULL);

    if (h == NULL) {
        return true;
    }

    name = headerGetString(h, RPMTAG_NAME);
    arch = get_rpm_header_arch(h);
    srpm = headerGetString(h, RPMTAG_SOURCERPM);

    init_result_params(&params);
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_RPMDEPS;
    params.file = specfile;

    /* iterate over the deps of the after build peer */
    TAILQ_FOREACH(req, after_deps, items) {
        /* only looking at lib* dependencies right now */
        if ((req->type == TYPE_PROVIDES)
            || !(strprefix(req->requirement, SHARED_LIB_PREFIX) && strstr(req->requirement, SHARED_LIB_SUFFIX))) {
            continue;
        }

        found = false;
        potential_prov = NULL;

        /* we have a lib Requires, find what subpackage Provides it */
        TAILQ_FOREACH(peer, ri->peers, items) {
            /* skip anything with no dependencies */
            if (peer->after_deprules == NULL || TAILQ_EMPTY(peer->after_deprules)) {
                continue;
            }

            peer_srpm = headerGetString(peer->after_hdr, RPMTAG_SOURCERPM);

            /*
             * issue 1246: Prevent false positives when the library is provided
             * by a subpackage coming from another SRPM.
             */
            if (ri->buildtype == KOJI_BUILD_MODULE && peer_srpm && strcmp(peer_srpm, srpm)) {
                continue;
            }

            /* check this peer's Provides */
            TAILQ_FOREACH(prov, peer->after_deprules, items) {
                /* skip the entry we're trying to match against */
                if (req == prov) {
                    continue;
                }

                /* only looking at Provides right now */
                if ((prov->type != TYPE_PROVIDES)
                    || !(strprefix(req->requirement, SHARED_LIB_PREFIX) && strstr(req->requirement, SHARED_LIB_SUFFIX))) {
                    continue;
                }

                pn = headerGetString(peer->after_hdr, RPMTAG_NAME);

                /* check that the Requires is explicit and matches the Provides */
                if (!strcmp(req->requirement, prov->requirement)) {
                    /* a package is allowed to to Provide and Require the same thing */
                    /* otherwise we found the subpackage that Provides this explicit Requires */
                    potential_prov = peer;

                    if (!list_contains(req->providers, pn)) {
                        req->providers = list_add(req->providers, pn);
                    }

                    found = true;
                } else if (strrchr(req->requirement, '(') || strrchr(prov->requirement, '(')) {
                    /*
                     * we may have a dependency such as:
                     *     Requires: %{name}-libs%{?_isa} = %{version}-%{release}'
                     * trim the '(x86-64)' or similar ISA substring for comparison purposes
                     * also trim leading '(' to account for rich deps
                     */
                    isareq = remove_isa_substring(req->requirement);
                    assert(isareq != NULL);

                    isaprov = remove_isa_substring(prov->requirement);
                    assert(isaprov != NULL);

                    if (!strcmp(isareq, isaprov)) {
                        potential_prov = peer;

                        if (!list_contains(req->providers, pn)) {
                            req->providers = list_add(req->providers, pn);
                        }

                        found = true;
                    }

                    free(isareq);
                    free(isaprov);
                }
            }
        }

        /* all of the explicit Requires for this req */
        explicit_requires = get_explicit_requires(ri, req);

        /* now look for the explicit Requires of potential_prov */
        if (list_len(req->providers) == 1 && list_len(explicit_requires) == 0) {
            if (found && potential_prov) {
                /* prove yourself again */
                found = false;

                pn = headerGetString(potential_prov->after_hdr, RPMTAG_NAME);
                pv = headerGetString(potential_prov->after_hdr, RPMTAG_VERSION);
                pr = headerGetString(potential_prov->after_hdr, RPMTAG_RELEASE);

                /* the version-release or epoch:version-release string */
                epoch = headerGetNumber(potential_prov->after_hdr, RPMTAG_EPOCH);

                xasprintf(&evr, "%ju:%s-%s", epoch, pv, pr);
                assert(evr != NULL);

                xasprintf(&vr, "%s-%s", pv, pr);
                assert(vr != NULL);

                TAILQ_FOREACH(verify, after_deps, items) {
                    /* look only at explicit deps right now */
                    if ((verify->type == TYPE_PROVIDES)
                        || (strprefix(verify->requirement, SHARED_LIB_PREFIX) && strstr(verify->requirement, SHARED_LIB_SUFFIX))) {
                        continue;
                    }

                    /* trim potential %{_isa} suffix */
                    isareq = remove_isa_substring(verify->requirement);
                    assert(isareq != NULL);

                    if (!strcmp(isareq, pn) && verify->op == OP_EQUAL && (!strcmp(verify->version, evr) || !strcmp(verify->version, vr))) {
                        verify->direct = true;
                        found = true;
                    }

                    free(isareq);

                    if (found) {
                        break;
                    }
                }

                /* could be circular */
                if (!found && !strcmp(name, pn)) {
                    found = true;
                }

                /* requires could be handled by a transitive dependency */
                if (!found) {
                    /*
                     * Collect all Requires down the build/
                     * This is weird, but first walk the current
                     * package's Requires list and collect the package
                     * names.  Then the next while loop will walk all
                     * of those Requires and look at those peer
                     * packages and gather their Requires until there
                     * are no more to gather.  We have found it via a
                     * transitive Requires if the transitive list
                     * contains the potential_prov's package name.
                     */
                    TAILQ_FOREACH(verify, after_deps, items) {
                        if (verify->type == TYPE_REQUIRES && *verify->requirement != '/'
                            && !strprefix(verify->requirement, SHARED_LIB_PREFIX) && !strstr(verify->requirement, SHARED_LIB_SUFFIX)) {
                            isareq = remove_isa_substring(verify->requirement);
                            assert(isareq != NULL);

                            if (is_subpackage(ri->peers, isareq) && !list_contains(transitive, isareq)) {
                                transitive = list_add(transitive, isareq);
                            }

                            free(isareq);
                        }
                    }

                    if (transitive && !TAILQ_EMPTY(transitive)) {
                        collect = true;

                        while (collect) {
                            TAILQ_FOREACH(peer, ri->peers, items) {
                                tn = headerGetString(peer->after_hdr, RPMTAG_NAME);
                                collect = false;

                                TAILQ_FOREACH(entry, transitive, items) {
                                    if (!strcmp(tn, entry->data)) {
                                        TAILQ_FOREACH(verify, peer->after_deprules, items) {
                                            if (verify->type == TYPE_REQUIRES
                                                && *verify->requirement != '/'
                                                && !strprefix(verify->requirement, SHARED_LIB_PREFIX) && !strstr(verify->requirement, SHARED_LIB_SUFFIX)) {
                                                isareq = remove_isa_substring(verify->requirement);
                                                assert(isareq != NULL);

                                                if (list_contains(transitive, pn)) {
                                                    found = true;
                                                    collect = false;
                                                    break;
                                                }

                                                if (is_subpackage(ri->peers, isareq) && !list_contains(transitive, isareq)) {
                                                    transitive = list_add(transitive, isareq);
                                                    collect = true;
                                                }

                                                free(isareq);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (!found && list_contains(transitive, pn)) {
                            found = true;
                        }
                    }

                    list_free(transitive, free);
                    transitive = NULL;
                }

                free(evr);
                free(vr);
            }

            /* report missing explicit package requires */
            if (!found && potential_prov && !is_debuginfo_rpm(h) && !is_debugsource_rpm(h)) {
                r = strdeprule(req);
                pn = headerGetString(potential_prov->after_hdr, RPMTAG_NAME);

                if (epoch > 0) {
                    rulestr = "%{epoch}:%{version}-%{release}";
                    params.remedy = get_remedy(REMEDY_RPMDEPS_EXPLICIT_EPOCH);
                } else {
                    rulestr = "%{version}-%{release}";
                    params.remedy = get_remedy(REMEDY_RPMDEPS_EXPLICIT);
                }

                xasprintf(&params.msg, _("Subpackage %s on %s carries '%s' which comes from subpackage %s but does not carry an explicit package version requirement.  Please add 'Requires: %s = %s' to the spec file to avoid the need to test interoperability between various combinations of old and new subpackages."), name, arch, r, pn, pn, rulestr);
                xasprintf(&noun, _("missing 'Requires: ${FILE} = %s' in %s on ${ARCH}"), rulestr, name);

                params.severity = RESULT_VERIFY;
                params.noun = noun;
                params.verb = VERB_FAILED;
                params.file = pn;
                params.arch = arch;
                add_result(ri, &params);
                free(noun);
                free(params.msg);
                free(r);

                result = false;
            }
        }

        /* check for multiple providers for each Requires */
        if (list_len(req->providers) > 1 && list_len(explicit_requires) > 0) {
            r = strdeprule(req);
            multiples = list_to_string(req->providers, ", ");
            requires = list_to_string(explicit_requires, ", ");

            xasprintf(&params.msg, _("Multiple subpackages provide '%s': [%s] but these subpackages carry explicit Requires: [%s]"), r, multiples, requires);
            xasprintf(&noun, _("%s all provide '${FILE}' on ${ARCH}"), multiples);
            params.severity = RESULT_VERIFY;
            params.file = r;
            params.arch = arch;
            params.remedy = get_remedy(REMEDY_RPMDEPS_MULTIPLE);
            params.verb = VERB_FAILED;
            params.noun = noun;
            add_result(ri, &params);
            free(params.msg);
            free(noun);
            free(r);

            free(multiples);
            free(requires);
            result = false;
        }

        list_free(explicit_requires, free);
    }

    return result;
}

/*
 * For packages in a deprule that carry an Epoch > 0, make sure they
 * are listed with the explicit Epoch value in the deprule.
 */
static bool check_explicit_epoch(struct rpminspect *ri, Header h, deprule_list_t *afterdeps)
{
    bool result = true;
    const char *pname = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    uint64_t epoch;
    deprule_entry_t *deprule = NULL;
    char *drs = NULL;
    char *noun = NULL;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;

    assert(ri != NULL);

    if (h == NULL) {
        return true;
    }

    pname = headerGetString(h, RPMTAG_NAME);
    arch = get_rpm_header_arch(h);

    /* need deps to continue here */
    if (afterdeps == NULL || TAILQ_EMPTY(afterdeps)) {
        return true;
    }

    /* set up result parameters */
    init_result_params(&params);
    params.header = NAME_RPMDEPS;
    params.file = specfile;

    if (is_rebase(ri)) {
        params.waiverauth = NOT_WAIVABLE;
        params.severity = RESULT_INFO;
    } else {
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.severity = RESULT_BAD;
    }

    TAILQ_FOREACH(deprule, afterdeps, items) {
        /* skip deprules that just carry a package name */
        if (deprule->version == NULL) {
            continue;
        }

        /* find the package providing this deprule */
        TAILQ_FOREACH(peer, ri->peers, items) {
            if (!peer->after_hdr) {
                continue;
            }

            name = headerGetString(peer->after_hdr, RPMTAG_NAME);
            assert(name != NULL);

            if (strcmp(deprule->requirement, name)) {
                /* not found */
                continue;
            }

            epoch = headerGetNumber(peer->after_hdr, RPMTAG_EPOCH);

            /* check the deprule to see that it carries the required Epoch */
            if (epoch > 0 && !strstr(deprule->version, ":")) {
                drs = strdeprule(deprule);
                xasprintf(&params.msg, _("Missing epoch prefix on the version-release in '%s' for %s on %s"), drs, pname, arch);
                xasprintf(&noun, _("'${FILE}' needs epoch in %s on ${ARCH}"), name);

                params.remedy = get_remedy(REMEDY_RPMDEPS_EPOCH);
                params.verb = VERB_FAILED;
                params.noun = noun;
                params.arch = arch;
                params.file = drs;

                add_result(ri, &params);
                free(params.msg);
                free(noun);
                free(drs);

                result = false;
                break;
            }
        }
    }

    return result;
}

/*
 * Check if the deprule change is expected (e.g., automatic Provides).
 */
static bool expected_deprule_change(const bool rebase, const deprule_entry_t *deprule, const Header h, const rpmpeer_t *peers)
{
    bool r = false;
    bool config = false;
    bool found = false;
    char *req = NULL;
    char *working_req = NULL;
    char *vr = NULL;
    char *evr = NULL;
    char *buf = NULL;
    char *suffix = NULL;
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    const char *peerarch = NULL;
    const char *version = NULL;
    const char *release = NULL;
    uint64_t epoch = 0;
    Header working_hdr = NULL;

    assert(deprule != NULL);
    assert(h != NULL);

    /* skip source packages */
    if (headerIsSource(h)) {
        return true;
    }

    /* changes always expected in a rebase */
    if (rebase) {
        return true;
    }

    /* Rich dependencies and explicit dependencies for automatic deps are expected */
    if (deprule->rich || deprule->direct) {
        return true;
    }

    /* this package arch */
    arch = get_rpm_header_arch(h);

    /* a copy of the req to work with */
    working_req = req = remove_isa_substring(deprule->requirement);
    assert(req != NULL);

    /* trim any leading '(' to account for rich deps */
    while (*working_req == '(') {
        working_req++;
    }

    /* trim config() wrapper */
    if (strprefix(working_req, "config(")) {
        config = true;
        working_req += 7;
        working_req[strcspn(working_req, ")")] = '\0';
    }

    /* gather any + suffix from the version */
    if (deprule->version) {
        buf = strrchr(deprule->version, '+');

        if (buf) {
            suffix = strdup(buf);
            assert(suffix != NULL);
        }
    }

    /* see if this deprule requirement name matches a subpackage */
    TAILQ_FOREACH(peer, peers, items) {
        /* no after peer, ignore */
        if (peer->after_hdr == NULL) {
            continue;
        }

        /* skip source */
        if (headerIsSource(peer->after_hdr)) {
            continue;
        }

        /* match the arch and name */
        peerarch = get_rpm_header_arch(peer->after_hdr);
        name = headerGetString(peer->after_hdr, RPMTAG_NAME);

        if (!strcmp(arch, peerarch) && !strcmp(name, working_req)) {
            /* we found the subpackage, which is now 'peer' for code below */
            found = true;
            working_hdr = peer->after_hdr;
            break;
        }
    }

    /*
     * deprule may be a virtual deprule that does not match a
     * subpackage but matches the version of the package, such
     * as:   Provides: thing = 1.2.3-4
     * where 1.2.3-4 matches %{version}-%{release}
     */
    if (!found && deprule->version && deprule->op == OP_EQUAL) {
        found = true;
        working_hdr = h;
    }

    /* deprule version strings are a combo of the version-release or epoch:version-release */
    if (deprule->version) {
        if (!found && !config) {
            /* use the main package vr and evr */
            if ((pkg_evr && !strcmp(deprule->version, pkg_evr)) || (pkg_vr && !strcmp(deprule->version, pkg_vr))) {
                r = true;
            }
        } else {
            /* check against the subpackage match we found */
            version = headerGetString(working_hdr, RPMTAG_VERSION);
            release = headerGetString(working_hdr, RPMTAG_RELEASE);
            epoch = headerGetNumber(working_hdr, RPMTAG_EPOCH);
            arch = get_rpm_header_arch(working_hdr);

            /* first, start with the basic strings */
            xasprintf(&vr, "%s-%s", version, release);
            assert(vr != NULL);

            xasprintf(&evr, "%ju:%s-%s", epoch, version, release);
            assert(evr != NULL);

            /* check for trailing information on the version */
            if (suffix) {
                xasprintf(&buf, ".%s%s", arch, suffix);
                assert(buf != NULL);
            }

            if (suffix && strsuffix(deprule->version, buf)) {
                vr = strappend(vr, buf, NULL);
                assert(vr != NULL);
                evr = strappend(evr, buf, NULL);
                assert(evr != NULL);
            } else {
                /* do we have an architecture trailing? */
                if (strsuffix(deprule->version, arch)) {
                    vr = strappend(vr, ".", arch, NULL);
                    assert(vr != NULL);
                    evr = strappend(evr, ".", arch, NULL);
                    assert(evr != NULL);
                }

                /* do we have '+debug' trailing (usually for kernel packages)? */
                if (suffix && strsuffix(deprule->version, suffix)) {
                   vr = strappend(vr, suffix, NULL);
                   assert(vr != NULL);
                   evr = strappend(evr, suffix, NULL);
                   assert(evr != NULL);
               }
           }

            /* determine if this is expected */
            if (deprule->version && ((evr && !strcmp(deprule->version, evr)) || (vr && !strcmp(deprule->version, vr)))) {
                r = true;
            }

            free(vr);
            free(evr);
            free(buf);
        }
    } else if (deprule->version == NULL && found) {
        r = true;
    }

    free(req);
    free(suffix);
    return r;
}

/*
 * Main driver for the 'rpmdeps' inspection.
 */
bool inspect_rpmdeps(struct rpminspect *ri)
{
    bool result = true;
    bool rebase = false;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    deprule_entry_t *deprule = NULL;
    const char *name = NULL;
    const char *arch = NULL;
    const char *version = NULL;
    const char *release = NULL;
    char *drs = NULL;
    char *pdrs = NULL;
    char *noun = NULL;
    bool found = false;
    struct result_params params;

    assert(ri != NULL);

    /* are these builds a rebase? */
    rebase = is_rebase(ri);

    /* set up result parameters */
    init_result_params(&params);
    params.header = NAME_RPMDEPS;
    params.file = specfile;

    /*
     * create global package evr and vr substrings for comparisons
     */
    if (ri->peers && !TAILQ_EMPTY(ri->peers)) {
        peer = TAILQ_FIRST(ri->peers);
        version = headerGetString(peer->after_hdr, RPMTAG_VERSION);
        release = headerGetString(peer->after_hdr, RPMTAG_RELEASE);
        pkg_epoch = headerGetNumber(peer->after_hdr, RPMTAG_EPOCH);
        xasprintf(&pkg_vr, "%s-%s", version, release);
        xasprintf(&pkg_evr, "%ju:%s-%s", pkg_epoch, version, release);
    }

    /*
     * for reporting, we need the name of the spec file from the SRPM
     *
     * NOTE: we only need this for reporting, so if we don't have a
     * spec file name, we will just adjust the reporting strings
     * later
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (!headerIsSource(peer->after_hdr)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
                specfile = file->localpath;
                found = true;
            }
        }

        if (found) {
            break;
        }
    }

    /* for cases where the job lacks the SRPM, just say spec file */
    if (specfile == NULL) {
        specfile = _("spec file");
    }

    params.file = specfile;

    /*
     * first pass gathers deps and performs simple checks
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Gather deprules */
        if (peer->before_hdr && peer->before_deprules == NULL) {
            peer->before_deprules = gather_deprules(peer->before_hdr, ri->deprules_ignore);
        }

        if (peer->after_hdr && peer->after_deprules == NULL) {
            peer->after_deprules = gather_deprules(peer->after_hdr, ri->deprules_ignore);
        }

        /* Peer up the before and after deps */
        find_deprule_peers(peer->before_deprules, peer->after_deprules);

        /* Name and arch of this peer (try after build first) */
        name = NULL;
        arch = NULL;

        if (peer->after_hdr) {
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);
            arch = get_rpm_header_arch(peer->after_hdr);
        }

        if (name == NULL && arch == NULL && peer->before_hdr) {
            name = headerGetString(peer->before_hdr, RPMTAG_NAME);
            arch = get_rpm_header_arch(peer->before_hdr);
        }

        /* Check for unexpanded macros in the version fields of dependencies */
        if (name && arch && !have_unexpanded_macros(ri, name, arch, peer->after_deprules)) {
            result = false;
        }
    }

    /*
     * the second pass performs more complex checks
     */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Check for required explicit 'lib' dependencies */
        if (!check_explicit_lib_deps(ri, peer->after_hdr, peer->after_deprules)) {
            result = false;
        }

        /* Check that packages defining an Epoch > 0 use it in deprules */
        if (!check_explicit_epoch(ri, peer->after_hdr, peer->after_deprules)) {
            result = false;
        }
    }

    /* report dependency findings between the before and after build */
    if (ri->before && ri->after) {
        TAILQ_FOREACH(peer, ri->peers, items) {
            if (peer->after_hdr == NULL) {
                continue;
            }

            /* Name and arch of this peer */
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);
            arch = get_rpm_header_arch(peer->after_hdr);

            if (peer->after_deprules) {
                TAILQ_FOREACH(deprule, peer->after_deprules, items) {
                    /* reporting level */
                    params.waiverauth = NOT_WAIVABLE;
                    params.severity = RESULT_INFO;

                    /* use shorter variable names in the if expressions */
                    drs = strdeprule(deprule);
                    pdrs = strdeprule(deprule->peer_deprule);

                    /* determine what to report */
                    if (drs && pdrs == NULL) {
                        if (!strcmp(arch, SRPM_ARCH_NAME)) {
                            if (expected_deprule_change(rebase, deprule, peer->after_hdr, ri->peers)) {
                                xasprintf(&params.msg, _("Gained '%s' in source package %s; this is expected"), drs, name);
                            } else {
                                xasprintf(&params.msg, _("Gained '%s' in source package %s"), drs, name);
                            }
                        } else {
                            if (expected_deprule_change(rebase, deprule, peer->after_hdr, ri->peers)) {
                                xasprintf(&params.msg, _("Gained '%s' in subpackage %s on %s; this is expected"), drs, name, arch);
                            } else {
                                xasprintf(&params.msg, _("Gained '%s' in subpackage %s on %s"), drs, name, arch);
                            }
                        }

                        xasprintf(&noun, _("'${FILE}' in %s on ${ARCH}"), name);
                        params.remedy = get_remedy(REMEDY_RPMDEPS_GAINED);
                        params.verb = VERB_ADDED;
                    } else if (deprules_match(deprule, deprule->peer_deprule)) {
                        if (!strcmp(arch, SRPM_ARCH_NAME)) {
                            if (expected_deprule_change(rebase, deprule, peer->after_hdr, ri->peers)) {
                                xasprintf(&params.msg, _("Retained '%s' in source package %s; this is expected"), drs, name);
                            } else {
                                xasprintf(&params.msg, _("Retained '%s' in source package %s"), drs, name);
                            }
                        } else {
                            if (expected_deprule_change(rebase, deprule, peer->after_hdr, ri->peers)) {
                                xasprintf(&params.msg, _("Retained '%s' in subpackage %s on %s; this is expected"), drs, name, arch);
                            } else {
                                xasprintf(&params.msg, _("Retained '%s' in subpackage %s on %s"), drs, name, arch);
                            }
                        }

                        xasprintf(&noun, _("'${FILE}' in %s on ${ARCH}"), name);
                        params.remedy = NULL;
                        params.verb = VERB_OK;
                    } else {
                        if (!strcmp(arch, SRPM_ARCH_NAME)) {
                            if (expected_deprule_change(rebase, deprule, peer->after_hdr, ri->peers)) {
                                xasprintf(&params.msg, _("Changed '%s' to '%s' in source package %s; this is expected"), pdrs, drs, name);
                            } else {
                                xasprintf(&params.msg, _("Changed '%s' to '%s' in source package %s"), pdrs, drs, name);
                            }
                        } else {
                            if (expected_deprule_change(rebase, deprule, peer->after_hdr, ri->peers)) {
                                xasprintf(&params.msg, _("Changed '%s' to '%s' in subpackage %s on %s; this is expected"), pdrs, drs, name, arch);
                            } else {
                                xasprintf(&params.msg, _("Changed '%s' to '%s' in subpackage %s on %s"), pdrs, drs, name, arch);
                            }
                        }

                        xasprintf(&noun, _("'%s' became '${FILE}' in %s on ${ARCH}"), pdrs, name);
                        params.remedy = get_remedy(REMEDY_RPMDEPS_CHANGED);
                        params.verb = VERB_CHANGED;
                    }

                    /* report the result */
                    params.noun = noun;
                    params.arch = arch;
                    params.file = drs;
                    add_result(ri, &params);
                    free(params.msg);
                    free(noun);
                    free(drs);
                    free(pdrs);
                }
            }

            if (peer->before_deprules) {
                TAILQ_FOREACH(deprule, peer->before_deprules, items) {
                    drs = strdeprule(deprule);
                    pdrs = strdeprule(deprule->peer_deprule);

                    if (drs && pdrs == NULL) {
                        if (!strcmp(arch, SRPM_ARCH_NAME)) {
                            xasprintf(&params.msg, _("Lost '%s' in source package %s"), drs, name);
                        } else {
                            xasprintf(&params.msg, _("Lost '%s' in subpackage %s on %s"), drs, name, arch);
                        }

                        params.details = NULL;
                        params.remedy = get_remedy(REMEDY_RPMDEPS_LOST);
                        params.verb = VERB_REMOVED;
                        xasprintf(&noun, _("'${FILE}' in %s on ${ARCH}"), name);
                        params.noun = noun;
                        params.file = pdrs;
                        params.arch = arch;
                        params.waiverauth = NOT_WAIVABLE;
                        params.severity = RESULT_INFO;

                        add_result(ri, &params);
                        free(params.msg);
                        free(noun);
                    }

                    free(pdrs);
                    free(drs);
                }
            }
        }
    }

    /* if everything was fine, just say so */
    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_RPMDEPS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    free(pkg_vr);
    free(pkg_evr);

    return result;
}
