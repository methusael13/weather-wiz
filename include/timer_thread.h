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

#ifndef _TIMER_THREAD_
#define _TIMER_THREAD_

#include <signal.h>
#include <stdint.h>
#include <pthread.h>

#define TIMER_RUNNING 0x11
#define TIMER_STOPPED 0x22
#define TIMER_EXECUTING 0x33

typedef uint16_t time_v;

typedef struct Timer {
    time_v delay;
    time_v interval;
    pthread_t thread;
    void (*callback_func)(void);
    volatile sig_atomic_t status;
} TimerThread;

sig_atomic_t
get_timer_status(TimerThread *timer);

TimerThread *
timer_thread_create(time_v delay, time_v interval,
    void (*on_timer_event)(void));

void
timer_thread_start(TimerThread *timer);

void
timer_thread_stop(TimerThread *timer);

void
timer_thread_destroy(TimerThread *timer);

#endif
