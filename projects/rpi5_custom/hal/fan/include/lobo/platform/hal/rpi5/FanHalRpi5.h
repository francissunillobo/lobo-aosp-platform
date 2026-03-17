#pragma once

#include <lobo/platform/hal/IFanHal.h>

namespace lobo {
namespace platform {
namespace rpi5 {

class FanHalRpi5 : public IFanHal {
public:
    explicit FanHalRpi5(int gpioPin = 18);
    ~FanHalRpi5() override;

    Result    setFanState(FanState state) override;
    FanState  getFanState() const override;
    float     readTemperatureCelsius() override;

private:
    int      mGpioPin;
    FanState mCurrentState;
};

} // namespace rpi5
} // namespace platform
} // namespace lobo
