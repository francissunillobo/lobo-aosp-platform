#pragma once

namespace lobo {
namespace platform {

class FanMath {
public:
    // Convert temperature (Celsius) to fan duty cycle (0-100)
    static int tempToDutyCycle(float tempCelsius, float minTemp, float maxTemp);

    // Clamp value between min and max
    static int clamp(int value, int min, int max);
};

} // namespace platform
} // namespace lobo
