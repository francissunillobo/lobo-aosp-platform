package com.lobo.platform.systemapp.main.api

interface MainContract {
    fun onStart()
    fun onStop()
    fun getTitle(): String
}
