#include "window_manager.h"

#include <glog/logging.h>

namespace win_man {

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

void run()
{
}

} // namespace win_man
