package com.lobo.platform.common.logging.impl

import android.util.Log
import com.lobo.platform.common.logging.api.Logger

class LoggerImpl : Logger {
    override fun log(tag: String, message: String) {
        Log.i(tag, message)
    }

    override fun logError(tag: String, message: String, throwable: Throwable?) {
        Log.e(tag, message, throwable)
    }
}
