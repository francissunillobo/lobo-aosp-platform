// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.client.thermalcontrol.impl

import android.os.IBinder
import android.os.IInterface
import android.os.Parcel
import android.os.RemoteException
import java.io.FileDescriptor

/**
 * NDK `FLAG_PRIVATE_VENDOR` (0x10000000): required for system-stability Java Binder callers
 * talking to vendor NDK AIDL services. Same idea as CalculatorClientApp.
 */
private const val FLAG_PRIVATE_VENDOR: Int = 0x10000000

/** Delegates to [base] but ORs [FLAG_PRIVATE_VENDOR] into every [transact]. */
internal class VendorPrivateBinder(private val base: IBinder) : IBinder {
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
