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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "timer_thread.h"

TimerThread *local_timer;
pthread_mutex_t mutex_lock;
pthread_cond_t status_cond;

sig_atomic_t
get_timer_status(TimerThread *timer) {
    sig_atomic_t status;
    pthread_mutex_lock(&mutex_lock);
    status = timer->status;
    pthread_mutex_unlock(&mutex_lock);

    return status;
}

static void
set_timer_status(TimerThread *timer, sig_atomic_t status) {
    pthread_mutex_lock(&mutex_lock);
    timer->status = status;
    pthread_mutex_unlock(&mutex_lock);
}

static void
local_timer_handler(int signum) {
    set_timer_status(local_timer, TIMER_EXECUTING);
    local_timer->callback_func();
    set_timer_status(local_timer, TIMER_RUNNING);
}

static void *
timer_thread_process(void *arg) {
    TimerThread *timer = (TimerThread *) arg;
    struct sigaction sa;
    struct itimerval timer_val;

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = local_timer_handler;
    sigaction(SIGALRM, &sa, NULL);

    /* Initiate timer intervals */
    memset(&timer_val, 0, sizeof timer_val);
    timer_val.it_value.tv_sec = timer->delay;
    timer_val.it_interval.tv_sec = timer->interval;
    setitimer(ITIMER_REAL, &timer_val, NULL);

    /* Try locking mutex and make the status change */
    pthread_mutex_trylock(&mutex_lock);
    timer->status = TIMER_RUNNING;
    pthread_mutex_unlock(&mutex_lock);

    /* Notify and wake up waiting threads */
    pthread_cond_signal(&status_cond);

    /* Keep the thread alive until timer has stopped,
     * and sleep, so as to prevent blocking CPU context switches */
    while (get_timer_status(timer) != TIMER_STOPPED)
        usleep(200);
    return NULL;
}

TimerThread *
timer_thread_create(time_v delay, time_v interval,
    void (*on_timer_event)(void)) {
    TimerThread *timer = (TimerThread *) malloc(sizeof(TimerThread));
    timer->delay = delay;
    timer->interval = interval;
    timer->callback_func = on_timer_event;

    pthread_mutex_init(&mutex_lock, NULL);
    set_timer_status(timer, TIMER_STOPPED);

    return timer;
}

void
timer_thread_start(TimerThread *timer) {
    if (get_timer_status(timer) != TIMER_STOPPED)
        return;

    local_timer = timer;
    pthread_cond_init(&status_cond, NULL);
    pthread_create(&timer->thread, NULL, timer_thread_process, timer);

    /* Wait for timer->thread to change the status to TIMER_RUNNING */
    pthread_mutex_lock(&mutex_lock);
    pthread_cond_wait(&status_cond, &mutex_lock);
    pthread_mutex_unlock(&mutex_lock);

    pthread_cond_destroy(&status_cond);
}

void
timer_thread_stop(TimerThread *timer) {
    if (!timer || get_timer_status(timer) == TIMER_STOPPED)
        return;

    set_timer_status(timer, TIMER_STOPPED);
    /* Wait for thread to actually terminate */
    pthread_join(timer->thread, NULL);
}

void
timer_thread_destroy(TimerThread *timer) {
    if (!timer) return;

    if (get_timer_status(timer) != TIMER_STOPPED)
        timer_thread_stop(timer);

    pthread_mutex_destroy(&mutex_lock);
    free(timer);
    local_timer = NULL;
}
