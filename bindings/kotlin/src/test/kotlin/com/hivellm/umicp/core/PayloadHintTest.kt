package com.hivellm.umicp.core

import com.hivellm.umicp.types.EncodingType
import com.hivellm.umicp.types.PayloadType
import io.kotest.matchers.shouldBe
import org.junit.jupiter.api.Test

class PayloadHintTest {

    @Test
    fun `should create payload hint with required fields`() {
        val hint = PayloadHint(
            type = PayloadType.VECTOR
        )

        hint.type shouldBe PayloadType.VECTOR
    }

    @Test
    fun `should create payload hint with all fields`() {
        val hint = PayloadHint(
            type = PayloadType.VECTOR,
            encoding = EncodingType.FLOAT32,
            count = 768,
            shape = listOf(768)
        )

        hint.type shouldBe PayloadType.VECTOR
        hint.encoding shouldBe EncodingType.FLOAT32
        hint.count shouldBe 768
        hint.shape shouldBe listOf(768)
    }

    @Test
    fun `should create payload hint with builder`() {
        val hint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .shape(768)
            .build()

        hint.type shouldBe PayloadType.VECTOR
        hint.shape shouldBe listOf(768)
    }

    @Test
    fun `should create payload hint with DSL`() {
        val hint = PayloadHint.build {
            type(PayloadType.VECTOR)
            encoding(EncodingType.FLOAT32)
            count(1024)
            shape(32, 32)
        }

        hint.count shouldBe 1024
        hint.shape shouldBe listOf(32, 32)
    }

    @Test
    fun `should handle multi-dimensional shapes`() {
        val hint = PayloadHint.build {
            type(PayloadType.VECTOR)
            shape(3, 224, 224) // Image dimensions
        }

        hint.shape shouldBe listOf(3, 224, 224)
    }
}

