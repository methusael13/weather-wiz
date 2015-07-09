#include <stdio.h>
#include <unistd.h>
#include "timer_thread.h"

int count = 0;

void
handle_alarm(void) {
    printf("\x1b[s\x1b[K");
    printf("\tAlarm \x1b[31;1m%d\x1b[0m\n", ++count);
    printf("\x1b[u");
}

const char *
get_timer_status_str(int stat) {
    switch (stat) {
        case TIMER_RUNNING:
            return "TIMER_RUNNING";
        case TIMER_EXECUTING:
            return "TIMER_EXECUTING";
        case TIMER_STOPPED:
            return "TIMER_STOPPED";
        default:
            return NULL;
    }
}

int
main(int argc, char *argv[]) {
    TimerThread *timer;
    timer = timer_thread_create(1, 1, handle_alarm);

    timer_thread_start(timer);
    printf("Timer status: %s\n",
        get_timer_status_str(get_timer_status(timer)));

    while (count != 20) usleep(10);
    printf("Requesting timer stop\n");
    timer_thread_stop(timer);
    printf("Timer status: %s\n",
        get_timer_status_string(get_timer_status(timer)));

    timer_thread_destroy(timer);
    return 0;
}
