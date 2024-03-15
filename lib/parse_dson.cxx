/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/* such software.  many freedoms. */

#include <assert.h>
#include <cdson.h>
#include <err.h>
#include <math.h>
#include "parser.h"
#include "rpminspect.h"

static bool dson_parse_file(parser_context **context_out, const char *filepath)
{
    char *errmsg = NULL, *buf = NULL;
    dson_value *out = NULL;
    off_t len = 0;

    buf = read_file_bytes(filepath, &len);
    errmsg = dson_parse(buf, len, false, &out);
    free(buf);

    if (errmsg != NULL) {
        warnx("*** %s", errmsg);
        free(errmsg);
        return true;
    }

    *context_out = (parser_context *) out;
    return false;
}

static void dson_fini(parser_context *context)
{
    dson_value *tree = (dson_value *) context;

    dson_free(&tree);
    return;
}

static dson_value *getobj(dson_value *tree, const char *key1, const char *key2)
{
    size_t i = 0;

    if (key1 == NULL) {
        assert(key2 == NULL);
        return tree;
    } else if (tree->type != DSON_DICT) {
        return NULL;
    }

    for (i = 0; tree->dict->keys[i] != NULL; i++) {
        if (!strcmp(key1, tree->dict->keys[i])) {
            return getobj(tree->dict->values[i], key2, NULL);
        }
    }

    return NULL;
}

/* many yaml.  what is types.  wow */
static char *as_str(dson_value *v)
{
    char *s = NULL;
    double integ = 0;
    double frac = 0;

    if (v == NULL) {
        return NULL;
    } else if (v->type == DSON_STRING) {
        if (!strcmp(v->s, "")) {
            return NULL;
        } else {
            return strdup(v->s);
        }
    } else if (v->type == DSON_BOOL) {
        if (v->b) {
            return strdup("true");
        } else {
            return strdup("false");
        }
    } else if (v->type == DSON_DOUBLE) {
        /* such extra digits.  delight.  printf sad shibe */
        frac = modf(v->n, &integ);

        if (frac == 0) {
            xasprintf(&s, "%ld", (unsigned long)integ);
        } else {
            xasprintf(&s, "%f", v->n);
        }

        return s;
    }

    return NULL;
}

static bool dson_have_section(parser_context *context, const char *section)
{
    size_t i = 0;
    dson_value *tree = (dson_value *) context;

    if (tree == NULL || section == NULL) {
        return false;
    }

    if (tree->type != DSON_DICT) {
        return false;
    }

    for (i = 0; tree->dict->keys[i] != NULL; i++) {
        if (!strcmp(section, tree->dict->keys[i])) {
            return true;
        }
    }

    return false;
}

static char *dson_getstr(parser_context *context, const char *key1, const char *key2)
{
    dson_value *tree = (dson_value *) context;

    return as_str(getobj(tree, key1, key2));
}

static bool dson_strarray_foreach(parser_context *context, const char *key1, const char *key2, parser_strarray_entry_fn lambda, void *cb_data)
{
    size_t i = 0;
    dson_value *tree = (dson_value *) context;
    dson_value *arrobj = NULL;
    char *cur = NULL;

    arrobj = getobj(tree, key1, key2);

    if (arrobj == NULL) {
        return false;
    } else if (arrobj->type != DSON_ARRAY) {
        return true;
    }

    for (i = 0; arrobj->array[i] != NULL; i++) {
        cur = as_str(arrobj->array[i]);

        if (cur == NULL || lambda(cur, cb_data)) {
            free(cur);
            return true;
        }

        free(cur);
    }

    return false;
}

static bool dson_strdict_foreach(parser_context *context, const char *key1, const char *key2, parser_strdict_entry_fn lambda, void *cb_data)
{
    size_t i = 0;
    dson_value *tree = (dson_value *) context;
    dson_value *dictobj = NULL;
    dson_dict *dict = NULL;
    char *cur = NULL;

    dictobj = getobj(tree, key1, key2);

    if (dictobj == NULL) {
        return false;
    } else if (dictobj->type != DSON_DICT) {
        return true;
    }

    dict = dictobj->dict;

    for (i = 0; dict->keys[i] != NULL; i++) {
        cur = as_str(dict->values[i]);

        if (cur == NULL || lambda(dict->keys[i], cur, cb_data)) {
            free(cur);
            return true;
        }

        free(cur);
    }

    return false;
}

static bool dson_keymap(parser_context *context, const char *key1, const char *key2, parser_keymap_key_fn lambda, void *cb_data)
{
    size_t i = 0;
    dson_value *tree = (dson_value *) context;
    dson_value *dictobj = NULL;

    dictobj = getobj(tree, key1, key2);

    if (dictobj == NULL) {
        return false;
    } else if (dictobj->type != DSON_DICT) {
        return true;
    }

    for (i = 0; dictobj->dict->keys[i] != NULL; i++) {
        if (lambda(dictobj->dict->keys[i], cb_data)) {
            return true;
        }
    }

    return false;
}

parser_plugin dson_parser = {
    .name = "dson",
    .parse_file = dson_parse_file,
    .fini = dson_fini,
    .havesection = dson_have_section,
    .getstr = dson_getstr,
    .strarray_foreach = dson_strarray_foreach,
    .strdict_foreach = dson_strdict_foreach,
    .keymap = dson_keymap,
};
