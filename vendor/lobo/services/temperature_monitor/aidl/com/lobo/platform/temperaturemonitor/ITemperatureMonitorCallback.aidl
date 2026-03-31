// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.temperaturemonitor;

import com.lobo.platform.temperaturemonitor.ThermalZoneReading;

interface ITemperatureMonitorCallback {
    oneway void onReadingsChanged(in ThermalZoneReading[] readings);
}
