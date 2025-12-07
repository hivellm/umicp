package com.hivellm.umicp.transport

import com.hivellm.umicp.core.Envelope
import kotlinx.coroutines.*
import mu.KotlinLogging
import org.java_websocket.WebSocket
import org.java_websocket.handshake.ClientHandshake
import org.java_websocket.server.WebSocketServer
import java.net.InetSocketAddress
import java.util.concurrent.ConcurrentHashMap

private val logger = KotlinLogging.logger {}

/**
 * WebSocket Server for UMICP
 *
 * @property host Server host
 * @property port Server port
 */
class UMICPWebSocketServer(
    private val host: String = "0.0.0.0",
    private val port: Int = 8080
) : AutoCloseable {

    private var server: InternalServer? = null
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())
    private val clients = ConcurrentHashMap<WebSocket, String>()

    private val onMessageHandlers = mutableListOf<suspend (String, Envelope) -> Unit>()
    private val onClientConnectHandlers = mutableListOf<suspend (String) -> Unit>()
    private val onClientDisconnectHandlers = mutableListOf<suspend (String) -> Unit>()
    private val onErrorHandlers = mutableListOf<suspend (String?, Exception) -> Unit>()

    /**
     * Start the server
     */
    fun start() {
        server = InternalServer(InetSocketAddress(host, port)).also {
            it.start()
        }
        logger.info { "WebSocket server started on $host:$port" }
    }

    /**
     * Stop the server
     */
    fun stop() {
        server?.stop()
        logger.info { "WebSocket server stopped" }
    }

    /**
     * Send an envelope to a specific client
     *
     * @param clientId Client identifier
     * @param envelope Envelope to send
     */
    fun send(clientId: String, envelope: Envelope) {
        val client = clients.entries.find { it.value == clientId }?.key
        if (client != null && client.isOpen) {
            val json = envelope.serialize()
            client.send(json)
        } else {
            logger.warn { "Client $clientId not found or not connected" }
        }
    }

    /**
     * Broadcast an envelope to all connected clients
     *
     * @param envelope Envelope to broadcast
     */
    fun broadcast(envelope: Envelope) {
        val json = envelope.serialize()
        server?.broadcast(json)
    }

    /**
     * Get list of connected client IDs
     */
    fun getConnectedClients(): List<String> {
        return clients.values.toList()
    }

    /**
     * Get number of connected clients
     */
    fun getClientCount(): Int = clients.size

    /**
     * Register a message handler
     */
    fun onMessage(handler: suspend (clientId: String, envelope: Envelope) -> Unit) {
        onMessageHandlers.add(handler)
    }

    /**
     * Register a client connect handler
     */
    fun onClientConnect(handler: suspend (String) -> Unit) {
        onClientConnectHandlers.add(handler)
    }

    /**
     * Register a client disconnect handler
     */
    fun onClientDisconnect(handler: suspend (String) -> Unit) {
        onClientDisconnectHandlers.add(handler)
    }

    /**
     * Register an error handler
     */
    fun onError(handler: suspend (String?, Exception) -> Unit) {
        onErrorHandlers.add(handler)
    }

    override fun close() {
        stop()
        scope.cancel()
        clients.clear()
    }

    private inner class InternalServer(address: InetSocketAddress) : WebSocketServer(address) {
        override fun onOpen(conn: WebSocket?, handshake: ClientHandshake?) {
            if (conn != null) {
                val clientId = conn.remoteSocketAddress.toString()
                clients[conn] = clientId

                scope.launch {
                    onClientConnectHandlers.forEach { it(clientId) }
                }

                logger.info { "Client connected: $clientId (total: ${clients.size})" }
            }
        }

        override fun onClose(conn: WebSocket?, code: Int, reason: String?, remote: Boolean) {
            if (conn != null) {
                val clientId = clients.remove(conn)
                if (clientId != null) {
                    scope.launch {
                        onClientDisconnectHandlers.forEach { it(clientId) }
                    }
                    logger.info { "Client disconnected: $clientId (total: ${clients.size})" }
                }
            }
        }

        override fun onMessage(conn: WebSocket?, message: String?) {
            if (conn != null && message != null) {
                scope.launch {
                    try {
                        val clientId = clients[conn] ?: return@launch
                        val envelope = Envelope.deserialize(message)
                        onMessageHandlers.forEach { handler -> handler(clientId, envelope) }
                    } catch (e: Exception) {
                        logger.error(e) { "Failed to process message: $message" }
                        val clientId = clients[conn]
                        onErrorHandlers.forEach { handler -> handler(clientId, e) }
                    }
                }
            }
        }

        override fun onError(conn: WebSocket?, ex: Exception?) {
            if (ex != null) {
                val clientId = conn?.let { clients[it] }
                scope.launch {
                    onErrorHandlers.forEach { handler -> handler(clientId, ex) }
                }
                logger.error(ex) { "WebSocket error for client: $clientId" }
            }
        }

        override fun onStart() {
            logger.info { "WebSocket server started successfully" }
        }
    }

    companion object {
        /**
         * Create a WebSocket server with DSL
         */
        inline fun create(host: String = "0.0.0.0", port: Int = 8080, block: UMICPWebSocketServer.() -> Unit = {}): UMICPWebSocketServer {
            return UMICPWebSocketServer(host, port).apply(block)
        }
    }
}

