#pragma once

extern "C" {
#include <X11/Xlib.h>
}

#include <memory>

namespace win_man {

class WindowManager {
public:
    // establish connection to XServer and create WM instance
    static std::unique_ptr<WindowManager> create();

    // general error handler
    static int onXError(Display* d, XErrorEvent* e);
    // error hander to determine whether another window manager is
    // running. It is set as the error handler right before selecting substructure
    // redirection mask on the root window, so it is invoked if and only if
    // another window manager is running.
    static int onWMDetected(Display* d, XErrorEvent* e);

    // ~disconect from XServer
    ~WindowManager();

    // enter the main event loop
    void run();

private:
    WindowManager(Display* d);

private:
    static bool wm_detected_;
    
    Display* display_;
    const Window root_;
};

} // namespace win_man
