package com.lobo.platform.common.logging.api

abstract class LogConfig {
    abstract val minLogLevel: Int
    abstract val tag: String
}
