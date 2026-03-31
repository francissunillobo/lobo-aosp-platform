// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#define LOG_TAG "temperature_monitord"

#include <lobo/platform/temperaturemonitor/TemperatureMonitorServiceImpl.h>

#include <aidl/com/lobo/platform/temperaturemonitor/ITemperatureMonitorCallback.h>
#include <android/binder_ibinder.h>
#include <android/binder_parcel_utils.h>
#include <android/binder_status.h>

#include <log/log.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <vector>

// Used by TemperatureMonitorServiceImpl member definitions (not visible from anonymous namespace).
using aidl::com::lobo::platform::temperaturemonitor::ITemperatureMonitorCallback;
using aidl::com::lobo::platform::temperaturemonitor::ThermalZoneReading;

namespace {

namespace hal = lobo::platform::hal;

int64_t boottimeNanos() {
    struct timespec ts {};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return static_cast<int64_t>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

std::vector<ThermalZoneReading> toAidl(const std::vector<hal::ThermalZoneSample>& in) {
    std::vector<ThermalZoneReading> out;
    out.reserve(in.size());
    const int64_t ts = boottimeNanos();
    for (const auto& s : in) {
        ThermalZoneReading r;
        r.zoneName = s.zone_name;
        r.tempMilliCelsius = s.temp_millicelsius;
        r.timestampNanos = ts;
        out.push_back(std::move(r));
    }
    return out;
}

bool shouldNotify(const std::vector<hal::ThermalZoneSample>& oldS,
                  const std::vector<hal::ThermalZoneSample>& newS,
                  int64_t thresholdMilli) {
    if (oldS.empty() && newS.empty()) {
        return false;
    }
    if (oldS.empty()) {
        return true;
    }
    if (oldS.size() != newS.size()) {
        return true;
    }
    for (size_t i = 0; i < oldS.size(); ++i) {
        if (oldS[i].zone_name != newS[i].zone_name) {
            return true;
        }
        const int64_t d = std::llabs(newS[i].temp_millicelsius - oldS[i].temp_millicelsius);
        if (d >= thresholdMilli) {
            return true;
        }
    }
    return false;
}

/**
 * Vendor daemon -> Java app callback: the generated Bp stub ORs FLAG_PRIVATE_VENDOR when
 * BINDER_STABILITY_SUPPORT is defined, which makes BpBinder reject a system-stability remote
 * callback from a vendor process. Mirror the generated parcel layout but use FLAG_ONEWAY only.
 */
binder_status_t transactOnReadingsChangedNoPrivateVendor(
        AIBinder* remote, const std::vector<ThermalZoneReading>& readings) {
    ndk::ScopedAParcel in;
    ndk::ScopedAParcel out;
    binder_status_t ret = AIBinder_prepareTransaction(remote, in.getR());
    if (ret != STATUS_OK) {
        return ret;
    }
    ret = ::ndk::AParcel_writeData(in.get(), readings);
    if (ret != STATUS_OK) {
        return ret;
    }
    return AIBinder_transact(
            remote,
            aidl::com::lobo::platform::temperaturemonitor::ITemperatureMonitorCallback::
                    TRANSACTION_onReadingsChanged,
            in.getR(),
            out.getR(),
            FLAG_ONEWAY);
}

} // namespace

namespace lobo {
namespace platform {
namespace temperaturemonitor {

TemperatureMonitorServiceImpl::TemperatureMonitorServiceImpl(
        std::unique_ptr<hal::ITemperatureMonitorHal> hal)
    : hal_(std::move(hal)),
      poll_interval_ms_(2000),
      // Default notify-on-change threshold.
      // Keep it small so the UI gets frequent updates even with minor temperature drift.
      notify_threshold_millicelsius_(10) {
    poll_thread_ = std::thread([this]() { pollLoop(); });
}

TemperatureMonitorServiceImpl::~TemperatureMonitorServiceImpl() {
    shutdown_ = true;
    if (poll_thread_.joinable()) {
        poll_thread_.join();
    }
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::getCurrentReadings(
        std::vector<ThermalZoneReading>* _aidl_return) {
    std::lock_guard<std::mutex> lock(mutex_);
    *_aidl_return = toAidl(last_samples_);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::registerCallback(
        const std::shared_ptr<ITemperatureMonitorCallback>& in_callback) {
    if (!in_callback || in_callback->asBinder().get() == nullptr) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    std::lock_guard<std::mutex> lock(mutex_);
    AIBinder* nb = in_callback->asBinder().get();
    for (const auto& c : callbacks_) {
        if (c->asBinder().get() == nb) {
            ALOGI("registerCallback: duplicate callback binder=%p callbacks=%zu", nb, callbacks_.size());
            return ndk::ScopedAStatus::ok();
        }
    }
    callbacks_.push_back(in_callback);
    ALOGI("registerCallback: added binder=%p callbacks=%zu", nb, callbacks_.size());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::unregisterCallback(
        const std::shared_ptr<ITemperatureMonitorCallback>& in_callback) {
    if (!in_callback || in_callback->asBinder().get() == nullptr) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    std::lock_guard<std::mutex> lock(mutex_);
    AIBinder* nb = in_callback->asBinder().get();
    const size_t before = callbacks_.size();
    callbacks_.erase(std::remove_if(callbacks_.begin(), callbacks_.end(),
                                   [nb](const std::shared_ptr<ITemperatureMonitorCallback>& c) {
                                       return c->asBinder().get() == nb;
                                   }),
                     callbacks_.end());
    ALOGI("unregisterCallback: binder=%p callbacks=%zu->%zu", nb, before, callbacks_.size());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::getPollIntervalMillis(int64_t* _aidl_return) {
    *_aidl_return = poll_interval_ms_.load();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::setPollIntervalMillis(int64_t intervalMs) {
    int64_t v = intervalMs;
    if (v < 200) {
        v = 200;
    }
    if (v > 600000) {
        v = 600000;
    }
    poll_interval_ms_.store(v);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::getNotifyThresholdMilliCelsius(
        int64_t* _aidl_return) {
    *_aidl_return = notify_threshold_millicelsius_.load();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TemperatureMonitorServiceImpl::setNotifyThresholdMilliCelsius(
        int64_t thresholdMilli) {
    int64_t v = thresholdMilli;
    if (v < 0) {
        v = 0;
    }
    notify_threshold_millicelsius_.store(v);
    return ndk::ScopedAStatus::ok();
}

void TemperatureMonitorServiceImpl::pollLoop() {
    while (!shutdown_) {
        std::vector<hal::ThermalZoneSample> fresh;
        if (!hal_->readSamples(fresh)) {
            ALOGE("readSamples failed");
        } else {
            bool notify = false;
            std::vector<hal::ThermalZoneSample> snapshot_for_notify;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                notify = shouldNotify(last_samples_, fresh,
                                      notify_threshold_millicelsius_.load());
                last_samples_ = fresh;
                snapshot_for_notify = last_samples_;
            }

            if (notify) {
                const std::vector<ThermalZoneReading> aidl = toAidl(snapshot_for_notify);
                std::vector<std::shared_ptr<ITemperatureMonitorCallback>> copy;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    copy = callbacks_;
                }
                ALOGI("notify: zones=%zu callbacks=%zu threshold=%lld",
                      aidl.size(), copy.size(),
                      static_cast<long long>(notify_threshold_millicelsius_.load()));
                std::vector<AIBinder*> failed;
                for (const auto& cb : copy) {
                    AIBinder* binder = cb->asBinder().get();
                    const binder_status_t tr = transactOnReadingsChangedNoPrivateVendor(binder, aidl);
                    if (tr != STATUS_OK) {
                        ALOGW("onReadingsChanged failed: binder=%p status=%d",
                              binder, static_cast<int>(tr));
                        failed.push_back(binder);
                    }
                }

                if (!failed.empty()) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    callbacks_.erase(std::remove_if(callbacks_.begin(), callbacks_.end(),
                                                    [&failed](const std::shared_ptr<ITemperatureMonitorCallback>& c) {
                                                        const AIBinder* b = c->asBinder().get();
                                                        return std::find(failed.begin(), failed.end(), b) != failed.end();
                                                    }),
                                     callbacks_.end());
                    if (!callbacks_.empty()) {
                        ALOGI("removed %zu dead callback(s), %zu callback(s) remain",
                              failed.size(), callbacks_.size());
                    } else {
                        ALOGI("removed %zu dead callback(s), no callbacks remain", failed.size());
                    }
                }
            }
        }

        int64_t interval = poll_interval_ms_.load();
        if (interval < 200) {
            interval = 200;
        }
        int64_t remaining = interval;
        while (remaining > 0 && !shutdown_) {
            const int64_t chunk = std::min(remaining, static_cast<int64_t>(200));
            std::this_thread::sleep_for(std::chrono::milliseconds(chunk));
            remaining -= chunk;
        }
    }
}

} // namespace temperaturemonitor
} // namespace platform
} // namespace lobo
