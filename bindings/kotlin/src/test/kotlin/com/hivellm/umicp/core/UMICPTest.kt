package com.hivellm.umicp.core

import com.hivellm.umicp.types.OperationType
import io.kotest.matchers.shouldBe
import io.kotest.matchers.shouldNotBe
import io.kotest.matchers.string.shouldContain
import org.junit.jupiter.api.Test

class UMICPTest {

    @Test
    fun `should return version`() {
        val version = UMICP.getVersion()
        version shouldBe "0.1.2"
    }

    @Test
    fun `should return protocol version`() {
        val version = UMICP.getProtocolVersion()
        version shouldBe "1.0"
    }

    @Test
    fun `should return build type`() {
        val buildType = UMICP.getBuildType()
        buildType shouldBe "kotlin-pure"
    }

    @Test
    fun `should create envelope with factory method`() {
        val envelope = UMICP.createEnvelope {
            from("client-001")
            to("server-001")
            operation(OperationType.DATA)
        }

        envelope.from shouldBe "client-001"
        envelope.to shouldBe "server-001"
    }

    @Test
    fun `should serialize envelope`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        val json = UMICP.serialize(envelope)
        json shouldContain "client-001"
    }

    @Test
    fun `should deserialize envelope`() {
        val json = """{"from":"client-001","to":"server-001","operation":"DATA","messageId":"test","timestamp":1000,"capabilities":{}}"""
        val envelope = UMICP.deserialize(json)

        envelope.from shouldBe "client-001"
    }

    @Test
    fun `should validate envelope`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA
        )

        UMICP.validate(envelope) shouldBe true
    }

    @Test
    fun `should compute hash`() {
        val envelope = Envelope(
            from = "client-001",
            to = "server-001",
            operation = OperationType.DATA,
            messageId = "test",
            timestamp = 1000L
        )

        val hash = UMICP.hash(envelope)
        hash shouldNotBe ""
        hash.length shouldBe 64
    }
}

