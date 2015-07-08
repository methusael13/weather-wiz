/*
An Indicator to display local weather information

The MIT License (MIT)

Copyright (c) 2015 Methusael Murmu
Authors:
    Methusael Murmu <blendit07@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <string.h>
#include "include/event_log.h"
#include "include/json_util.h"
#include "include/util.h"

/* Helper macros */
#define CHECK_REF_PARAM(r, k, d, e) \
    CHECK_REF(r, e); \
    CHECK_REF(k, e); \
    CHECK_REF(d, e);
#define ERROR_LABEL(e) \
    e: \
        return FALSE;

#define GET_KEY_OBJECT(root, key, dst, err, tmp) \
    CHECK_REF_PARAM(root, key, dst, err); \
    json_t *tmp = json_get_leaf_object(root, key); \
    CHECK_REF(tmp, err);
#define FUNC_FOOTER(err) \
    return TRUE; \
    ERROR_LABEL(err);

json_t *
json_get_leaf_object(json_t *root, const char *key) {
    CHECK_REF(root, err);
    CHECK_REF(key, err);

    json_t *tmp;
    char *tok, *_key, *ptr;
    const char *delim = ".";

    /* Make a copy, because key is read-only */
    _key = strdup(key);
    tok = strtok_r(_key, delim, &ptr);
    tmp = json_object_get(root, tok);
    CHECK_REF(tmp, err);

    while ((tok = strtok_r(NULL, delim, &ptr)) != NULL) {
        tmp = json_object_get(tmp, tok);
        CHECK_REF(tmp, err);
    }

    free(_key);
    return tmp;
    err:
        if (_key) free(_key);
        return NULL;
}

int
json_load_data_obj(json_t *root, const char *key, json_t **dst) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_object(tmp), *dst = tmp, err);
    FUNC_FOOTER(err);
}

int
json_load_data_arr(json_t *root, const char *key, json_t **dst) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_array(tmp), *dst = tmp, err);
    FUNC_FOOTER(err);
}

int
json_load_data_string(json_t *root, const char *key, char *dst, size_t size) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_string(tmp),
        { strncpy(dst, json_string_value(tmp), size); }, err);
    FUNC_FOOTER(err);
}

int
json_load_data_to_string(json_t *root, const char *key, char *dst, size_t size) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);

    switch (json_typeof(tmp)) {
        case JSON_STRING:
            strncpy(dst, json_string_value(tmp), size);
            break;
        case JSON_INTEGER:
            snprintf(dst, size, "%d", (int) json_integer_value(tmp));
            break;
        case JSON_REAL:
            snprintf(dst, size, "%.1f", (float) json_real_value(tmp));
            break;
        case JSON_TRUE:
            memcpy(dst, "TRUE", 5);
            break;
        case JSON_FALSE:
            memcpy(dst, "FALSE", 6);
            break;
        case JSON_NULL:
            memcpy(dst, "NULL", 5);
        default:
            return FALSE;
    }

    FUNC_FOOTER(err);
}

int
json_load_data_string_nc(json_t *root, const char *key, json_t **dst) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_string(tmp), *dst = tmp, err);
    FUNC_FOOTER(err);
}

int
json_load_data_int(json_t *root, const char *key, int *dst) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_number(tmp), *dst = (int)json_number_value(tmp), err);
    FUNC_FOOTER(err);
}

int
json_load_data_float(json_t *root, const char *key, float *dst) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_number(tmp), *dst = (float)json_number_value(tmp), err);
    FUNC_FOOTER(err);
}

int
json_load_data_bool(json_t *root, const char *key, int *dst) {
    GET_KEY_OBJECT(root, key, dst, err, tmp);
    CHECK_COND(json_is_boolean(tmp), *dst = json_is_true(tmp), err);
    FUNC_FOOTER(err);
}
