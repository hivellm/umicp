package com.hivellm.umicp.transport

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.types.ConnectionException
import com.hivellm.umicp.types.TransportException
import kotlinx.coroutines.*
import kotlinx.coroutines.channels.Channel
import mu.KotlinLogging
import org.java_websocket.client.WebSocketClient
import org.java_websocket.handshake.ServerHandshake
import java.net.URI
import java.util.concurrent.atomic.AtomicBoolean

private val logger = KotlinLogging.logger {}

/**
 * WebSocket Client for UMICP
 *
 * @property uri WebSocket server URI
 * @property autoReconnect Enable automatic reconnection
 * @property reconnectDelay Delay between reconnection attempts (ms)
 */
class UMICPWebSocketClient(
    private val uri: URI,
    private val autoReconnect: Boolean = true,
    private val reconnectDelay: Long = 5000
) : AutoCloseable {

    private var client: InternalClient? = null
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())
    private val messageChannel = Channel<Envelope>(Channel.UNLIMITED)
    private val connected = AtomicBoolean(false)

    private val onMessageHandlers = mutableListOf<suspend (Envelope) -> Unit>()
    private val onConnectHandlers = mutableListOf<suspend () -> Unit>()
    private val onDisconnectHandlers = mutableListOf<suspend () -> Unit>()
    private val onErrorHandlers = mutableListOf<suspend (Exception) -> Unit>()

    /**
     * Connect to the WebSocket server
     */
    suspend fun connect() = withContext(Dispatchers.IO) {
        if (connected.get()) {
            logger.warn { "Already connected to $uri" }
            return@withContext
        }

        client = InternalClient(uri).also {
            it.connect()
        }

        // Wait for connection
        var attempts = 0
        while (!connected.get() && attempts < 50) {
            delay(100)
            attempts++
        }

        if (!connected.get()) {
            throw ConnectionException("Failed to connect to $uri after ${attempts * 100}ms")
        }

        logger.info { "Connected to $uri" }
    }

    /**
     * Disconnect from the server
     */
    suspend fun disconnect() = withContext(Dispatchers.IO) {
        client?.close()
        connected.set(false)
        logger.info { "Disconnected from $uri" }
    }

    /**
     * Send an envelope
     *
     * @param envelope Envelope to send
     */
    suspend fun send(envelope: Envelope) = withContext(Dispatchers.IO) {
        if (!connected.get()) {
            throw TransportException("Not connected")
        }

        val json = envelope.serialize()
        client?.send(json)
    }

    /**
     * Register a message handler
     */
    fun onMessage(handler: suspend (Envelope) -> Unit) {
        onMessageHandlers.add(handler)
    }

    /**
     * Register a connect handler
     */
    fun onConnect(handler: suspend () -> Unit) {
        onConnectHandlers.add(handler)
    }

    /**
     * Register a disconnect handler
     */
    fun onDisconnect(handler: suspend () -> Unit) {
        onDisconnectHandlers.add(handler)
    }

    /**
     * Register an error handler
     */
    fun onError(handler: suspend (Exception) -> Unit) {
        onErrorHandlers.add(handler)
    }

    /**
     * Check if connected
     */
    fun isConnected(): Boolean = connected.get()

    override fun close() {
        runBlocking {
            disconnect()
        }
        scope.cancel()
        messageChannel.close()
    }

    private inner class InternalClient(uri: URI) : WebSocketClient(uri) {
        override fun onOpen(handshakedata: ServerHandshake?) {
            connected.set(true)
            scope.launch {
                onConnectHandlers.forEach { it() }
            }
        }

        override fun onMessage(message: String?) {
            message?.let {
                scope.launch {
                    try {
                        val envelope = Envelope.deserialize(it)
                        messageChannel.send(envelope)
                        onMessageHandlers.forEach { handler -> handler(envelope) }
                    } catch (e: Exception) {
                        logger.error(e) { "Failed to process message: $it" }
                        onErrorHandlers.forEach { handler -> handler(e) }
                    }
                }
            }
        }

        override fun onClose(code: Int, reason: String?, remote: Boolean) {
            connected.set(false)
            scope.launch {
                onDisconnectHandlers.forEach { it() }

                if (autoReconnect && !isClosed) {
                    logger.info { "Attempting to reconnect in ${reconnectDelay}ms..." }
                    delay(reconnectDelay)
                    try {
                        connect()
                    } catch (e: Exception) {
                        logger.error(e) { "Reconnection failed" }
                    }
                }
            }
        }

        override fun onError(ex: Exception?) {
            ex?.let {
                scope.launch {
                    onErrorHandlers.forEach { handler -> handler(it) }
                }
            }
        }
    }

    companion object {
        /**
         * Create a WebSocket client with DSL
         */
        inline fun create(uri: String, block: UMICPWebSocketClient.() -> Unit = {}): UMICPWebSocketClient {
            return UMICPWebSocketClient(URI(uri)).apply(block)
        }
    }
}

