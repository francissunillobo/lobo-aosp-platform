// Copyright (C) 2026 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.thermalmonitor.client.ui

import android.os.Bundle
import android.view.View
import android.widget.ScrollView
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.updatePadding
import com.lobo.platform.thermalmonitor.client.R
import com.lobo.platform.thermalmonitor.client.presentation.ThermalMonitorViewModel
import com.lobo.platform.thermalmonitor.client.presentation.ThermalUiState
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {

    private val viewModel: ThermalMonitorViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_thermal_monitor)

        // Ensure content isn't covered by the car/system top banner by applying
        // status bar / cutout insets to the ScrollView.
        val scrollView = findViewById<ScrollView>(R.id.scroll_root)
        val basePaddingTop = scrollView.paddingTop
        ViewCompat.setOnApplyWindowInsetsListener(scrollView) { view, insets ->
            val top = insets.getInsets(WindowInsetsCompat.Type.statusBars() or WindowInsetsCompat.Type.displayCutout()).top
            // IMPORTANT: paddingTop() already includes any previous inset adjustments.
            // Use the original base padding to avoid repeatedly growing padding and
            // pushing content off-screen.
            val newTopPadding = basePaddingTop + top
            view.updatePadding(top = newTopPadding)
            insets
        }

        val statusText = findViewById<TextView>(R.id.status_text)
        val errorText = findViewById<TextView>(R.id.error_text)
        val inputPoll = findViewById<EditText>(R.id.input_poll_ms)
        val inputThreshold = findViewById<EditText>(R.id.input_threshold)
        val zonesText = findViewById<TextView>(R.id.zones_text)
        val btnApply = findViewById<Button>(R.id.btn_apply)

        btnApply.setOnClickListener {
            val pollStr = inputPoll.text?.toString()?.trim().orEmpty()
            val thrStr = inputThreshold.text?.toString()?.trim().orEmpty()
            val poll = pollStr.toLongOrNull()
            val thr = thrStr.toLongOrNull()
            if ((pollStr.isNotEmpty() && poll == null) ||
                (thrStr.isNotEmpty() && thr == null)
            ) {
                Toast.makeText(this, R.string.error_invalid_number, Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }
            viewModel.applyDaemonSettings(
                pollIntervalMillis = poll,
                notifyThresholdMilliCelsius = thr,
            )
        }

        lifecycleScope.launch {
            repeatOnLifecycle(Lifecycle.State.STARTED) {
                viewModel.uiState.collect { state ->
                    render(state, statusText, errorText, inputPoll, inputThreshold, zonesText)
                }
            }
        }
    }

    private fun render(
        state: ThermalUiState,
        statusText: TextView,
        errorText: TextView,
        inputPoll: EditText,
        inputThreshold: EditText,
        zonesText: TextView,
    ) {
        statusText.setText(
            if (state.connected) {
                R.string.status_connected
            } else {
                R.string.status_disconnected
            },
        )
        if (state.errorMessage != null) {
            errorText.visibility = View.VISIBLE
            errorText.text = state.errorMessage
        } else {
            errorText.visibility = View.GONE
            errorText.text = ""
        }
        if (!inputPoll.hasFocus()) {
            inputPoll.setText(
                if (state.connected) {
                    state.pollIntervalMillis.toString()
                } else {
                    ""
                },
            )
        }
        if (!inputThreshold.hasFocus()) {
            inputThreshold.setText(
                if (state.connected) {
                    state.notifyThresholdMilliCelsius.toString()
                } else {
                    ""
                },
            )
        }
        if (state.zones.isEmpty()) {
            zonesText.setText(R.string.zones_empty)
        } else {
            val sb = StringBuilder()
            for (z in state.zones) {
                sb.append(z.zoneName)
                    .append(": ")
                    .append(z.tempMilliCelsius)
                    .append(" m°C  (t=")
                    .append(z.timestampNanos)
                    .append(" ns)\n")
            }
            zonesText.text = sb.toString().trimEnd()
        }
    }
}
