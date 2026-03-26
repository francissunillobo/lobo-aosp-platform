// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.calculator.impl

import android.os.IBinder
import android.os.IInterface
import android.os.Parcel
import android.os.RemoteException
import android.util.Log
import com.lobo.platform.calculator.api.ICalculatorClient
import com.lobo.platform.calculator.ICalculatorService
import java.io.FileDescriptor

/**
 * NDK/libbinder `FLAG_PRIVATE_VENDOR` (not exposed on the Java `IBinder` API
 * surface): tells the driver this user transaction targets a vendor-stability
 * remote from a system-stability Binder context (Treble / `BpBinder::transact`).
 */
private const val FLAG_PRIVATE_VENDOR: Int = 0x10000000

/**
 * Delegates to [base] but ORs [FLAG_PRIVATE_VENDOR] into every [transact] so a
 * system-stability Java Binder stack can call a vendor NDK AIBinder service.
 */
private class VendorPrivateBinder(private val base: IBinder) : IBinder {
    @Throws(RemoteException::class)
    override fun getInterfaceDescriptor(): String? = base.interfaceDescriptor

    override fun pingBinder(): Boolean = base.pingBinder()

    override fun isBinderAlive(): Boolean = base.isBinderAlive

    override fun queryLocalInterface(descriptor: String): IInterface? =
        base.queryLocalInterface(descriptor)

    @Throws(RemoteException::class)
    override fun dump(fd: FileDescriptor, args: Array<String>?) = base.dump(fd, args)

    @Throws(RemoteException::class)
    override fun dumpAsync(fd: FileDescriptor, args: Array<String>?) =
        base.dumpAsync(fd, args)

    @Throws(RemoteException::class)
    override fun transact(code: Int, data: Parcel, reply: Parcel?, flags: Int): Boolean =
        base.transact(code, data, reply, flags or FLAG_PRIVATE_VENDOR)

    @Throws(RemoteException::class)
    override fun linkToDeath(recipient: IBinder.DeathRecipient, flags: Int) =
        base.linkToDeath(recipient, flags)

    override fun unlinkToDeath(recipient: IBinder.DeathRecipient, flags: Int): Boolean =
        base.unlinkToDeath(recipient, flags)
}

/**
 * Kotlin implementation of ICalculatorClient.
 * Connects to the calculatord Binder service and delegates all calls to it.
 */
class CalculatorClientImpl : ICalculatorClient {

    private val tag = "CalculatorClientImpl"
    private val serviceName = "com.lobo.platform.calculator.ICalculatorService"

    private val service: ICalculatorService? by lazy {
        val binder = checkBinderService(serviceName)
        if (binder == null) {
            Log.e(tag, "Service '$serviceName' not found")
            null
        } else {
            Log.i(tag, "binder ok, using FLAG_PRIVATE_VENDOR wrapper for $serviceName")
            ICalculatorService.Stub.asInterface(VendorPrivateBinder(binder))
        }
    }

    /** ServiceManager is not on the vendor SDK surface; use reflection at runtime. */
    private fun checkBinderService(name: String): IBinder? = try {
        val sm = Class.forName("android.os.ServiceManager")
        val m = sm.getMethod("checkService", String::class.java)
        m.invoke(null, name) as? IBinder
    } catch (e: ReflectiveOperationException) {
        Log.e(tag, "checkService failed for '$name'", e)
        null
    }

    override fun add(a: Int, b: Int): Int =
        service?.add(a, b) ?: 0.also { Log.e(tag, "add: service unavailable") }

    override fun subtract(a: Int, b: Int): Int =
        service?.subtract(a, b) ?: 0.also { Log.e(tag, "subtract: service unavailable") }

    override fun multiply(a: Int, b: Int): Int =
        service?.multiply(a, b) ?: 0.also { Log.e(tag, "multiply: service unavailable") }

    override fun divide(a: Int, b: Int): Int =
        service?.divide(a, b) ?: 0.also { Log.e(tag, "divide: service unavailable") }
}
