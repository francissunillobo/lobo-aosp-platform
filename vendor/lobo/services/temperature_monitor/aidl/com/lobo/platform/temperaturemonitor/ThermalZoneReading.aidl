// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.temperaturemonitor;

/** One zone reading; temperatures are millidegrees Celsius (Linux sysfs). */
parcelable ThermalZoneReading {
    String zoneName;
    long tempMilliCelsius;
    long timestampNanos;
}
