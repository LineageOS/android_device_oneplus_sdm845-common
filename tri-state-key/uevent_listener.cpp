/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uevent_listener.h"

#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

#include <memory>

#include <android-base/logging.h>
#include <cutils/uevent.h>

#define UEVENT_DEBUG false

namespace android {

static void ParseEvent(const char* msg, Uevent* uevent) {
    uevent->action.clear();
    uevent->name.clear();
    uevent->state.clear();

    while (*msg) {
        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            uevent->action = msg;
        } else if (!strncmp(msg, "NAME=", 5)) {
            msg += 5;
            uevent->state = msg;
        } else if (!strncmp(msg, "STATE=", 6)) {
            msg += 6;
            uevent->state = msg;
        }
        // advance to after the next \0
        while (*msg++)
            ;
    }

    if (UEVENT_DEBUG) {
        LOG(INFO) << "ACTION=" << uevent->action << " NAME=" << uevent->name
                  << " STATE=" << uevent->state;
    }
}

UeventListener::UeventListener() {
    // is 256K enough? udev uses 16MB!
    device_fd_.reset(uevent_open_socket(256 * 1024, true));
    if (device_fd_ == -1) {
        LOG(FATAL) << "Could not open uevent socket";
    }

    fcntl(device_fd_, F_SETFL, O_NONBLOCK);
}

bool UeventListener::ReadUevent(Uevent* uevent) const {
    char msg[UEVENT_MSG_LEN + 2];
    int n = uevent_kernel_multicast_recv(device_fd_, msg, UEVENT_MSG_LEN);
    if (n <= 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG(ERROR) << "Error reading from Uevent Fd";
        }
        return false;
    }
    if (n >= UEVENT_MSG_LEN) {
        LOG(ERROR) << "Uevent overflowed buffer, discarding";
        // Return true here even if we discard as we may have more uevents pending and we
        // want to keep processing them.
        return true;
    }

    msg[n] = '\0';
    msg[n + 1] = '\0';

    ParseEvent(msg, uevent);

    return true;
}

void UeventListener::Poll(const ListenerCallback& callback,
                          const std::optional<std::chrono::milliseconds> relative_timeout) const {
    using namespace std::chrono;

    pollfd ufd;
    ufd.events = POLLIN;
    ufd.fd = device_fd_;

    auto start_time = steady_clock::now();

    while (true) {
        ufd.revents = 0;

        int timeout_ms = -1;
        if (relative_timeout) {
            auto now = steady_clock::now();
            auto time_elapsed = duration_cast<milliseconds>(now - start_time);
            if (time_elapsed > *relative_timeout) return;

            auto remaining_timeout = *relative_timeout - time_elapsed;
            timeout_ms = remaining_timeout.count();
        }

        int nr = poll(&ufd, 1, timeout_ms);
        if (nr == 0) return;
        if (nr < 0) {
            PLOG(ERROR) << "poll() of uevent socket failed, continuing";
            continue;
        }
        if (ufd.revents & POLLIN) {
            // We're non-blocking, so if we receive a poll event keep processing until
            // we have exhausted all uevent messages.
            Uevent uevent;
            while (ReadUevent(&uevent)) {
                if (callback(uevent) == ListenerAction::kStop) return;
            }
        }
    }
}

}  // namespace android
