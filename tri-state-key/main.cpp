#define LOG_TAG "tri-state-key-daemon"

#include <android-base/logging.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>

#include "uevent_listener.h"

#define KEY_NONE 601
#define KEY_SILENCE 602
#define KEY_VIBRATION 603

using namespace android;

int main() {
    int err;
    int uinputFd;
    struct uinput_user_dev uidev;

    LOG(INFO) << "Started";

    uinputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinputFd < 0) {
        LOG(ERROR) << "Unable to open uinput node";
        return 1;
    }

    err = ioctl(uinputFd, UI_SET_EVBIT, EV_KEY) | ioctl(uinputFd, UI_SET_KEYBIT, KEY_NONE) |
          ioctl(uinputFd, UI_SET_KEYBIT, KEY_SILENCE) |
          ioctl(uinputFd, UI_SET_KEYBIT, KEY_VIBRATION);
    if (err != 0) {
        LOG(ERROR) << "Unable to enable KEY events";
        close(uinputFd);
        return 1;
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-tri-state-key");
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor = 0;
    uidev.id.product = 0;
    uidev.id.version = 0;

    err = write(uinputFd, &uidev, sizeof(uidev));
    if (err < 0) {
        LOG(ERROR) << "Write user device to uinput node failed";
        close(uinputFd);
        return 1;
    }

    err = ioctl(uinputFd, UI_DEV_CREATE);
    if (err < 0) {
        LOG(ERROR) << "Unable to create uinput device";
        close(uinputFd);
        return 1;
    }

    LOG(INFO) << "Successfully registered uinput-tri-state-key for KEY events";

    UeventListener uevent_listener;
    uevent_listener.Poll([&uinputFd](const Uevent& uevent) {
        int err;
        struct input_event event;

        if (uevent.action != "change" && uevent.name != "soc:tri_state_key") {
            return ListenerAction::kContinue;
        }

        bool none = uevent.state.find("USB=0") != std::string::npos;
        bool vibration = uevent.state.find("USB_HOST=0") != std::string::npos;
        bool silent = uevent.state.find("(null)=0") != std::string::npos;

        int keyCode;
        if (none && !vibration && !silent) {
            keyCode = KEY_NONE;
        } else if (!none && vibration && !silent) {
            keyCode = KEY_VIBRATION;
        } else if (!none && !vibration && silent) {
            keyCode = KEY_SILENCE;
        } else {
            // Ignore intermediate states
            return ListenerAction::kContinue;
        }

        // Report the key
        event.type = EV_KEY;
        event.code = keyCode;
        event.value = 1;
        err = write(uinputFd, &event, sizeof(event));
        if (err < 0) {
            LOG(ERROR) << "Write EV_KEY to uinput node failed";
            return ListenerAction::kContinue;
        }

        // Force a flush with an EV_SYN
        event.type = EV_SYN;
        event.code = SYN_REPORT;
        event.value = 0;
        err = write(uinputFd, &event, sizeof(event));
        if (err < 0) {
            LOG(ERROR) << "Write EV_SYN to uinput node failed";
            return ListenerAction::kContinue;
        }

        // Report the key
        event.type = EV_KEY;
        event.code = keyCode;
        event.value = 0;
        err = write(uinputFd, &event, sizeof(event));
        if (err < 0) {
            LOG(ERROR) << "Write EV_KEY to uinput node failed";
            return ListenerAction::kContinue;
        }

        // Force a flush with an EV_SYN
        event.type = EV_SYN;
        event.code = SYN_REPORT;
        event.value = 0;
        err = write(uinputFd, &event, sizeof(event));
        if (err < 0) {
            LOG(ERROR) << "Write EV_SYN to uinput node failed";
            return ListenerAction::kContinue;
        }

        return ListenerAction::kContinue;
    });

    // Clean up
    close(uinputFd);

    // The loop can only be exited via failure or signal
    return 1;
}
