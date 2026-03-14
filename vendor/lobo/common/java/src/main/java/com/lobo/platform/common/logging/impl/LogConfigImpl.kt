package com.lobo.platform.common.logging.impl

import android.util.Log
import com.lobo.platform.common.logging.api.LogConfig

class LogConfigImpl(
    override val minLogLevel: Int = Log.INFO,
    override val tag: String = "LoboLogging",
) : LogConfig()
