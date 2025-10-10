package com.hivellm.umicp.types

import io.kotest.matchers.shouldBe
import io.kotest.matchers.shouldNotBe
import org.junit.jupiter.api.Test

class OperationTypeTest {
    @Test
    fun `should contain all expected operation types`() {
        OperationType.entries.size shouldBe 6
        OperationType.CONTROL shouldNotBe null
        OperationType.DATA shouldNotBe null
        OperationType.ACK shouldNotBe null
        OperationType.ERROR shouldNotBe null
        OperationType.REQUEST shouldNotBe null
        OperationType.RESPONSE shouldNotBe null
    }

    @Test
    fun `should convert from string case-insensitive`() {
        OperationType.fromString("DATA") shouldBe OperationType.DATA
        OperationType.fromString("data") shouldBe OperationType.DATA
        OperationType.fromString("DaTa") shouldBe OperationType.DATA
    }

    @Test
    fun `should return null for invalid string`() {
        OperationType.fromString("INVALID") shouldBe null
    }
}

class PayloadTypeTest {
    @Test
    fun `should contain all expected payload types`() {
        PayloadType.entries.size shouldBe 4
        PayloadType.VECTOR shouldNotBe null
        PayloadType.TEXT shouldNotBe null
        PayloadType.METADATA shouldNotBe null
        PayloadType.BINARY shouldNotBe null
    }

    @Test
    fun `should convert from string case-insensitive`() {
        PayloadType.fromString("VECTOR") shouldBe PayloadType.VECTOR
        PayloadType.fromString("vector") shouldBe PayloadType.VECTOR
    }
}

class EncodingTypeTest {
    @Test
    fun `should contain all expected encoding types`() {
        EncodingType.entries.size shouldBe 8
        EncodingType.FLOAT32 shouldNotBe null
        EncodingType.FLOAT64 shouldNotBe null
        EncodingType.INT32 shouldNotBe null
        EncodingType.INT64 shouldNotBe null
    }

    @Test
    fun `should convert from string case-insensitive`() {
        EncodingType.fromString("FLOAT32") shouldBe EncodingType.FLOAT32
        EncodingType.fromString("float32") shouldBe EncodingType.FLOAT32
    }
}

