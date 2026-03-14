package com.lobo.platform.systemapp.settings.impl

import android.preference.PreferenceFragment
import android.util.Log

class SettingsFragment : PreferenceFragment() {
    private val tag = "SettingsFragment"

    override fun onStart() {
        super.onStart()
        Log.i(tag, "SettingsFragment started")
    }
}
