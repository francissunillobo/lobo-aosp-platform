#pragma once

#include <lobo/platform/types/CommonTypes.h>

namespace lobo {
namespace platform {

// Abstract HAL interface — board-independent.
// Board-specific implementations are in hal/<board>/ and must NOT be
// referenced directly by service code. Services use only this header.
class IFanHal {
public:
    virtual ~IFanHal() = default;

    virtual Result setFanState(FanState state) = 0;
    virtual FanState getFanState() const = 0;
    virtual float readTemperatureCelsius() = 0;
};

} // namespace platform
} // namespace lobo
