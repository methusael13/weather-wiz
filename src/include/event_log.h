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

#ifndef _EVENT_LOG_
#define _EVENT_LOG_

#include <stdio.h>

/* Colors for Log types. Meant for terminal emulators */
#define ERROR_COL "\x1b[31;1m"
#define WARNING_COL "\x1b[33;1m"
#define INFO_COL "\x1b[32;1m"

#define LOG(COL, M) \
        do { \
            fprintf(stderr, "" COL "[%s:%d]\x1b[0m " M "\n", \
                    __FILE__, __LINE__); \
        } while (0)

#define LOG_ARG(COL, M, ...) \
        do { \
            fprintf(stderr, "" COL "[%s:%d]\x1b[0m " M "\n", \
                    __FILE__, __LINE__, __VA_ARGS__); \
        } while (0)

/* Logger functions. Logs a single string */
#define LOG_ERROR(M) LOG(ERROR_COL, M)
#define LOG_WARNING(M) LOG(WARNING_COL, M)
#define LOG_INFO(M) LOG(INFO_COL, M)

/* Logger functions same as above but with varags after the string */
#define LOG_ARG_ERROR(M, ...) LOG_ARG(ERROR_COL, M, __VA_ARGS__)
#define LOG_ARG_WARNING(M, ...) LOG_ARG(WARNING_COL, M, __VA_ARGS__)
#define LOG_ARG_INFO(M, ...) LOG_ARG(INFO_COL, M, __VA_ARGS__)


#endif
