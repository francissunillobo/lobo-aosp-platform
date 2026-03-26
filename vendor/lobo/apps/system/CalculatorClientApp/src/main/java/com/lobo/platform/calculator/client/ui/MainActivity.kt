// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

package com.lobo.platform.calculator.client.ui

import android.app.Activity
import android.os.Bundle
import android.widget.ScrollView
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import com.lobo.platform.calculator.client.R
import com.lobo.platform.calculator.di.AppModule

/**
 * Simple UI to exercise [com.lobo.platform.calculator.impl.CalculatorClientImpl] against calculatord.
 */
class MainActivity : Activity() {

    private val client by lazy { AppModule.provideCalculatorClient() }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_calculator_client)

        // On car displays the ScrollView can open scrolled toward B.
        // Force focus + scroll to A so the UI always shows both inputs.
        val scrollView = findViewById<ScrollView>(R.id.scroll_root)
        val inputA = findViewById<EditText>(R.id.input_a)
        val inputB = findViewById<EditText>(R.id.input_b)
        val resultText = findViewById<TextView>(R.id.result_text)

        scrollView.post {
            inputA.requestFocus()
            scrollView.scrollTo(0, 0)
        }

        fun parseInts(): Pair<Int, Int>? {
            val aStr = inputA.text?.toString()?.trim().orEmpty()
            val bStr = inputB.text?.toString()?.trim().orEmpty()
            if (aStr.isEmpty() || bStr.isEmpty()) {
                Toast.makeText(this, R.string.error_parse, Toast.LENGTH_SHORT).show()
                return null
            }
            return try {
                Pair(aStr.toInt(), bStr.toInt())
            } catch (_: NumberFormatException) {
                Toast.makeText(this, R.string.error_parse, Toast.LENGTH_SHORT).show()
                null
            }
        }

        fun setResult(value: String) {
            resultText.text = value
        }

        findViewById<Button>(R.id.btn_add).setOnClickListener {
            val p = parseInts() ?: return@setOnClickListener
            try {
                setResult("${client.add(p.first, p.second)}")
            } catch (e: RuntimeException) {
                Toast.makeText(this, R.string.error_service, Toast.LENGTH_SHORT).show()
            }
        }
        findViewById<Button>(R.id.btn_subtract).setOnClickListener {
            val p = parseInts() ?: return@setOnClickListener
            try {
                setResult("${client.subtract(p.first, p.second)}")
            } catch (e: RuntimeException) {
                Toast.makeText(this, R.string.error_service, Toast.LENGTH_SHORT).show()
            }
        }
        findViewById<Button>(R.id.btn_multiply).setOnClickListener {
            val p = parseInts() ?: return@setOnClickListener
            try {
                setResult("${client.multiply(p.first, p.second)}")
            } catch (e: RuntimeException) {
                Toast.makeText(this, R.string.error_service, Toast.LENGTH_SHORT).show()
            }
        }
        findViewById<Button>(R.id.btn_divide).setOnClickListener {
            val p = parseInts() ?: return@setOnClickListener
            try {
                setResult("${client.divide(p.first, p.second)}")
            } catch (e: Exception) {
                // ServiceSpecificException is not on the vendor compile classpath; match at runtime.
                if (e.javaClass.name == "android.os.ServiceSpecificException") {
                    Toast.makeText(this, R.string.error_divide_zero, Toast.LENGTH_SHORT).show()
                } else {
                    Toast.makeText(this, R.string.error_service, Toast.LENGTH_SHORT).show()
                }
            }
        }
    }
}
