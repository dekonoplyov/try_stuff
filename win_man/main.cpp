#include "window_manager.h"

#include <glog/logging.h>

int main(int argc, char const* argv[])
{
    google::InitGoogleLogging(argv[0]);
    auto window_manager = win_man::WindowManager::create();
    if (!window_manager) {
        LOG(ERROR) << "Failed to create window manager";
        return EXIT_FAILURE;
    }

    window_manager->run();

    return 0;
}
