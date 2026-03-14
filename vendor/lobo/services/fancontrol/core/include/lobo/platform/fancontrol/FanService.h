#pragma once

#include <lobo/platform/types/CommonTypes.h>
#include <string>

namespace lobo {
namespace platform {
namespace fancontrol {

class FanService {
public:
    FanService();
    ~FanService();

    bool start();
    void stop();
    bool isRunning() const;
    Result setDutyCycle(int dutyCycle);
    int getDutyCycle() const;

private:
    int  mDutyCycle;
    bool mRunning;
};

} // namespace fancontrol
} // namespace platform
} // namespace lobo
