package com.lobo.platform.nameservice.api

interface INameServiceClient {
    fun resolve(key: String): String?
    fun register(key: String, value: String): Boolean
    fun unregister(key: String): Boolean
}
