// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <aidl/com/lobo/platform/temperaturemonitor/BnTemperatureMonitorService.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <lobo/platform/hal/ITemperatureMonitorHal.h>

namespace lobo {
namespace platform {
namespace temperaturemonitor {

/**
 * NDK Binder service: polls ITemperatureMonitorHal, debounces, notifies AIDL callbacks.
 */
class TemperatureMonitorServiceImpl
        : public aidl::com::lobo::platform::temperaturemonitor::BnTemperatureMonitorService {
public:
    explicit TemperatureMonitorServiceImpl(
            std::unique_ptr<lobo::platform::hal::ITemperatureMonitorHal> hal);
    ~TemperatureMonitorServiceImpl() override;

    ndk::ScopedAStatus getCurrentReadings(
            std::vector<aidl::com::lobo::platform::temperaturemonitor::ThermalZoneReading>* _aidl_return) override;

    ndk::ScopedAStatus registerCallback(
            const std::shared_ptr<
                    aidl::com::lobo::platform::temperaturemonitor::ITemperatureMonitorCallback>&
                    in_callback) override;

    ndk::ScopedAStatus unregisterCallback(
            const std::shared_ptr<
                    aidl::com::lobo::platform::temperaturemonitor::ITemperatureMonitorCallback>&
                    in_callback) override;

    ndk::ScopedAStatus getPollIntervalMillis(int64_t* _aidl_return) override;
    ndk::ScopedAStatus setPollIntervalMillis(int64_t intervalMs) override;
    ndk::ScopedAStatus getNotifyThresholdMilliCelsius(int64_t* _aidl_return) override;
    ndk::ScopedAStatus setNotifyThresholdMilliCelsius(int64_t thresholdMilli) override;

private:
    void pollLoop();

    std::unique_ptr<lobo::platform::hal::ITemperatureMonitorHal> hal_;

    std::mutex mutex_;
    std::vector<lobo::platform::hal::ThermalZoneSample> last_samples_;
    std::vector<std::shared_ptr<
            aidl::com::lobo::platform::temperaturemonitor::ITemperatureMonitorCallback>>
            callbacks_;

    std::atomic<int64_t> poll_interval_ms_;
    std::atomic<int64_t> notify_threshold_millicelsius_;

    std::thread poll_thread_;
    std::atomic<bool> shutdown_{false};
};

} // namespace temperaturemonitor
} // namespace platform
} // namespace lobo
