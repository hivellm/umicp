package com.hivellm.umicp.transport

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.types.OperationType
import io.kotest.matchers.shouldBe
import io.kotest.matchers.shouldNotBe
import io.kotest.matchers.string.shouldContain
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.condition.EnabledIf

/**
 * HTTP Server Tests
 *
 * Note: Integration tests require server to be running
 */
class HttpServerTest {

    companion object {
        fun integrationTestsEnabled(): Boolean {
            return System.getProperty("enable.integration.tests", "false").toBoolean()
        }
    }

    @Test
    fun `should create HTTP server`() {
        val server = UMICPHttpServer.create("localhost", 9090)

        server shouldNotBe null
        server.close()
    }

    @Test
    fun `should create HTTP server with HTTP2 disabled`() {
        val server = UMICPHttpServer("localhost", 9091, enableHttp2 = false)

        server shouldNotBe null
        server.close()
    }

    @Test
    fun `should register routes`() {
        val server = UMICPHttpServer.create("localhost", 9092) {
            get("/test") { request ->
                UMICPHttpServer.ok("Test response")
            }

            post("/data") { request ->
                UMICPHttpServer.ok(body = """{"status":"ok"}""")
            }
        }

        server shouldNotBe null
        server.close()
    }

    @Test
    fun `should create OK response`() {
        val response = UMICPHttpServer.ok("Test body")

        response.statusCode shouldBe 200
        response.body shouldBe "Test body"
    }

    @Test
    fun `should create error response`() {
        val response = UMICPHttpServer.error("Test error", 404)

        response.statusCode shouldBe 404
        response.body shouldContain "Test error"
    }

    @Test
    @EnabledIf("integrationTestsEnabled")
    fun `should start and stop server`() = runBlocking {
        val server = UMICPHttpServer.create("localhost", 9093)

        server.start()
        server.isRunning() shouldBe true

        delay(100)

        server.stop()

        server.close()
    }

    @Test
    @EnabledIf("integrationTestsEnabled")
    fun `should handle GET requests`() = runBlocking {
        val server = UMICPHttpServer.create("localhost", 9094) {
            get("/hello") { request ->
                UMICPHttpServer.ok("""{"message":"Hello, World!"}""")
            }
        }

        server.start()
        delay(100)

        // Make request with HTTP client
        val client = UMICPHttpClient.create("http://localhost:9094")
        val response = client.get("/hello")

        response shouldNotBe null

        client.close()
        server.stop()
    }

    @Test
    @EnabledIf("integrationTestsEnabled")
    fun `should handle POST requests with envelopes`() = runBlocking {
        val server = UMICPHttpServer.create("localhost", 9095) {
            envelope("/api/message") { request ->
                // Echo back with ACK
                Envelope.build {
                    from("server")
                    to(request.from)
                    operation(OperationType.ACK)
                    capability("original_from", request.from)
                }
            }
        }

        server.start()
        delay(100)

        val client = UMICPHttpClient.create("http://localhost:9095")
        val envelope = Envelope.build {
            from("client")
            to("server")
            operation(OperationType.DATA)
        }

        val response = client.post("/api/message", envelope)
        response.operation shouldBe OperationType.ACK

        client.close()
        server.stop()
    }

    @Test
    fun `should get server address`() {
        val server = UMICPHttpServer("localhost", 9096)

        server.getAddress() shouldBe "http://localhost:9096"

        server.close()
    }
}

