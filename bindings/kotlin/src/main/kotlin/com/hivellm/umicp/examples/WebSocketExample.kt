package com.hivellm.umicp.examples

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.transport.UMICPWebSocketClient
import com.hivellm.umicp.transport.UMICPWebSocketServer
import com.hivellm.umicp.types.OperationType
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking

/**
 * WebSocket Example
 *
 * Demonstrates:
 * - WebSocket server
 * - WebSocket client
 * - Message handling
 * - Event handlers
 */
fun main() = runBlocking {
    println("=== UMICP Kotlin SDK - WebSocket Example ===\n")

    val host = "localhost"
    val port = 9876

    // Create and start server
    println("1. Starting WebSocket Server...")
    val server = UMICPWebSocketServer.create(host, port) {
        onClientConnect { clientId ->
            println("  [SERVER] Client connected: $clientId")
        }

        onClientDisconnect { clientId ->
            println("  [SERVER] Client disconnected: $clientId")
        }

        onMessage { clientId, envelope ->
            println("  [SERVER] Received from $clientId:")
            println("    From: ${envelope.from}")
            println("    To: ${envelope.to}")
            println("    Operation: ${envelope.operation}")

            // Echo back an acknowledgment
            val ack = Envelope.build {
                from("server")
                to(envelope.from)
                operation(OperationType.ACK)
                capability("original_message", envelope.messageId)
            }

            send(clientId, ack)
            println("  [SERVER] Sent ACK")
        }

        onError { clientId, error ->
            println("  [SERVER] Error for $clientId: ${error.message}")
        }
    }

    server.start()
    println("  Server started on $host:$port")
    println()

    delay(1000) // Wait for server to fully start

    // Create and connect client
    println("2. Starting WebSocket Client...")
    val client = UMICPWebSocketClient.create("ws://$host:$port") {
        onConnect {
            println("  [CLIENT] Connected to server")
        }

        onDisconnect {
            println("  [CLIENT] Disconnected from server")
        }

        onMessage { envelope ->
            println("  [CLIENT] Received:")
            println("    From: ${envelope.from}")
            println("    To: ${envelope.to}")
            println("    Operation: ${envelope.operation}")
        }

        onError { error ->
            println("  [CLIENT] Error: ${error.message}")
        }
    }

    client.connect()
    println()

    delay(1000) // Wait for connection

    // Send messages
    println("3. Sending Messages...")
    repeat(3) { i ->
        val envelope = Envelope.build {
            from("client-001")
            to("server")
            operation(OperationType.DATA)
            capability("sequence", i.toString())
            capability("message", "Hello from Kotlin!")
        }

        client.send(envelope)
        println("  [CLIENT] Sent message #$i")

        delay(500)
    }
    println()

    delay(1000) // Wait for responses

    // Server broadcast
    println("4. Server Broadcast:")
    val broadcast = Envelope.build {
        from("server")
        to("all")
        operation(OperationType.DATA)
        capability("type", "broadcast")
        capability("message", "Server announcement!")
    }

    server.broadcast(broadcast)
    println("  [SERVER] Broadcast sent to all clients")
    println()

    delay(1000)

    // Statistics
    println("5. Server Statistics:")
    println("  Connected clients: ${server.getClientCount()}")
    println("  Client IDs: ${server.getConnectedClients()}")
    println()

    // Cleanup
    println("6. Shutting Down...")
    client.disconnect()
    delay(500)
    server.stop()

    println("  Cleanup complete")
    println()

    println("=== Example Complete ===")
}

