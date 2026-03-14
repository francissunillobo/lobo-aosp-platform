#include <lobo/platform/fan_math/FanMath.h>

namespace lobo {
namespace platform {

int FanMath::tempToDutyCycle(float tempCelsius, float minTemp, float maxTemp) {
    if (tempCelsius <= minTemp) return 0;
    if (tempCelsius >= maxTemp) return 100;
    return static_cast<int>((tempCelsius - minTemp) / (maxTemp - minTemp) * 100.0f);
}

int FanMath::clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

} // namespace platform
} // namespace lobo
