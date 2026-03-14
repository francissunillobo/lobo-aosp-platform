#pragma once

#include <lobo/platform/types/CommonTypes.h>
#include <vector>

namespace lobo {
namespace platform {

class IThermalHal {
public:
    virtual ~IThermalHal() = default;

    virtual std::vector<ThermalZone> getThermalZones() = 0;
    virtual float readZoneTemperature(const std::string& zoneName) = 0;
};

} // namespace platform
} // namespace lobo
