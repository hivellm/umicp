package com.hivellm.umicp.transport

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.types.OperationType
import com.hivellm.umicp.types.TransportException
import io.undertow.Undertow
import io.undertow.server.HttpHandler
import io.undertow.server.HttpServerExchange
import io.undertow.util.Headers
import io.undertow.util.Methods
import io.undertow.util.StatusCodes
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import mu.KotlinLogging
import java.util.concurrent.ConcurrentHashMap

private val logger = KotlinLogging.logger {}

/**
 * HTTP/2 Server for UMICP
 *
 * Provides HTTP/2 server with automatic protocol negotiation.
 *
 * @property host Server host
 * @property port Server port
 * @property enableHttp2 Enable HTTP/2 protocol
 */
class UMICPHttpServer(
    private val host: String = "0.0.0.0",
    private val port: Int = 8080,
    private val enableHttp2: Boolean = true
) : AutoCloseable {

    private var server: Undertow? = null
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())
    private val routes = ConcurrentHashMap<String, RouteHandler>()

    private val onRequestHandlers = mutableListOf<suspend (HttpRequest) -> Unit>()
    private val onErrorHandlers = mutableListOf<suspend (Exception) -> Unit>()

    /**
     * Route handler function type
     */
    typealias RouteHandler = suspend (HttpRequest) -> HttpResponse

    /**
     * HTTP Request representation
     */
    data class HttpRequest(
        val method: String,
        val path: String,
        val headers: Map<String, String>,
        val body: String?,
        val envelope: Envelope?
    )

    /**
     * HTTP Response representation
     */
    data class HttpResponse(
        val statusCode: Int = 200,
        val headers: Map<String, String> = emptyMap(),
        val body: String? = null,
        val envelope: Envelope? = null
    )

    /**
     * Start the HTTP/2 server
     */
    fun start() {
        val builder = Undertow.builder()
            .addHttpListener(port, host)
            .setHandler(createHandler())

        if (enableHttp2) {
            builder.setServerOption(io.undertow.UndertowOptions.ENABLE_HTTP2, true)
        }

        server = builder.build()
        server?.start()

        logger.info { "HTTP/2 server started on $host:$port (HTTP/2: $enableHttp2)" }
    }

    /**
     * Stop the HTTP/2 server
     */
    fun stop() {
        server?.stop()
        logger.info { "HTTP/2 server stopped" }
    }

    /**
     * Register a POST route
     *
     * @param path Route path
     * @param handler Handler function
     */
    fun post(path: String, handler: RouteHandler) {
        routes["POST:$path"] = handler
        logger.debug { "Registered POST route: $path" }
    }

    /**
     * Register a GET route
     *
     * @param path Route path
     * @param handler Handler function
     */
    fun get(path: String, handler: RouteHandler) {
        routes["GET:$path"] = handler
        logger.debug { "Registered GET route: $path" }
    }

    /**
     * Register a PUT route
     *
     * @param path Route path
     * @param handler Handler function
     */
    fun put(path: String, handler: RouteHandler) {
        routes["PUT:$path"] = handler
        logger.debug { "Registered PUT route: $path" }
    }

    /**
     * Register a DELETE route
     *
     * @param path Route path
     * @param handler Handler function
     */
    fun delete(path: String, handler: RouteHandler) {
        routes["DELETE:$path"] = handler
        logger.debug { "Registered DELETE route: $path" }
    }

    /**
     * Register an envelope route (POST with automatic envelope parsing)
     *
     * @param path Route path
     * @param handler Handler function that receives and returns envelopes
     */
    fun envelope(path: String, handler: suspend (Envelope) -> Envelope) {
        post(path) { request ->
            try {
                val envelope = request.envelope
                    ?: request.body?.let { Envelope.deserialize(it) }
                    ?: throw TransportException("No envelope in request")

                val response = handler(envelope)

                HttpResponse(
                    statusCode = 200,
                    headers = mapOf("Content-Type" to "application/json"),
                    envelope = response,
                    body = response.serialize()
                )
            } catch (e: Exception) {
                logger.error(e) { "Error processing envelope request" }
                HttpResponse(
                    statusCode = 500,
                    body = """{"error":"${e.message}"}"""
                )
            }
        }
    }

    /**
     * Register a request handler
     */
    fun onRequest(handler: suspend (HttpRequest) -> Unit) {
        onRequestHandlers.add(handler)
    }

    /**
     * Register an error handler
     */
    fun onError(handler: suspend (Exception) -> Unit) {
        onErrorHandlers.add(handler)
    }

    /**
     * Check if server is running
     */
    fun isRunning(): Boolean = server != null

    /**
     * Get server address
     */
    fun getAddress(): String = "http://$host:$port"

    private fun createHandler(): HttpHandler {
        return HttpHandler { exchange ->
            exchange.dispatch {
                scope.launch {
                    try {
                        handleRequest(exchange)
                    } catch (e: Exception) {
                        logger.error(e) { "Error handling request" }
                        onErrorHandlers.forEach { it(e) }
                        sendErrorResponse(exchange, e)
                    }
                }
            }
        }
    }

    private suspend fun handleRequest(exchange: HttpServerExchange) {
        val method = exchange.requestMethod.toString()
        val path = exchange.requestPath
        val routeKey = "$method:$path"

        // Parse request
        val headers = mutableMapOf<String, String>()
        exchange.requestHeaders.forEach { header ->
            headers[header.headerName.toString()] = header.first
        }

        val body = if (exchange.requestMethod == Methods.POST ||
                       exchange.requestMethod == Methods.PUT) {
            exchange.inputStream.bufferedReader().use { it.readText() }
        } else {
            null
        }

        val envelope = body?.let {
            try {
                Envelope.deserialize(it)
            } catch (e: Exception) {
                null
            }
        }

        val request = HttpRequest(method, path, headers, body, envelope)

        // Notify handlers
        onRequestHandlers.forEach { it(request) }

        // Find and execute route handler
        val handler = routes[routeKey]

        if (handler != null) {
            val response = handler(request)
            sendResponse(exchange, response)
        } else {
            sendNotFoundResponse(exchange)
        }
    }

    private fun sendResponse(exchange: HttpServerExchange, response: HttpResponse) {
        exchange.statusCode = response.statusCode

        response.headers.forEach { (key, value) ->
            exchange.responseHeaders.put(io.undertow.util.HttpString(key), value)
        }

        if (response.body != null) {
            exchange.responseHeaders.put(Headers.CONTENT_TYPE, "application/json")
            exchange.responseSender.send(response.body)
        } else if (response.envelope != null) {
            exchange.responseHeaders.put(Headers.CONTENT_TYPE, "application/json")
            exchange.responseSender.send(response.envelope.serialize())
        } else {
            exchange.responseSender.send("")
        }
    }

    private fun sendNotFoundResponse(exchange: HttpServerExchange) {
        exchange.statusCode = StatusCodes.NOT_FOUND
        exchange.responseHeaders.put(Headers.CONTENT_TYPE, "application/json")
        exchange.responseSender.send("""{"error":"Route not found"}""")
    }

    private fun sendErrorResponse(exchange: HttpServerExchange, error: Exception) {
        exchange.statusCode = StatusCodes.INTERNAL_SERVER_ERROR
        exchange.responseHeaders.put(Headers.CONTENT_TYPE, "application/json")
        exchange.responseSender.send("""{"error":"${error.message}"}""")
    }

    override fun close() {
        stop()
    }

    companion object {
        /**
         * Create HTTP server with DSL
         */
        inline fun create(
            host: String = "0.0.0.0",
            port: Int = 8080,
            enableHttp2: Boolean = true,
            block: UMICPHttpServer.() -> Unit = {}
        ): UMICPHttpServer {
            return UMICPHttpServer(host, port, enableHttp2).apply(block)
        }

        /**
         * Helper to create a simple OK response
         */
        fun ok(body: String? = null, envelope: Envelope? = null): HttpResponse {
            return HttpResponse(
                statusCode = 200,
                body = body,
                envelope = envelope
            )
        }

        /**
         * Helper to create an error response
         */
        fun error(message: String, statusCode: Int = 500): HttpResponse {
            return HttpResponse(
                statusCode = statusCode,
                body = """{"error":"$message"}"""
            )
        }
    }
}

