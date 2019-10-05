#include "window_manager.h"

#include <glog/logging.h>

namespace win_man {

bool WindowManager::wm_detected_ = false;

std::unique_ptr<WindowManager> WindowManager::create()
{
    auto display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        LOG(ERROR) << "Failed to open XDisplay: " << XDisplayName(nullptr);
        return nullptr;
    }
    return std::make_unique<WindowManager>(display);
}

WindowManager::WindowManager(Display* d)
    : display_{CHECK_NOTNULL(d)}
    , root_{DefaultRootWindow(display_)}
{
}

WindowManager::~WindowManager()
{
    XCloseDisplay(display_);
}

void WindowManager::run()
{
    // Select events on root window. Use a special error handler so we can
    // exit gracefully if another window manager is already running.
    wm_detected_ = false;
    XSetErrorHandler(&WindowManager::onWMDetected);
    XSelectInput(display_, root_,
        SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display_, false);
    if (wm_detected_) {
        LOG(ERROR) << "Detected another window manager on display "
                   << XDisplayString(display_);
        return;
    }

    // set general error handler
    XSetErrorHandler(&WindowManager::onXError);

    while (true) {
        XEvent e;
        XNextEvent(display_, &e);

        // LOG(INFO) << "Received event: " << ToString(e);

        switch (e.type) {
        case CreateNotify:
            /* code */
            break;
        case DestroyNotify:
            /* code */
            break;
        case ReparentNotify:
            /* code */
            break;

        default:
            LOG(WARNING) << "Ignored event";
            break;
        }
        /* code */
    }
}

int WindowManager::onWMDetected(Display* d, XErrorEvent* e)
{
    (void)d;
    // In the case of an already running window manager, the error code from
    // XSelectInput is BadAccess. We don't expect this handler to receive any
    // other errors.
    CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
    // Set flag.
    WindowManager::wm_detected_ = true;
    // The return value is ignored.
    return 0;
}

int WindowManager::onXError(Display* d, XErrorEvent* e)
{
    (void)e;
    (void)d;
    // Print error
    return 0;
}

} // namespace win_man
