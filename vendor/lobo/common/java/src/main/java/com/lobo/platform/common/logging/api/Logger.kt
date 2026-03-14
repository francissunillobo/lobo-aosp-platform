package com.lobo.platform.common.logging.api

interface Logger {
    fun log(tag: String, message: String)
    fun logError(tag: String, message: String, throwable: Throwable? = null)
}
