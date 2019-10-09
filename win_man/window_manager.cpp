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
            // TODO implement
            // OnCreateNotify(e.xcreatewindow);
            break;
        case ConfigureRequest:
            onConfigureRequest(e.xconfigurerequest);
            break;
        case MapRequest:
            onMapRequest(e.xmaprequest);
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

void WindowManager::onConfigureRequest(const XConfigureRequestEvent& e)
{
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;
    XConfigureWindow(display_, e.window, e.value_mask, &changes);
    LOG(INFO) << "Resize " << e.window << " to " << e.width << " " << e.height;
}

void WindowManager::onMapRequest(const XMapRequestEvent& e)
{
    // Frame or re-frame window.
    frame(e.window);
    // Actually map window.
    XMapWindow(display_, e.window);
}

void WindowManager::frame(Window w)
{
    // Visual properties of the frame to create.
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;

    XWindowAttributes window_attrs;
    CHECK(XGetWindowAttributes(display_, w, &window_attrs));

    const Window frame = XCreateSimpleWindow(display_, root_,
        window_attrs.x, window_attrs.y,
        window_attrs.width, window_attrs.height,
        BORDER_WIDTH, BORDER_COLOR,
        BG_COLOR);

    // Select events on frame
    XSelectInput(display_, frame,
        SubstructureRedirectMask | SubstructureNotifyMask);

    // Add client to save set, so that it will be restored and kept alive if we crash.
    XAddToSaveSet(display_, w);

    // Reparent client window.
    XReparentWindow(display_, w, frame,
        0, 0); // Offset of client window within frame.

    // Map frame.
    XMapWindow(display_, frame);
    // Save frame handle.
    clients_[w] = frame;

    // Grab universal window management actions on client window.
    // Move windows with alt + left button.
    XGrabButton(display_,
        Button1, Mod1Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync, GrabModeAsync,
        None, None);
    // Resize windows with alt + right button.
    XGrabButton(
        display_,
        Button3,
        Mod1Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);
    // Kill windows with alt + f4.
    // XGrabKey(
    //     display_,
    //     XKeysymToKeycode(display_, XK_F4),
    //     Mod1Mask,
    //     w,
    //     false,
    //     GrabModeAsync,
    //     GrabModeAsync);
    // Switch windows with alt + tab.
    // XGrabKey(
    //     display_,
    //     XKeysymToKeycode(display_, XK_Tab),
    //     Mod1Mask,
    //     w,
    //     false,
    //     GrabModeAsync,
    //     GrabModeAsync);

    LOG(INFO) << "Framed window " << w << " [" << frame << "]";
}

} // namespace win_man
