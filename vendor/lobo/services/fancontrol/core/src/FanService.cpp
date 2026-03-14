#include <lobo/platform/fancontrol/FanService.h>
#include <lobo/platform/logging/Logging.h>

namespace lobo {
namespace platform {
namespace fancontrol {

FanService::FanService() : mDutyCycle(0), mRunning(false) {}
FanService::~FanService() { stop(); }

bool FanService::start() {
    mRunning = true;
    Logging::log("FanService", "Service started");
    return true;
}

void FanService::stop() {
    mRunning = false;
    Logging::log("FanService", "Service stopped");
}

bool FanService::isRunning() const { return mRunning; }

Result FanService::setDutyCycle(int dutyCycle) {
    if (dutyCycle < 0 || dutyCycle > 100) return Result::INVALID_ARG;
    mDutyCycle = dutyCycle;
    return Result::OK;
}

int FanService::getDutyCycle() const { return mDutyCycle; }

} // namespace fancontrol
} // namespace platform
} // namespace lobo
