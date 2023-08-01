#define LOG_TAG "libqti-perfd-client"

#include <stdint.h>
#include <log/log.h>

void perf_get_feedback() {}

void perf_hint() {}

int perf_lock_acq(int handle, int duration, int arg3[], int arg4) {
    ALOGI("perf_lock_acq: handle: %d, duration: %d, arg3[0]: %d, arg4: %d",
            handle, duration, arg3[0], arg4);
    return handle ?: 233;
}

void perf_lock_cmd() {}

int perf_lock_rel(int handle) {
    ALOGI("perf_lock_rel: handle: %d", handle);
    return handle ?: 233;
}

void perf_lock_use_profile() {}
