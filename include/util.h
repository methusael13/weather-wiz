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

#ifndef _UTIL_
#define _UTIL_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if !defined(TRUE) && !defined(FALSE)
    #define TRUE 1
    #define FALSE 0
#endif
#define URL_SIZE 256
#define PATH_SIZE 512
#define HTTP_STATUS_OK 200

// 50 KBs of buffer size
#define BUFFER_SZ (50 * 1024)
#define CHECK_REF(ptr, err_label) \
    if (!ptr) { goto err_label; }
#define CHECK_COND(condition, op, err_label) \
    if (condition) { \
        op; \
    } else { goto err_label; }
#define CHECK_COND_OP(condition, op1, op2) \
    if (condition) { \
        op1; \
    } else { op2; }

#define W_N_ELEMENTS(arr) (sizeof(arr)/sizeof(*arr))


int8_t
file_exists_readable(const char *fname);

const char *
get_usr_home_dir(void);

size_t
get_str_from_stream(char **strptr, size_t sz, FILE *stream);

size_t
get_str_from_path(char **strptr, size_t sz, const char *fpath);

int
write_str_to_file_path(const char *str, const char *fpath);

#ifdef _REQUIRE_MEM_PERMISSIONS_
int
set_write_permission(const void *ptr, size_t len);
#endif

#endif
