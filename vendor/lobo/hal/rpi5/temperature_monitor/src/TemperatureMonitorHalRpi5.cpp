// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#include <lobo/platform/hal/rpi5/TemperatureMonitorHalRpi5.h>

#include <dirent.h>

#include <algorithm>
#include <fstream>
#include <string>

namespace lobo {
namespace platform {
namespace hal {
namespace rpi5 {

namespace {

constexpr const char* kThermalClass = "/sys/class/thermal";

} // namespace

bool TemperatureMonitorHalRpi5::readSamples(std::vector<ThermalZoneSample>& out) {
    out.clear();
    DIR* dir = opendir(kThermalClass);
    if (!dir) {
        return false;
    }
    dirent* e;
    while ((e = readdir(dir)) != nullptr) {
        std::string name = e->d_name;
        if (name.rfind("thermal_zone", 0) != 0) {
            continue;
        }
        const std::string base = std::string(kThermalClass) + "/" + name;
        std::string typeStr;
        {
            std::ifstream fType(base + "/type");
            if (fType) {
                std::getline(fType, typeStr);
            }
        }
        if (typeStr.empty()) {
            typeStr = name;
        }
        int64_t tempMilli = 0;
        {
            std::ifstream fTemp(base + "/temp");
            if (fTemp) {
                fTemp >> tempMilli;
            }
        }
        ThermalZoneSample z;
        z.zone_name = std::move(typeStr);
        z.temp_millicelsius = tempMilli;
        out.push_back(std::move(z));
    }
    closedir(dir);
    std::sort(out.begin(), out.end(), [](const ThermalZoneSample& a, const ThermalZoneSample& b) {
        return a.zone_name < b.zone_name;
    });
    return true;
}

} // namespace rpi5
} // namespace hal
} // namespace platform
} // namespace lobo
