package com.lobo.platform.systemapp.main.impl

import com.lobo.platform.systemapp.main.api.MainContract

class MainPresenter : MainContract {
    override fun onStart() {}
    override fun onStop() {}
    override fun getTitle(): String = "Lobo System App"
}
