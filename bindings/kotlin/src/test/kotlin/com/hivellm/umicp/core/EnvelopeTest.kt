package com.hivellm.umicp.core

import com.hivellm.umicp.types.EncodingType
import com.hivellm.umicp.types.OperationType
import com.hivellm.umicp.types.PayloadType
import com.hivellm.umicp.types.ValidationException
import io.kotest.assertions.throwables.shouldThrow
import io.kotest.matchers.shouldBe
import io.kotest.matchers.shouldNotBe
import io.kotest.matchers.string.shouldContain
import io.kotest.matchers.string.shouldNotBeBlank
import org.junit.jupiter.api.Test

class EnvelopeTest {

    @Test
    fun `should create envelope with required fields`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        envelope.from shouldBe "client-001"
        envelope.to shouldBe "server-001"
        envelope.operation shouldBe OperationType.DATA
        envelope.messageId.shouldNotBeBlank()
        envelope.timestamp shouldNotBe 0L
    }

    @Test
    fun `should create envelope with builder`() {
        val envelope = Envelope.builder()
            .from("client-001")
            .to("server-001")
            .operation(OperationType.DATA)
            .capability("priority", "high")
            .build()

        envelope.from shouldBe "client-001"
        envelope.capabilities["priority"] shouldBe "high"
    }

    @Test
    fun `should create envelope with DSL`() {
        val envelope = Envelope.build {
            from("client-001")
            to("server-001")
            operation(OperationType.DATA)
            capability("test", "value")
        }

        envelope.from shouldBe "client-001"
        envelope.capabilities["test"] shouldBe "value"
    }

    @Test
    fun `should validate correct envelope`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        envelope.validate() shouldBe true
    }

    @Test
    fun `should fail validation with blank from`() {
        val envelope = Envelope(
            from = "",
            to = "server-001",
            operation = OperationType.DATA
        )

        envelope.validate() shouldBe false

        shouldThrow<ValidationException> {
            envelope.validateOrThrow()
        }
    }

    @Test
    fun `should fail validation with blank to`() {
        val envelope = Envelope(
            from = "client-001",
            to = "",
            operation = OperationType.DATA
        )

        envelope.validate() shouldBe false
    }

    @Test
    fun `should serialize to JSON`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        val json = envelope.serialize()
        json shouldContain "client-001"
        json shouldContain "server-001"
        json shouldContain "DATA"
    }

    @Test
    fun `should deserialize from JSON`() {
        val original = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        val json = original.serialize()
        val deserialized = Envelope.deserialize(json)

        deserialized.from shouldBe original.from
        deserialized.to shouldBe original.to
        deserialized.operation shouldBe original.operation
    }

    @Test
    fun `should compute consistent hash`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA,
            messageId = "test-123",
            timestamp = 1000L
        )

        val hash1 = envelope.hash()
        val hash2 = envelope.hash()

        hash1 shouldBe hash2
        hash1.length shouldBe 64 // SHA-256 hex string
    }

    @Test
    fun `should create envelope with payload hint`() {
        val hint = PayloadHint(
            type = PayloadType.VECTOR,
            encoding = EncodingType.FLOAT32,
            count = 768
        )

        val envelope = Envelope.builder()
            .from("client-001")
            .to("server-001")
            .operation(OperationType.DATA)
            .payloadHint(hint)
            .build()

        envelope.payloadHint shouldNotBe null
        envelope.payloadHint?.type shouldBe PayloadType.VECTOR
        envelope.payloadHint?.count shouldBe 768
    }

    @Test
    fun `should convert to builder`() {
        val original = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        val modified = original.toBuilder()
            .from("client-002")
            .build()

        modified.from shouldBe "client-002"
        modified.to shouldBe original.to
    }
}

