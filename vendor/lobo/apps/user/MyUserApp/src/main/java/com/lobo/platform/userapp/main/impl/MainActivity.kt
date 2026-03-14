package com.lobo.platform.userapp.main.impl

import android.app.Activity
import android.os.Bundle
import android.util.Log

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("MyUserApp", "MainActivity created")
    }
}
