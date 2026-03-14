#include <lobo/platform/fancontrol/FanService.h>
#include <lobo/platform/logging/Logging.h>
#include <android-base/logging.h>

int main(int /*argc*/, char* /*argv*/[]) {
    android::base::InitLogging(nullptr);
    lobo::platform::Logging::log("FanControlService", "Starting FanControlService");

    lobo::platform::fancontrol::FanService service;
    if (!service.start()) {
        lobo::platform::Logging::logError("FanControlService", "Failed to start");
        return 1;
    }

    // Block until stopped
    while (service.isRunning()) {
        // main loop handled inside service
    }
    return 0;
}
