package com.hivellm.umicp.examples

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.transport.UMICPHttpServer
import com.hivellm.umicp.types.OperationType
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking

/**
 * HTTP/2 Server Example
 *
 * Demonstrates:
 * - HTTP/2 server creation
 * - Route registration (GET, POST, PUT, DELETE)
 * - Envelope handling
 * - Event handlers
 */
fun main() = runBlocking {
    println("=== UMICP Kotlin SDK - HTTP/2 Server Example ===\n")

    // Create HTTP/2 server
    println("1. Creating HTTP/2 Server:")
    val server = UMICPHttpServer.create(host = "localhost", port = 8080, enableHttp2 = true) {

        // GET route
        get("/api/status") { request ->
            println("  [SERVER] GET /api/status")
            UMICPHttpServer.ok(
                body = """{"status":"online","version":"0.1.2","protocol":"HTTP/2"}"""
            )
        }

        // POST route
        post("/api/data") { request ->
            println("  [SERVER] POST /api/data")
            println("    Body: ${request.body?.take(100)}...")
            UMICPHttpServer.ok(
                body = """{"received":true,"size":${request.body?.length ?: 0}}"""
            )
        }

        // PUT route
        put("/api/update") { request ->
            println("  [SERVER] PUT /api/update")
            UMICPHttpServer.ok(
                body = """{"updated":true}"""
            )
        }

        // DELETE route
        delete("/api/resource/:id") { request ->
            println("  [SERVER] DELETE ${request.path}")
            UMICPHttpServer.ok(
                body = """{"deleted":true}"""
            )
        }

        // Envelope route (automatic envelope parsing)
        envelope("/api/message") { envelope ->
            println("  [SERVER] ENVELOPE /api/message")
            println("    From: ${envelope.from}")
            println("    To: ${envelope.to}")
            println("    Operation: ${envelope.operation}")

            // Respond with ACK
            Envelope.build {
                from("server")
                to(envelope.from)
                operation(OperationType.ACK)
                capability("original_message_id", envelope.messageId)
                capability("processed", "true")
            }
        }

        // Another envelope route
        envelope("/api/process") { envelope ->
            println("  [SERVER] ENVELOPE /api/process")

            // Simulate processing
            delay(100)

            // Respond with DATA
            Envelope.build {
                from("server")
                to(envelope.from)
                operation(OperationType.RESPONSE)
                capability("result", "processed")
                capability("computation_time", "100ms")
            }
        }

        // Event handlers
        onRequest { request ->
            println("  [EVENT] Request: ${request.method} ${request.path}")
        }

        onError { error ->
            println("  [ERROR] ${error.message}")
        }
    }

    // Start server
    println("  Starting server...")
    server.start()
    println("  ✓ Server running at ${server.getAddress()}")
    println("  ✓ HTTP/2 enabled")
    println()

    // Show available endpoints
    println("2. Available Endpoints:")
    println("  GET    /api/status       - Server status")
    println("  POST   /api/data         - Send data")
    println("  PUT    /api/update       - Update resource")
    println("  DELETE /api/resource/:id - Delete resource")
    println("  POST   /api/message      - Send UMICP envelope")
    println("  POST   /api/process      - Process UMICP envelope")
    println()

    // Keep server running
    println("3. Server Running:")
    println("  Test with curl:")
    println("    curl http://localhost:8080/api/status")
    println("    curl -X POST http://localhost:8080/api/data -d '{\"test\":\"data\"}'")
    println()
    println("  Or use the HTTP client example to interact with this server")
    println()
    println("  Press Ctrl+C to stop the server...")
    println()

    // Example: Simulating some requests
    println("4. Server Log (waiting for requests):")

    // Keep server alive for 60 seconds for testing
    repeat(60) { i ->
        delay(1000)
        if (i % 10 == 0) {
            println("  [INFO] Server uptime: ${i}s")
        }
    }

    // Shutdown
    println()
    println("5. Shutting Down:")
    server.stop()
    println("  ✓ Server stopped")
    println()

    println("=== Example Complete ===")
}

