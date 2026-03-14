package com.lobo.platform.systemapp.main.impl

import android.app.Activity
import android.os.Bundle
import android.util.Log

class MainActivity : Activity() {
    private val tag = "MySystemApp"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(tag, "MainActivity created")
    }

    override fun onStart() {
        super.onStart()
        Log.i(tag, "MainActivity started")
    }

    override fun onStop() {
        super.onStop()
        Log.i(tag, "MainActivity stopped")
    }
}
