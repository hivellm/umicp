package com.hivellm.umicp.examples

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.transport.UMICPHttpClient
import com.hivellm.umicp.types.OperationType
import kotlinx.coroutines.runBlocking

/**
 * HTTP/2 Client Example
 * 
 * Demonstrates:
 * - HTTP/2 client usage
 * - POST/GET/PUT/DELETE requests
 * - Protocol detection
 * 
 * Note: Requires a running HTTP server
 */
fun main() = runBlocking {
    println("=== UMICP Kotlin SDK - HTTP/2 Example ===\n")
    
    val baseUrl = "http://localhost:8080"
    
    println("Note: This example requires a running UMICP HTTP server")
    println("      Start a server first or the requests will fail")
    println()
    
    // Create HTTP client
    println("1. Creating HTTP/2 Client:")
    val client = UMICPHttpClient.create(baseUrl, timeout = 30_000)
    println("  ✓ Client created for $baseUrl")
    println()
    
    // Check HTTP/2 support
    println("2. Checking HTTP/2 Support:")
    try {
        val isHttp2 = client.isHttp2()
        if (isHttp2) {
            println("  ✓ HTTP/2 is supported")
        } else {
            println("  ⚠ HTTP/2 not available, using HTTP/1.1")
        }
    } catch (e: Exception) {
        println("  ⚠ Server not reachable: ${e.message}")
    }
    println()
    
    // Example envelope
    val envelope = Envelope.build {
        from("http-client")
        to("http-server")
        operation(OperationType.DATA)
        capability("method", "POST")
        capability("message", "Hello from Kotlin HTTP client!")
    }
    
    // POST request
    println("3. POST Request:")
    println("  Endpoint: /api/messages")
    try {
        val response = client.post("/api/messages", envelope)
        println("  ✓ Response received")
        println("    From: ${response.from}")
        println("    Operation: ${response.operation}")
    } catch (e: Exception) {
        println("  ✗ Error: ${e.message}")
    }
    println()
    
    // GET request
    println("4. GET Request:")
    println("  Endpoint: /api/status")
    try {
        val response = client.get("/api/status")
        println("  ✓ Response received")
        println("    Operation: ${response.operation}")
    } catch (e: Exception) {
        println("  ✗ Error: ${e.message}")
    }
    println()
    
    // PUT request
    println("5. PUT Request:")
    println("  Endpoint: /api/update")
    try {
        val updateEnvelope = envelope.toBuilder()
            .capability("action", "update")
            .build()
        
        val response = client.put("/api/update", updateEnvelope)
        println("  ✓ Response received")
    } catch (e: Exception) {
        println("  ✗ Error: ${e.message}")
    }
    println()
    
    // DELETE request
    println("6. DELETE Request:")
    println("  Endpoint: /api/resource/123")
    try {
        val response = client.delete("/api/resource/123")
        println("  ✓ Response received")
    } catch (e: Exception) {
        println("  ✗ Error: ${e.message}")
    }
    println()
    
    // Cleanup
    println("7. Cleanup:")
    client.close()
    println("  ✓ Client closed")
    println()
    
    println("=== Example Complete ===")
    println()
    println("To run a full example:")
    println("1. Start a UMICP HTTP server on port 8080")
    println("2. Run this example again")
    println("3. Check server logs for received messages")
}

