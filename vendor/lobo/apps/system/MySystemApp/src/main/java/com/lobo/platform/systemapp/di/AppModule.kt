package com.lobo.platform.systemapp.di

import com.lobo.platform.systemapp.main.api.MainContract
import com.lobo.platform.systemapp.main.impl.MainPresenter
import com.lobo.platform.systemapp.settings.api.SettingsContract
import com.lobo.platform.systemapp.settings.impl.SettingsPresenter

object AppModule {
    fun provideMainPresenter(): MainContract = MainPresenter()
    fun provideSettingsPresenter(): SettingsContract = SettingsPresenter()
}
