// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.temperaturemonitor;

import com.lobo.platform.temperaturemonitor.ITemperatureMonitorCallback;
import com.lobo.platform.temperaturemonitor.ThermalZoneReading;

/**
 * Vendor thermal monitor service.
 * Binder name: com.lobo.platform.temperaturemonitor.ITemperatureMonitorService
 * @hide
 */
interface ITemperatureMonitorService {
    ThermalZoneReading[] getCurrentReadings();
    void registerCallback(in ITemperatureMonitorCallback callback);
    void unregisterCallback(in ITemperatureMonitorCallback callback);
    long getPollIntervalMillis();
    void setPollIntervalMillis(long intervalMs);
    long getNotifyThresholdMilliCelsius();
    void setNotifyThresholdMilliCelsius(long thresholdMilli);
}
