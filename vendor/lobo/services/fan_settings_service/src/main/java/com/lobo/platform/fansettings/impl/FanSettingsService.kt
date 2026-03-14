package com.lobo.platform.fansettings.impl

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log

class FanSettingsService : Service() {
    private val tag = "FanSettingsService"

    override fun onCreate() {
        super.onCreate()
        Log.i(tag, "FanSettingsService created")
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.i(tag, "FanSettingsService started")
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onDestroy() {
        Log.i(tag, "FanSettingsService destroyed")
        super.onDestroy()
    }
}
