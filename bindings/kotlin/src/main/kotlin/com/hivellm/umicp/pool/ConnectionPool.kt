package com.hivellm.umicp.pool

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.transport.UMICPWebSocketClient
import kotlinx.coroutines.*
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import mu.KotlinLogging
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicInteger

private val logger = KotlinLogging.logger {}

/**
 * Pooled connection wrapper
 */
data class PooledConnection(
    val id: String,
    val client: UMICPWebSocketClient,
    var lastUsed: Long = System.currentTimeMillis(),
    var inUse: Boolean = false
)

/**
 * Connection Pool Statistics
 */
data class PoolStats(
    val totalConnections: Int,
    val activeConnections: Int,
    val idleConnections: Int,
    val totalRequests: Int,
    val totalErrors: Int
)

/**
 * Connection Pool for UMICP WebSocket clients
 *
 * @property uri Server URI
 * @property minSize Minimum pool size
 * @property maxSize Maximum pool size
 * @property maxIdleTime Maximum idle time (ms)
 */
class ConnectionPool(
    private val uri: String,
    private val minSize: Int = 2,
    private val maxSize: Int = 10,
    private val maxIdleTime: Long = 300_000 // 5 minutes
) : AutoCloseable {

    private val connections = ConcurrentHashMap<String, PooledConnection>()
    private val mutex = Mutex()
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    private val totalRequests = AtomicInteger(0)
    private val totalErrors = AtomicInteger(0)
    private val connectionCounter = AtomicInteger(0)

    init {
        // Start cleanup task
        scope.launch {
            while (isActive) {
                delay(60_000) // Check every minute
                cleanupStaleConnections()
            }
        }
    }

    /**
     * Initialize the pool with minimum connections
     */
    suspend fun initialize() {
        repeat(minSize) {
            createConnection()
        }
        logger.info { "Connection pool initialized with $minSize connections to $uri" }
    }

    /**
     * Acquire a connection from the pool
     *
     * @return Pooled connection
     */
    suspend fun acquire(): PooledConnection = mutex.withLock {
        totalRequests.incrementAndGet()

        // Try to find an idle connection
        val idle = connections.values.find { !it.inUse && it.client.isConnected() }
        if (idle != null) {
            idle.inUse = true
            idle.lastUsed = System.currentTimeMillis()
            return@withLock idle
        }

        // Create new connection if under max size
        if (connections.size < maxSize) {
            val conn = createConnection()
            conn.inUse = true
            return@withLock conn
        }

        // Wait for a connection to become available
        // In a real implementation, use a condition variable
        throw IllegalStateException("Connection pool exhausted")
    }

    /**
     * Release a connection back to the pool
     *
     * @param connection Connection to release
     */
    suspend fun release(connection: PooledConnection) = mutex.withLock {
        connection.inUse = false
        connection.lastUsed = System.currentTimeMillis()
    }

    /**
     * Send an envelope using a pooled connection
     *
     * @param envelope Envelope to send
     */
    suspend fun send(envelope: Envelope) {
        val conn = acquire()
        try {
            conn.client.send(envelope)
        } catch (e: Exception) {
            totalErrors.incrementAndGet()
            throw e
        } finally {
            release(conn)
        }
    }

    /**
     * Execute a block with a pooled connection
     *
     * @param block Block to execute
     * @return Result of the block
     */
    suspend fun <T> withConnection(block: suspend (UMICPWebSocketClient) -> T): T {
        val conn = acquire()
        return try {
            block(conn.client)
        } catch (e: Exception) {
            totalErrors.incrementAndGet()
            throw e
        } finally {
            release(conn)
        }
    }

    /**
     * Get pool statistics
     */
    fun getStats(): PoolStats {
        val active = connections.values.count { it.inUse }
        return PoolStats(
            totalConnections = connections.size,
            activeConnections = active,
            idleConnections = connections.size - active,
            totalRequests = totalRequests.get(),
            totalErrors = totalErrors.get()
        )
    }

    /**
     * Clean up stale connections
     */
    private suspend fun cleanupStaleConnections() = mutex.withLock {
        val now = System.currentTimeMillis()
        val stale = connections.values.filter {
            !it.inUse && (now - it.lastUsed) > maxIdleTime && connections.size > minSize
        }

        stale.forEach { conn ->
            connections.remove(conn.id)
            conn.client.close()
            logger.debug { "Removed stale connection: ${conn.id}" }
        }

        if (stale.isNotEmpty()) {
            logger.info { "Cleaned up ${stale.size} stale connections" }
        }
    }

    /**
     * Create a new connection
     */
    private suspend fun createConnection(): PooledConnection {
        val id = "conn-${connectionCounter.incrementAndGet()}"
        val client = UMICPWebSocketClient.create(uri)
        client.connect()

        val pooled = PooledConnection(id, client)
        connections[id] = pooled

        logger.debug { "Created connection: $id" }
        return pooled
    }

    override fun close() {
        scope.cancel()
        connections.values.forEach { it.client.close() }
        connections.clear()
        logger.info { "Connection pool closed" }
    }
}

