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

    // ~disconect from XServer
    ~WindowManager();

    // enter the main event loop
    void run();

private:
    WindowManager(Display* d);

private:
    Display* display_;
    const Window root_;
};

} // namespace win_man
