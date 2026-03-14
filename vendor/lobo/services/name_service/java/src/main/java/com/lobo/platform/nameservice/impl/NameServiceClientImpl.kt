package com.lobo.platform.nameservice.impl

import android.util.Log
import com.lobo.platform.nameservice.api.INameServiceClient

class NameServiceClientImpl : INameServiceClient {
    private val tag = "NameServiceClient"

    override fun resolve(key: String): String? {
        Log.d(tag, "Resolving: $key")
        return null // Binder IPC call would go here
    }

    override fun register(key: String, value: String): Boolean {
        Log.d(tag, "Registering: $key -> $value")
        return true
    }

    override fun unregister(key: String): Boolean {
        Log.d(tag, "Unregistering: $key")
        return true
    }
}
