/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * Given a deprule requirement string, return true if it is a rich
 * dependency string.  This does what rpmdsIsRich() does in
 * lib/rpmds.c from rpm, but without first creating a rpmds which we
 * don't need here.
 */
static bool is_rich_dep(const char *requirement)
{
    if (requirement && *requirement == '(') {
        return true;
    }

    return false;
}

/*
 * Gather the specific type of deprules and add them to rules.  Return
 * the list.
 */
static deprule_list_t *gather_deprules_by_type(deprule_list_t *rules, Header hdr, dep_type_t type, deprule_ignore_map_t *ignores)
{
    rpmTag rtag, otag, vtag;
    rpmtd req = NULL;
    rpmtd op = NULL;
    rpmtd ver = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    const char *r = NULL;
    const char *v = NULL;
    deprule_list_t *deprules = rules;
    deprule_entry_t *deprule_entry = NULL;
    deprule_ignore_map_t *ignore_rule = NULL;
    deprule_ignore_map_t *tmp_ignore_rule = NULL;
    bool ignore = false;

    assert(hdr != NULL);
    assert(type != TYPE_NULL);

    /* determine the header tags to use */
    if (type == TYPE_REQUIRES) {
        rtag = RPMTAG_REQUIRENAME;
        otag = RPMTAG_REQUIREFLAGS;
        vtag = RPMTAG_REQUIREVERSION;
    } else if (type == TYPE_PROVIDES) {
        rtag = RPMTAG_PROVIDENAME;
        otag = RPMTAG_PROVIDEFLAGS;
        vtag = RPMTAG_PROVIDEVERSION;
    } else if (type == TYPE_CONFLICTS) {
        rtag = RPMTAG_CONFLICTNAME;
        otag = RPMTAG_CONFLICTFLAGS;
        vtag = RPMTAG_CONFLICTVERSION;
    } else if (type == TYPE_OBSOLETES) {
        rtag = RPMTAG_OBSOLETENAME;
        otag = RPMTAG_OBSOLETEFLAGS;
        vtag = RPMTAG_OBSOLETEVERSION;
#ifdef RPMTAG_ENHANCES
    } else if (type == TYPE_ENHANCES) {
        rtag = RPMTAG_ENHANCENAME;
        otag = RPMTAG_ENHANCEFLAGS;
        vtag = RPMTAG_ENHANCEVERSION;
#endif
#ifdef RPMTAG_RECOMMENDS
    } else if (type == TYPE_RECOMMENDS) {
        rtag = RPMTAG_RECOMMENDNAME;
        otag = RPMTAG_RECOMMENDFLAGS;
        vtag = RPMTAG_RECOMMENDVERSION;
#endif
#ifdef RPMTAG_SUGGESTS
    } else if (type == TYPE_SUGGESTS) {
        rtag = RPMTAG_SUGGESTNAME;
        otag = RPMTAG_SUGGESTFLAGS;
        vtag = RPMTAG_SUGGESTVERSION;
#endif
#ifdef RPMTAG_SUPPLEMENTS
    } else if (type == TYPE_SUPPLEMENTS) {
        rtag = RPMTAG_SUPPLEMENTNAME;
        otag = RPMTAG_SUPPLEMENTFLAGS;
        vtag = RPMTAG_SUPPLEMENTVERSION;
#endif
    } else {
        /*
         * These are tags for other dependencies that we do not
         * directly check.  For example, auto-generated dependencies
         * on things like "rtld(GNU_HASH)" for use as triggers.  Other
         * inspections may check these things, but 'rpmdeps' focuses
         * on Provides/Requires and the similar dependency types.
         */
        return deprules;
    }

    /* start new header transactions */
    req = rpmtdNew();
    op = rpmtdNew();
    ver = rpmtdNew();

    if (headerGet(hdr, rtag, req, flags) && headerGet(hdr, otag, op, flags) && headerGet(hdr, vtag, ver, flags)) {
        if (deprules == NULL) {
            deprules = calloc(1, sizeof(*deprules));
            assert(deprules != NULL);
            TAILQ_INIT(deprules);
        }

        /* collect all of the rules for this package */
        while ((rpmtdNext(req) != -1) && (rpmtdNext(op) != -1) && (rpmtdNext(ver) != -1)) {
            r = rpmtdGetString(req);

            /* skip some rules types */
            if (!strcmp(r, DEBUGINFO_PROVIDE)
                || strsuffix(r, DEBUGSOURCE_SUFFIX)
                || strsuffix(r, DEBUGINFO_SUFFIX)
                || ((strprefix(r, "rpmlib(") || strprefix(r, "rtld(")) && strsuffix(r, ")"))
                || ((strprefix(r, "kernel(") || strprefix(r, "modalias(") || strprefix(r, "ksym(") || strprefix(r, "kmod(")) && strsuffix(r, ")"))) {
                continue;
            }

            /* ignore any rules set by the configuration file */
            if (ignores != NULL) {
                ignore = false;

                HASH_ITER(hh, ignores, ignore_rule, tmp_ignore_rule) {
                    if (ignore_rule->type != type) {
                        continue;
                    }

                    if (regexec(ignore_rule->ignore, r, 0, NULL, 0) == 0) {
                        ignore = true;
                        break;
                    }
                }

                if (ignore) {
                    continue;
                }
            }

            v = rpmtdGetString(ver);

            deprule_entry = (deprule_entry_t *) calloc(1, sizeof(*deprule_entry));
            assert(deprule_entry != NULL);

            deprule_entry->type = type;

            deprule_entry->requirement = strdup(r);
            assert(deprule_entry->requirement != NULL);

            deprule_entry->op = get_dep_operator(*(rpmtdGetUint32(op)));

            if (!strcmp(v, "")) {
                deprule_entry->version = NULL;
            } else {
                deprule_entry->version = strdup(v);
                assert(deprule_entry->version != NULL);
            }

            deprule_entry->rich = is_rich_dep(deprule_entry->requirement);
            deprule_entry->direct = false;

            TAILQ_INSERT_TAIL(deprules, deprule_entry, items);
        }
    }

    rpmtdFreeData(req);
    rpmtdFreeData(op);
    rpmtdFreeData(ver);
    rpmtdFree(req);
    rpmtdFree(op);
    rpmtdFree(ver);

    return deprules;
}

/*
 * Collect the dependency type specified and return the allocated
 * deprule_list_t.
 */
deprule_list_t *gather_deprules(Header hdr, deprule_ignore_map_t *ignores)
{
    deprule_list_t *rules = NULL;
    dep_type_t t = TYPE_NULL;

    if (hdr == NULL) {
        return NULL;
    }

    for (t = FIRST_DEP_TYPE; t <= LAST_DEP_TYPE; t++) {
        rules = gather_deprules_by_type(rules, hdr, t, ignores);
    }

    return rules;
}

/* Given a deprule requirement, trim off rich dependency markup */
static char *trim_rich_dep(const char *requirement)
{
    char *r = NULL;
    char *req = NULL;
    char *tmp = NULL;

    if (requirement == NULL) {
        return NULL;
    }

    tmp = req = strdup(requirement);
    assert(req != NULL);

    while (*tmp == '(') {
        tmp++;
    }

    tmp[strcspn(tmp, " \f\n\r\t\v")] = '\0';
    r = strdup(tmp);
    assert(r != NULL);
    free(req);

    return r;
}

/* Given a pair of deprules, see if they are peers */
static bool process_pair(deprule_entry_t *left, deprule_entry_t *right, const bool strict)
{
    char *ra = NULL;
    char *rb = NULL;
    bool match = false;

    assert(left != NULL);
    assert(right != NULL);

    /* The types must match before anything else */
    if (left->type != right->type) {
        return false;
    }

    if (strict) {
        if ((left->requirement && right->requirement && !strcmp(left->requirement, right->requirement))
            && left->op == right->op
            && ((left->version == NULL && right->version == NULL) || (left->version && right->version && !strcmp(left->version, right->version)))) {
            match = true;
        }
    } else {
        /* handle any possible rich dependency strings */
        /* trim leading parens and cut everything after the first whitespace */
        ra = trim_rich_dep(left->requirement);
        rb = trim_rich_dep(right->requirement);

        if (!strcmp(ra, rb)) {
            match = true;
        }

        free(ra);
        free(rb);
    }

    if (match) {
        left->peer_deprule = right;
        right->peer_deprule = left;
    }

    return match;
}

/**
 * @brief Find matching deprules between the before and after lists.
 *
 * Scan the before build and look for matching peer deprules in the after
 * build.  The peer_deprule members are populated with each other's
 * entries.  That is, the before build's peer_deprule will point to the
 * after build deprule and the after build peer_deprule will point to the
 * before build deprule.  If a deprule_entry_t peer_deprule is NULL, it
 * means it has no peer that could be found.
 *
 * @param before Before build package's deprule_list_t list.
 * @param after After build package's deprule_list_t list.
 */
void find_deprule_peers(deprule_list_t *before, deprule_list_t *after)
{
    int i = 0;
    bool strict = true;
    deprule_entry_t *before_entry = NULL;
    deprule_entry_t *after_entry = NULL;

    /* Make sure there is something to match */
    if ((before == NULL || TAILQ_EMPTY(before)) || (after == NULL || TAILQ_EMPTY(after))) {
        return;
    }

    /* Just do two passes across deprules to find peers */
    for (i = 0; i < 2; i++) {
        /* match from after to before */
        TAILQ_FOREACH(after_entry, after, items) {
            if (after_entry->peer_deprule) {
                continue;
            }

            TAILQ_FOREACH(before_entry, before, items) {
                if (before_entry->peer_deprule) {
                    continue;
                }

                if (process_pair(after_entry, before_entry, strict)) {
                    /* stop trying to match this peer since we found a match */
                    break;
                }
            }
        }

        /* match from before to after */
        TAILQ_FOREACH(before_entry, before, items) {
            if (before_entry->peer_deprule) {
                continue;
            }

            TAILQ_FOREACH(after_entry, after, items) {
                if (after_entry->peer_deprule) {
                    continue;
                }

                if (process_pair(before_entry, after_entry, strict)) {
                    /* stop trying to match this peer since we found a match */
                    break;
                }
            }
        }

        /* relax peer matching for subsequent runs */
        if (strict) {
            strict = false;
        }
    }

    return;
}

/*
 * Given a deprule type, return a descriptive string for use in
 * reporting.
 */
const char *get_deprule_desc(const dep_type_t type)
{
    if (type == TYPE_REQUIRES) {
        return "Requires";
    } else if (type == TYPE_PROVIDES) {
        return "Provides";
    } else if (type == TYPE_CONFLICTS) {
        return "Conflicts";
    } else if (type == TYPE_OBSOLETES) {
        return "Obsoletes";
    } else if (type == TYPE_ENHANCES) {
        return "Enhances";
    } else if (type == TYPE_RECOMMENDS) {
        return "Recommends";
    } else if (type == TYPE_SUGGESTS) {
        return "Suggests";
    } else if (type == TYPE_SUPPLEMENTS) {
        return "Supplements";
    } else {
        return NULL;
    }
}

/*
 * Given rpmsenseFlags from the RPM header, convert it to our dep_t
 * type.
 */
dep_op_t get_dep_operator(const rpmsenseFlags f)
{
    rpmsenseFlags localf = f & RPMSENSE_SENSEMASK;

    if ((localf & RPMSENSE_LESS) && (localf & RPMSENSE_EQUAL)) {
        return OP_LESSEQUAL;
    } else if ((localf & RPMSENSE_GREATER) && (localf & RPMSENSE_EQUAL)) {
        return OP_GREATEREQUAL;
    } else if (localf & RPMSENSE_LESS) {
        return OP_LESS;
    } else if (localf & RPMSENSE_GREATER) {
        return OP_GREATER;
    } else if (localf & RPMSENSE_EQUAL) {
        return OP_EQUAL;
    } else {
        return OP_NULL;
    }
}

/*
 * Given a dep_op_t, return a string representing the operator.
 */
const char *get_deprule_operator_desc(const dep_op_t op)
{
    if (op == OP_EQUAL) {
        return "=";
    } else if (op == OP_LESS) {
        return "<";
    } else if (op == OP_GREATER) {
        return ">";
    } else if (op == OP_LESSEQUAL) {
        return "<=";
    } else if (op == OP_GREATEREQUAL) {
        return ">=";
    } else {
        return NULL;
    }
}

/*
 * Given a deprule, construct a human-readable version of it.  Caller
 * must free the returned string.
 */
char *strdeprule(const deprule_entry_t *deprule)
{
    char *r = NULL;

    if (deprule == NULL || deprule->requirement == NULL) {
        return NULL;
    }

    /* start with the basic string */
    xasprintf(&r, "%s: %s", get_deprule_desc(deprule->type), deprule->requirement);
    assert(r != NULL);

    /* we may have an operator and version */
    if (deprule->op != OP_NULL && deprule->version) {
        r = strappend(r, " ", get_deprule_operator_desc(deprule->op), " ", deprule->version, NULL);
        assert(r != NULL);
    }

    return r;
}

/*
 * Compare two deprule_entry_t structures and return true if they are
 * the same, false otherwise.
 */
bool deprules_match(const deprule_entry_t *a, const deprule_entry_t *b)
{
    bool r = false;
    char *ora = NULL;
    char *ra = NULL;
    char *orb = NULL;
    char *rb = NULL;
    char *ova = NULL;
    char *va = NULL;
    char *ovb = NULL;
    char *vb = NULL;
    bool n_match = false;
    bool v_match = false;

    assert(a != NULL);
    assert(b != NULL);

    /* make copies of the requirement and version strings */
    if (a->requirement) {
        ra = ora = strdup(a->requirement);
        assert(ora != NULL);
    }

    if (b->requirement) {
        rb = orb = strdup(b->requirement);
        assert(orb != NULL);
    }

    if (a->version) {
        va = ova = strdup(a->version);
        assert(ova != NULL);
    }

    if (b->version) {
        vb = ovb = strdup(b->version);
        assert(ovb != NULL);
    }

    /* trim any leading parens in case of rich deps */
    while (ra && *ra == '(') {
        ra++;
    }

    while (rb && *rb == '(') {
        rb++;
    }

    /* drop any rich deps clauses from the version strings */
    if (va) {
        va[strcspn(va, " \f\n\r\t\v")] = '\0';
    }

    if (vb) {
        vb[strcspn(vb, " \f\n\r\t\v")] = '\0';
    }

    n_match = (ra == NULL && rb == NULL) || (ra && rb && !strcmp(ra, rb));
    v_match = (va == NULL && vb == NULL) || (va && vb && !strcmp(va, vb));

    r = n_match && (a->op == b->op) && v_match;

    free(ora);
    free(orb);
    free(ova);
    free(ovb);

    return r;
}
