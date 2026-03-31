// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.client.thermalcontrol.impl

import android.os.IBinder
import android.util.Log
import com.lobo.platform.client.thermalcontrol.api.ThermalControlManager
import com.lobo.platform.client.thermalcontrol.api.ThermalReadingsListener
import com.lobo.platform.temperaturemonitor.ITemperatureMonitorCallback
import com.lobo.platform.temperaturemonitor.ITemperatureMonitorService
import com.lobo.platform.temperaturemonitor.ThermalZoneReading
import java.util.concurrent.ConcurrentHashMap

private const val TAG = "ThermalCtrlMgr"
private const val SERVICE_NAME =
    "com.lobo.platform.temperaturemonitor.ITemperatureMonitorService"

/**
 * Default [ThermalControlManager]: Binder to `temperature_monitord` only. Fan integration
 * will extend this class or compose additional services later.
 */
class ThermalControlManagerImpl : ThermalControlManager {

    private var service: ITemperatureMonitorService? = null

    private val listenerToCallback =
        ConcurrentHashMap<ThermalReadingsListener, ITemperatureMonitorCallback>()

    override val isConnected: Boolean
        get() = service != null

    override fun connect(): Boolean {
        if (service != null) {
            return true
        }
        val binder = checkBinderService(SERVICE_NAME) ?: run {
            Log.e(TAG, "Service '$SERVICE_NAME' not found")
            return false
        }
        Log.i(TAG, "binder ok, using FLAG_PRIVATE_VENDOR wrapper for $SERVICE_NAME")
        service = ITemperatureMonitorService.Stub.asInterface(VendorPrivateBinder(binder))
        return true
    }

    override fun disconnect() {
        val s = service ?: return
        for ((_, callback) in listenerToCallback.entries.toList()) {
            try {
                s.unregisterCallback(callback)
            } catch (e: Exception) {
                Log.w(TAG, "unregisterCallback failed", e)
            }
        }
        listenerToCallback.clear()
        service = null
    }

    override fun getCurrentReadings(): Array<ThermalZoneReading>? {
        val s = service ?: return null
        return try {
            s.getCurrentReadings()
        } catch (e: Exception) {
            Log.e(TAG, "getCurrentReadings failed", e)
            null
        }
    }

    override fun registerListener(listener: ThermalReadingsListener) {
        val s = service ?: run {
            Log.e(TAG, "registerListener: not connected")
            return
        }
        if (listenerToCallback.containsKey(listener)) {
            return
        }
        val localStub =
            object : ITemperatureMonitorCallback.Stub() {
                override fun onReadingsChanged(readings: Array<ThermalZoneReading>?) {
                    val arr = readings ?: emptyArray()
                    listener.onReadingsChanged(arr)
                }
            }
        // Register the app-local callback stub directly.
        // Register the app-local callback stub directly over the same service handle.
        listenerToCallback[listener] = localStub
        try {
            s.registerCallback(localStub)
        } catch (e: Exception) {
            listenerToCallback.remove(listener)
            Log.e(TAG, "registerCallback failed", e)
        }
    }

    override fun unregisterListener(listener: ThermalReadingsListener) {
        val s = service ?: return
        val callback = listenerToCallback.remove(listener) ?: return
        try {
            s.unregisterCallback(callback)
        } catch (e: Exception) {
            Log.w(TAG, "unregisterCallback failed", e)
        }
    }

    override fun getPollIntervalMillis(): Long {
        val s = service ?: return 0L
        return try {
            s.getPollIntervalMillis()
        } catch (e: Exception) {
            Log.e(TAG, "getPollIntervalMillis failed", e)
            0L
        }
    }

    override fun setPollIntervalMillis(intervalMs: Long) {
        val s = service ?: return
        try {
            s.setPollIntervalMillis(intervalMs)
        } catch (e: Exception) {
            Log.e(TAG, "setPollIntervalMillis failed", e)
        }
    }

    override fun getNotifyThresholdMilliCelsius(): Long {
        val s = service ?: return 0L
        return try {
            s.getNotifyThresholdMilliCelsius()
        } catch (e: Exception) {
            Log.e(TAG, "getNotifyThresholdMilliCelsius failed", e)
            0L
        }
    }

    override fun setNotifyThresholdMilliCelsius(thresholdMilli: Long) {
        val s = service ?: return
        try {
            s.setNotifyThresholdMilliCelsius(thresholdMilli)
        } catch (e: Exception) {
            Log.e(TAG, "setNotifyThresholdMilliCelsius failed", e)
        }
    }

    private fun checkBinderService(name: String): IBinder? =
        try {
            val sm = Class.forName("android.os.ServiceManager")
            val m = sm.getMethod("checkService", String::class.java)
            m.invoke(null, name) as? IBinder
        } catch (e: ReflectiveOperationException) {
            Log.e(TAG, "checkService failed for '$name'", e)
            null
        }
}
