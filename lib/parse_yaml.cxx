/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/* This file is derived from libtyranny:
 * https://github.com/frozencemetery/libtyranny/ */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "internal/tyranny.h"
#include "parser.h"
#include "rpminspect.h"

/* Uncomment to enable debug printing. */
/* #define DEBUG 1 */
#ifdef DEBUG
#    define dprintf printf
#else
#    define dprintf(...)
#endif

/* Always 0-initialized, unless the compiler disagrees. */
static inline void *xalloc(size_t s)
{
    void *ret = NULL;

    ret = calloc(1, s);
    assert(ret != NULL);
    return ret;
}

/* Note that xrealloc(NULL, ...) works the way you'd want. */
static inline void *xrealloc(void *p, size_t s)
{
    void *ret;

    if (p == NULL) {
        return xalloc(s);
    }

    ret = realloc(p, s);
    assert(ret);
    return ret;
}

/*
 * Not only do they not give a method for this awfulness, and not only
 * do they put enums in their public headers, but they can't even be
 * bothered to initialize them.  Cross your fingers and hope we were
 * built with a compiler close enough to the distro's libyaml!
 */
const char *tokname(yaml_token_type_t t)
{
    switch (t) {
        case YAML_NO_TOKEN:
            return "YAML_NO_TOKEN";
        case YAML_STREAM_START_TOKEN:
            return "YAML_STREAM_START_TOKEN";
        case YAML_STREAM_END_TOKEN:
            return "YAML_STREAM_END_TOKEN";
        case YAML_VERSION_DIRECTIVE_TOKEN:
            return "YAML_VERSION_DIRECTIVE_TOKEN";
        case YAML_TAG_DIRECTIVE_TOKEN:
            return "YAML_TAG_DIRECTIVE_TOKEN";
        case YAML_DOCUMENT_START_TOKEN:
            return "YAML_DOCUMENT_START_TOKEN";
        case YAML_DOCUMENT_END_TOKEN:
            return "YAML_DOCUMENT_END_TOKEN";
        case YAML_BLOCK_SEQUENCE_START_TOKEN:
            return "YAML_BLOCK_SEQUENCE_START_TOKEN";
        case YAML_BLOCK_MAPPING_START_TOKEN:
            return "YAML_BLOCK_MAPPING_START_TOKEN";
        case YAML_BLOCK_END_TOKEN:
            return "YAML_BLOCK_END_TOKEN";
        case YAML_FLOW_SEQUENCE_START_TOKEN:
            return "YAML_FLOW_SEQUENCE_START_TOKEN";
        case YAML_FLOW_SEQUENCE_END_TOKEN:
            return "YAML_FLOW_SEQUENCE_END_TOKEN";
        case YAML_FLOW_MAPPING_START_TOKEN:
            return "YAML_FLOW_MAPPING_START_TOKEN";
        case YAML_FLOW_MAPPING_END_TOKEN:
            return "YAML_FLOW_MAPPING_END_TOKEN";
        case YAML_BLOCK_ENTRY_TOKEN:
            return "YAML_BLOCK_ENTRY_TOKEN";
        case YAML_FLOW_ENTRY_TOKEN:
            return "YAML_FLOW_ENTRY_TOKEN";
        case YAML_KEY_TOKEN:
            return "YAML_KEY_TOKEN";
        case YAML_VALUE_TOKEN:
            return "YAML_VALUE_TOKEN";
        case YAML_ALIAS_TOKEN:
            return "YAML_ALIAS_TOKEN";
        case YAML_ANCHOR_TOKEN:
            return "YAML_ANCHOR_TOKEN";
        case YAML_TAG_TOKEN:
            return "YAML_TAG_TOKEN";
        case YAML_SCALAR_TOKEN:
            return "YAML_SCALAR_TOKEN";
        default:
            return "uNrEcOgNiZeD tOkEn TyPe (corruption?)";
    }
}

/* Output token is a shallow copy - don't free it. */
static bool peek(context *c, yaml_token_t *token)
{
    if (!c->have_lookahead) {
        if (yaml_parser_scan(c->parser, &c->token) == 0) {
            warnx(_("*** %s: broken document"), __func__);
            return true;
        }

        dprintf("> %s", tokname(c->token.type));

        if (c->token.type == YAML_SCALAR_TOKEN) {
            dprintf(" \"%*s\"", (int)c->token.data.scalar.length, c->token.data.scalar.value);
        }

        dprintf("\n");
        c->have_lookahead = true;
    }

    memcpy(token, &c->token, sizeof(*token));
    return false;
}

static void next(context *c, yaml_token_t *token)
{
    if (c->have_lookahead) {
        c->have_lookahead = false;
        memcpy(token, &c->token, sizeof(*token));
        return;
    } else if (yaml_parser_scan(c->parser, token) == 0) {
        warnx(_("*** %s: broken document"), __func__);
        exit(1);
    }

    dprintf("> %s", tokname(token->type));

    if (token->type == YAML_SCALAR_TOKEN) {
        dprintf(" \"%*s\"", (int)token->data.scalar.length, token->data.scalar.value);
    }

    dprintf("\n");
    return;
}

static void wait_for(context *c, yaml_token_t *token, yaml_token_type_t target)
{
    next(c, token);

    while (token->type != YAML_NO_TOKEN && token->type != target) {
        warnx(_("*** skipping token type %s"), tokname(token->type));
        yaml_token_delete(token);
        next(c, token);
    }
}

/*
 * Terminology:
 * - "block" is the weird yaml-y way of writing things; "flow" is json
 * - "sequence" is array; "mapping" is dict
 *
 * I don't keep track of the difference between block and flow because this is
 * a *parser* and *that's the point*.
 */
static y_value *p_value(context *context)
{
    yaml_token_t token = { 0 }, ptoken = { 0 };
    y_value *ret = NULL;
    size_t i = 0;
    bool implicit = false;

    while (1) {
        next(context, &token);

        if (token.type == YAML_BLOCK_END_TOKEN
            || token.type == YAML_FLOW_SEQUENCE_END_TOKEN
            || token.type == YAML_FLOW_MAPPING_END_TOKEN
            || token.type == YAML_NO_TOKEN) {
            /* 4 ways to start a block, but only 3 to end it.  Great job. */
            goto done;
        } else if (token.type == YAML_FLOW_ENTRY_TOKEN
                   || token.type == YAML_DOCUMENT_START_TOKEN) {
            /* These tokens don't mean anything to us. */
            continue;
        } else if (token.type == YAML_SCALAR_TOKEN) {
            ret = xalloc(sizeof(*ret));
            ret->type = Y_STRING;
            ret->v.string = strndup((char *)token.data.scalar.value, token.data.scalar.length);
            assert(ret->v.string);

            goto done;
        } else if (token.type == YAML_BLOCK_MAPPING_START_TOKEN
                   || token.type == YAML_FLOW_MAPPING_START_TOKEN) {
            ret = xalloc(sizeof(*ret));
            ret->type = Y_DICT;
            ret->v.dict.keys = xalloc(sizeof(ret->v.dict.keys));
            ret->v.dict.values = xalloc(sizeof(ret->v.dict.values));

            while (1) {
                yaml_token_delete(&token);
                next(context, &token);

                if (token.type == YAML_BLOCK_END_TOKEN
                    || token.type == YAML_FLOW_MAPPING_END_TOKEN) {
                    /* Friggin YAML, I tell you hwat. */
                    break;
                } else if (token.type != YAML_KEY_TOKEN) {
                    fprintf(stderr, "expected YAML_KEY_TOKEN, got %s\n", tokname(token.type));
                    free(ret);
                    ret = NULL;
                    goto done;
                }

                yaml_token_delete(&token);
                wait_for(context, &token, YAML_SCALAR_TOKEN);

                /* We're going to write to i. */
                ret->v.dict.keys = xrealloc(ret->v.dict.keys, (i + 2) * sizeof(ret->v.dict.keys));
                ret->v.dict.keys[i + 1] = NULL;
                ret->v.dict.values = xrealloc(ret->v.dict.values, (i + 2) * sizeof(ret->v.dict.values));
                ret->v.dict.values[i + 1] = NULL;
                ret->v.dict.keys[i] = strndup((char *)token.data.scalar.value, token.data.scalar.length);
                assert(ret->v.dict.keys[i]);
                yaml_token_delete(&token);
                wait_for(context, &token, YAML_VALUE_TOKEN);

                if (peek(context, &ptoken)) {
                    free(ret);
                    ret = NULL;
                    goto done;
                }

                if (ptoken.type == YAML_KEY_TOKEN) {
                    /*
                     * And this right here is the reason we need a
                     * lookahead (key, scalar, value, key).  Really
                     * dubious behavior.
                     */
                    ret->v.dict.values[i] = NULL;
                } else {
                    ret->v.dict.values[i] = p_value(context);

                    if (ret->v.dict.values[i] == NULL) {
                        /* Recursion eats the block end. */
                        goto done;
                    }
                }

                i++;
            }

            goto done;
        } else if (token.type == YAML_BLOCK_SEQUENCE_START_TOKEN
                   || token.type == YAML_FLOW_SEQUENCE_START_TOKEN
                   || token.type == YAML_BLOCK_ENTRY_TOKEN) {
            if (token.type == YAML_BLOCK_ENTRY_TOKEN) {
                implicit = true;
            }

            ret = xalloc(sizeof(*ret));
            ret->type = Y_ARRAY;

            while (1) {
                /* Allocation strategy same as dict case. */
                ret->v.array = xrealloc(ret->v.array, (i + 2) * sizeof(ret->v.array));
                ret->v.array[i + 1] = NULL;

                if (peek(context, &ptoken)) {
                    free(ret);
                    ret = NULL;
                    goto done;
                }

                if (ptoken.type == YAML_BLOCK_ENTRY_TOKEN) {
                    /* Sigh.  This type is useless, and optional. */
                    yaml_token_delete(&token);
                    wait_for(context, &token, YAML_BLOCK_ENTRY_TOKEN);
                } else if (ptoken.type == YAML_KEY_TOKEN) {
                    /* Oh hello there, implicit block end. */
                    goto done;
                } else if (implicit && ptoken.type == YAML_BLOCK_END_TOKEN) {
                    /* This doesn't belong to us. */
                    goto done;
                }

                ret->v.array[i] = p_value(context);

                if (ret->v.array[i] == NULL) {
                    /* Yes, it's slightly larger.  No, I don't care. */
                    break;
                }

                i++;
            }

            goto done;
        } else {
            warnx(_("*** skipping token type %s"), tokname(token.type));
            free(ret);
            ret = NULL;
            goto done;
        }

        yaml_token_delete(&token);
    }

done:
    yaml_token_delete(&token);
    return ret;
}

static bool yaml_parse_file(parser_context **context_out, const char *filename)
{
    FILE *fp = NULL;
    yaml_parser_t parser = { 0 };
    yaml_token_t token = { 0 };
    context context = { 0 };
    y_value *value = NULL;

    if (filename == NULL) {
        return true;
    }

    /* prepare a YAML parser */
    if (!yaml_parser_initialize(&parser)) {
        warnx("*** yaml_parser_initialize");
        return true;
    }

    context.parser = &parser;
    fp = fopen(filename, "r");

    if (fp == NULL) {
        warn("*** fopen");
        return true;
    }

    yaml_parser_set_input_file(&parser, fp);

    /* I don't want to handle aliases.  Don't do that. */
    wait_for(&context, &token, YAML_STREAM_START_TOKEN);
    yaml_token_delete(&token);

    value = p_value(&context);
    yaml_parser_delete(&parser);
    fclose(fp);

    if (value == NULL) {
        return true;
    }

    *context_out = (parser_context *)value;
    return false;
}

static void y_free_tree(y_value *v)
{
    size_t i = 0;

    if (v == NULL || v->type == Y_UNINITIALIZED) {
        return;
    } else if (v->type == Y_STRING) {
        free(v->v.string);
    } else if (v->type == Y_ARRAY) {
        for (i = 0; v->v.array[i] != NULL; i++) {
            y_free_tree(v->v.array[i]);
        }

        free(v->v.array);
    } else if (v->type == Y_DICT) {
        for (i = 0; v->v.dict.keys[i] != NULL; i++) {
            free(v->v.dict.keys[i]);
            y_free_tree(v->v.dict.values[i]);
        }

        free(v->v.dict.keys);
        free(v->v.dict.values);
    } else {
        warnx(_("*** malformed type - freed?"));
        return;
    }

    v->type = Y_UNINITIALIZED;
    free(v);
    return;
}

static void yaml_fini(parser_context *context)
{
    y_free_tree((void *) context);
    return;
}

static y_value *getobj(y_value *y, const char *key1, const char *key2)
{
    size_t i = 0;
    if (key1 == NULL) {
        assert(key2 == NULL);
        return y;
    } else if (y == NULL || y->type != Y_DICT) {
        return NULL;
    }

    for (i = 0; y->v.dict.keys[i] != NULL; i++) {
        if (!strcmp(key1, y->v.dict.keys[i])) {
            return getobj(y->v.dict.values[i], key2, NULL);
        }
    }

    return NULL;
}

static bool yaml_have_section(parser_context *context, const char *section)
{
    size_t i = 0;
    y_value *y = (y_value *) context;

    if (y == NULL || section == NULL) {
        return false;
    }

    for (i = 0; y->v.dict.keys[i] != NULL; i++) {
        if (!strcmp(section, y->v.dict.keys[i])) {
            return true;
        }
    }

    return false;
}

static char *yaml_getstr(parser_context *context, const char *key1, const char *key2)
{
    y_value *y = (y_value *) context;
    y_value *cur = NULL;

    cur = getobj(y, key1, key2);

    if (cur == NULL || cur->type != Y_STRING) {
        return NULL;
    }

    return strdup(cur->v.string);
}

static bool yaml_strarray_foreach(parser_context *context, const char *key1, const char *key2, parser_strarray_entry_fn lambda, void *cb_data)
{
    size_t i = 0;
    y_value *y = (y_value *) context;
    const y_value *arrobj = NULL;

    arrobj = getobj(y, key1, key2);

    if (arrobj == NULL) {
        return false;
    } else if (arrobj->type != Y_ARRAY) {
        return true;
    }

    for (i = 0; arrobj->v.array[i] != NULL; i++) {
        if (arrobj->v.array[i]->type != Y_STRING || lambda(arrobj->v.array[i]->v.string, cb_data)) {
            return true;
        }
    }

    return false;
}

static bool yaml_strdict_foreach(parser_context *context, const char *key1, const char *key2, parser_strdict_entry_fn lambda, void *cb_data)
{
    size_t i = 0;
    size_t j = 0;
    y_value *y = (y_value *) context;
    const y_value *dictobj = NULL;
    const y_value *arrayobj = NULL;
    const y_value *val = NULL;

    dictobj = getobj(y, key1, key2);

    if (dictobj == NULL) {
        return false;
    } else if (dictobj->type == Y_DICT) {
        for (i = 0; dictobj->v.dict.keys[i] != NULL; i++) {
            val = dictobj->v.dict.values[i];

            if (val == NULL) {
                return true;
            } else if (val->type == Y_ARRAY) {
                /* handle dict members with an array of strings as the value */
                for (j = 0; val->v.array[j] != NULL; j++) {
                    arrayobj = val->v.array[j];

                    if (arrayobj == NULL || arrayobj->type != Y_STRING || lambda(dictobj->v.dict.keys[i], arrayobj->v.string, cb_data)) {
                        return true;
                    }
                }
            } else if (val->type == Y_DICT || lambda(dictobj->v.dict.keys[i], val->v.string, cb_data)) {
                return true;
            }
        }

        return false;
    } else if (dictobj->type != Y_ARRAY) {
        return true;
    }

    /*
     * Handle an array of single-element dicts as if it were a single
     * dict for backward compatibility with the old parser.
     */
    arrayobj = dictobj;

    for (i = 0; arrayobj->v.array[i] != NULL; i++) {
        dictobj = arrayobj->v.array[i];

        /* Make sure it has only one element; else it's just malformed. */
        if (dictobj->type != Y_DICT || (dictobj->v.dict.keys[0] != NULL && dictobj->v.dict.keys[1] != NULL)) {
            return true;
        }

        val = dictobj->v.dict.values[0];

        if (val == NULL || val->type != Y_STRING || lambda(dictobj->v.dict.keys[0], val->v.string, cb_data)) {
            return true;
        }
    }

    return false;
}

static bool yaml_keymap(parser_context *context, const char *key1, const char *key2, parser_keymap_key_fn lambda, void *cb_data)
{
    size_t i = 0;
    y_value *y = (y_value *) context;
    const y_value *dictobj = NULL;

    dictobj = getobj(y, key1, key2);

    if (dictobj == NULL) {
        return false;
    } else if (dictobj->type != Y_DICT) {
        return true;
    }

    for (i = 0; dictobj->v.dict.keys[i] != NULL; i++) {
        if (lambda(dictobj->v.dict.keys[i], cb_data)) {
            return true;
        }
    }

    return false;
}

parser_plugin yaml_parser = {
    .name = "yaml",
    .parse_file = yaml_parse_file,
    .fini = yaml_fini,
    .havesection = yaml_have_section,
    .getstr = yaml_getstr,
    .strarray_foreach = yaml_strarray_foreach,
    .strdict_foreach = yaml_strdict_foreach,
    .keymap = yaml_keymap,
};
