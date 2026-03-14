package com.lobo.platform.systemapp.settings.impl

import android.util.Log
import com.lobo.platform.systemapp.settings.api.SettingsContract

class SettingsPresenter : SettingsContract {
    override fun loadSettings() { Log.d("SettingsPresenter", "loadSettings") }
    override fun saveSettings() { Log.d("SettingsPresenter", "saveSettings") }
}
