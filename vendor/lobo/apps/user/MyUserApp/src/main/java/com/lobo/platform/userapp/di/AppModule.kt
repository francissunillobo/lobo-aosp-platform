package com.lobo.platform.userapp.di

import com.lobo.platform.userapp.main.api.MainContract
import com.lobo.platform.userapp.main.impl.MainPresenter

object AppModule {
    fun provideMainPresenter(): MainContract = MainPresenter()
}
