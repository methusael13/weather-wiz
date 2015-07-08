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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define _REQUIRE_MEM_PERMISSIONS_
#include "include/util.h"
#include "include/event_log.h"

int8_t
file_exists_readable(const char *fpath) {
    struct stat file_stat;

    if (stat(fpath, &file_stat) == -1 ||
        file_stat.st_size == 0 ||
        (file_stat.st_mode & S_IFMT) != S_IFREG )
        return FALSE;

    return TRUE;
}

const char *
get_usr_home_dir(void) {
    char *usr_home;

    if ((usr_home = getenv("HOME")) == NULL)
        usr_home = ((struct passwd *)getpwuid(getuid()))->pw_dir;
    return usr_home;
}

size_t
get_str_from_stream(char **strptr, size_t sz, FILE *stream) {
    char *line = NULL;
    ssize_t read;
    size_t n, size;

    if (*strptr == NULL) {
        *strptr = (char *) malloc(BUFFER_SZ);
        sz = BUFFER_SZ;
    }

    n = 0;
    /* Make sure stream always points to the beginning of file */
    rewind(stream);
    while ((read = getline(&line, &size, stream)) != -1) {
        if (sz - n <= read) {
            *strptr = (char *) realloc(*strptr, n + 2*read);
            sz = n + 2*read;
        }
        /* Copy 'read + 1' bytes of data including the null byte */
        memcpy(*strptr + n, line, read + 1);
        n += read;
    }

    /* Trim extra bytes from strptr */
    *strptr = (char *) realloc(*strptr, n + 1);
    free(line);

    return n;
}

size_t
get_str_from_path(char **strptr, size_t sz, const char *fpath) {
    FILE *stream;
    if ((stream = fopen(fpath, "r")) == NULL) {
        LOG_ARG_WARNING("Unable to read from %s", fpath);
        return 0;
    }

    size_t size;
    size = get_str_from_stream(strptr, sz, stream);

    fclose(stream);
    return size;
}

int
write_str_to_file_path(const char *str, const char *fpath) {
    FILE *stream;
    if ((stream = fopen(fpath, "w")) == NULL) {
        LOG_ARG_WARNING("Unable to write to %s", fpath);
        return -1;
    }

    int res;
    res = fputs(str, stream);
    fclose(stream);

    return res;
}

#ifdef _REQUIRE_MEM_PERMISSIONS_
#include <sys/mman.h>

int
set_write_permission(const void *ptr, size_t len) {
    int pagesize;
    void *page_bound;
    uint64_t page_offset;

    if ((pagesize = sysconf(_SC_PAGESIZE)) < 1)
        return FALSE;

    page_offset = (uint64_t)ptr % (uint64_t)pagesize;
    page_bound = (char *)ptr - page_offset;
    if (mprotect(page_bound, len + page_offset,
        (PROT_READ | PROT_WRITE | PROT_EXEC)) == -1)
        return FALSE;
    return TRUE;
}
#endif
