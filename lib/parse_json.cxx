/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <inttypes.h>
#include <json.h>
#include "parser.h"
#include "rpminspect.h"
#include "stdbool.h"

static bool json_parse_file(parser_context **context_out, const char *filepath)
{
    char *buf = NULL;
    off_t len = 0;
    json_tokener *tok = NULL;
    enum json_tokener_error jerr = json_tokener_success;
    json_object *jo = NULL;
    bool res = true;

    buf = read_file_bytes(filepath, &len);
    tok = json_tokener_new();
    assert(tok != NULL);
    jo = json_tokener_parse_ex(tok, buf, len);
    jerr = json_tokener_get_error(tok);

    if (jerr != json_tokener_success) {
        warnx("*** json_tokener_parse_ex: %s", json_tokener_error_desc(jerr));
        goto done;
    }

    *context_out = (parser_context *)jo;
    res = false;
done:
    free(buf);
    json_tokener_free(tok);
    return res;
}

static void json_fini(parser_context *context)
{
    json_object *jo = (json_object *) context;

    json_object_put(jo);
    return;
}

/* Nice typesystem you've got there.  Mind if I ignore it? */
static char *as_str(json_object *jo)
{
    json_type type = json_type_null;
    char *out = NULL;
    const char *s = NULL;

    type = json_object_get_type(jo);

    if (type == json_type_boolean) {
        if (json_object_get_boolean(jo)) {
            return strdup("true");
        }

        return strdup("false");
    } else if (type == json_type_double) {
        xasprintf(&out, "%f", json_object_get_double(jo));
        return out;
    } else if (type == json_type_int) {
        xasprintf(&out, "%"PRId32, json_object_get_int(jo));
        return out;
    } else if (type == json_type_string) {
        s = json_object_get_string(jo);

        if (!strcmp(s, "")) {
            return NULL;
        } else {
            return strdup(s);
        }
    }

    return NULL;
}

static json_object *getobj(json_object *jo, const char *key1, const char *key2)
{
    json_object *cont = NULL;

    if (key1 == NULL) {
        assert(key2 == NULL);
        return jo;
    } else if (!json_object_is_type(jo, json_type_object)) {
        return NULL;
    }

    if (!json_object_object_get_ex(jo, key1, &cont)) {
        return NULL;
    }

    return getobj(cont, key2, NULL);
}

static bool json_have_section(parser_context *context, const char *section)
{
    json_object *jo = (json_object *) context;
    json_object *cont = NULL;

    if (jo == NULL || section == NULL) {
        return false;
    }

    if (!json_object_is_type(jo, json_type_object)) {
        return false;
    }

    return json_object_object_get_ex(jo, section, &cont);
}

static char *json_getstr(parser_context *context, const char *key1, const char *key2)
{
    json_object *jo = (json_object *) context;
    json_object *cur = NULL;

    cur = getobj(jo, key1, key2);

    if (cur == NULL) {
        return NULL;
    }

    return as_str(cur);
}

static bool json_strarray_foreach(parser_context *context, const char *key1, const char *key2, parser_strarray_entry_fn lambda, void *cb_data)
{
    size_t i = 0;
    json_object *jo = (json_object *) context;
    json_object *arrayobj = NULL, *cur = NULL;
    size_t bound = 0;
    char *selt = NULL;

    arrayobj = getobj(jo, key1, key2);

    if (arrayobj == NULL) {
        return false;
    } else if (!json_object_is_type(arrayobj, json_type_array)) {
        return true;
    }

    bound = json_object_array_length(arrayobj);

    for (i = 0; i < bound; i++) {
        cur = json_object_array_get_idx(arrayobj, i);
        selt = as_str(cur);

        if (selt == NULL || lambda(selt, cb_data)) {
            free(selt);
            return true;
        }

        free(selt);
    }

    return false;
}

static bool json_strdict_foreach(parser_context *context, const char *key1, const char *key2, parser_strdict_entry_fn lambda, void *cb_data)
{
    json_object *jo = (json_object *) context;
    json_object *dictobj = NULL;
    char *valstr = NULL;

    dictobj = getobj(jo, key1, key2);

    if (dictobj == NULL) {
        return false;
    } else if (!json_object_is_type(dictobj, json_type_object)) {
        return true;
    }

    json_object_object_foreach(dictobj, key, value) {
        valstr = as_str(value);

        if (valstr == NULL || lambda(key, valstr, cb_data)) {
            free(valstr);
            return true;
        }
    }

    return false;
}

static bool json_keymap(parser_context *context, const char *key1, const char *key2, parser_keymap_key_fn lambda, void *cb_data)
{
    json_object *jo = (json_object *) context;
    json_object *dictobj = NULL;

    dictobj = getobj(jo, key1, key2);

    if (dictobj== NULL) {
        return false;
    } else if (!json_object_is_type(dictobj, json_type_object)) {
        return true;
    }

    json_object_object_foreach(dictobj, key, value) {
        (void)value; /* json_object_object_foreach is a macro. */

        if (lambda(key, cb_data)) {
            return true;
        }
    }

    return false;
}

parser_plugin json_parser = {
    .name = "json",
    .parse_file = json_parse_file,
    .fini = json_fini,
    .havesection = json_have_section,
    .getstr = json_getstr,
    .strarray_foreach = json_strarray_foreach,
    .strdict_foreach = json_strdict_foreach,
    .keymap = json_keymap,
};
