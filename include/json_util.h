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

#ifndef _JSON_UTIL_
#define _JSON_UTIL_

#include <stdlib.h>
#include <jansson.h>

/* All the function overloading done here */
int
json_load_data_obj(json_t *root, const char *key, json_t **dst);

int
json_load_data_arr(json_t *root, const char *key, json_t **dst);

int
json_load_data_string(json_t *root, const char *key, char *dst, size_t size);

int
json_load_data_string_nc(json_t *root, const char *key, json_t **dst);

int
json_load_data_int(json_t *root, const char *key, int *dst);

int
json_load_data_float(json_t *root, const char *key, float *dst);

int
json_load_data_bool(json_t *root, const char *key, int *dst);

int
json_load_data_to_string(json_t *root, const char *key, char *dst, size_t size);

json_t *
json_get_leaf_object(json_t *root, const char *key);

#endif
