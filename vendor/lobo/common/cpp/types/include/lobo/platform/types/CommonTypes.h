#pragma once

#include <cstdint>
#include <string>

namespace lobo {
namespace platform {

// Fan state
enum class FanState : uint8_t {
    OFF     = 0,
    LOW     = 1,
    MEDIUM  = 2,
    HIGH    = 3,
    FULL    = 4,
};

// Thermal zone info
struct ThermalZone {
    std::string name;
    float       temperatureCelsius;
    FanState    recommendedState;
};

// Generic result type
enum class Result : int32_t {
    OK           =  0,
    ERROR        = -1,
    UNSUPPORTED  = -2,
    INVALID_ARG  = -3,
};

} // namespace platform
} // namespace lobo
