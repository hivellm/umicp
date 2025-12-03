package com.hivellm.umicp.transport.http;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.transport.TransportStats;
import com.hivellm.umicp.transport.ServerEventListener;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.function.Function;

/**
 * HTTP/2 Server implementation for UMICP protocol.
 * Provides RESTful endpoints for envelope exchange with support for
 * health checks, statistics, and message handling.
 *
 * @author HiveLLM Team
 * @version 0.1.3
 */
public class UMICPHttpServer implements AutoCloseable {
    private static final Logger logger = LoggerFactory.getLogger(UMICPHttpServer.class);

    private final String host;
    private final int port;
    private final boolean enableCors;
    private final int maxRequestSize;
    private final TransportStats stats;
    private final Instant startedAt;

    private HttpServer server;
    private ExecutorService executor;
    private Function<Envelope, Envelope> messageHandler;
    private ServerEventListener eventListener;

    /**
     * Create new HTTP server with default settings.
     *
     * @param port Port to listen on
     */
    public UMICPHttpServer(int port) {
        this("0.0.0.0", port, true, 100 * 1024 * 1024); // 100MB max
    }

    /**
     * Create new HTTP server with custom settings.
     *
     * @param host Host to bind to
     * @param port Port to listen on
     * @param enableCors Enable CORS headers
     * @param maxRequestSize Maximum request size in bytes
     */
    public UMICPHttpServer(
            @NotNull String host,
            int port,
            boolean enableCors,
            int maxRequestSize) {
        this.host = host;
        this.port = port;
        this.enableCors = enableCors;
        this.maxRequestSize = maxRequestSize;
        this.stats = new TransportStats();
        this.startedAt = Instant.now();
    }

    /**
     * Set message handler callback.
     *
     * @param handler Function that processes envelope and returns optional response
     */
    public void setMessageHandler(@NotNull Function<Envelope, Envelope> handler) {
        this.messageHandler = handler;
    }

    /**
     * Set event listener for server events.
     *
     * @param listener Event listener
     */
    public void setEventListener(@Nullable ServerEventListener listener) {
        this.eventListener = listener;
    }

    /**
     * Start the HTTP server.
     *
     * @throws IOException if server fails to start
     */
    public void start() throws IOException {
        InetSocketAddress address = new InetSocketAddress(host, port);
        server = HttpServer.create(address, 0);

        // Create thread pool for handling requests
        executor = Executors.newFixedThreadPool(
                Runtime.getRuntime().availableProcessors() * 2
        );
        server.setExecutor(executor);

        // Setup routes
        server.createContext("/", new RootHandler());
        server.createContext("/health", new HealthHandler());
        server.createContext("/stats", new StatsHandler());
        server.createContext("/message", new MessageHandler());
        server.createContext("/envelope", new MessageHandler()); // Alias

        server.start();

        logger.info("HTTP server started on {}:{}", host, port);

        if (eventListener != null) {
            eventListener.onServerStarted();
        }
    }

    /**
     * Stop the HTTP server.
     */
    public void stop() {
        if (server != null) {
            server.stop(2); // 2 seconds delay
            logger.info("HTTP server stopped");
        }

        if (executor != null) {
            executor.shutdown();
        }

        if (eventListener != null) {
            eventListener.onServerStopped();
        }
    }

    /**
     * Get server statistics.
     *
     * @return Current statistics
     */
    @NotNull
    public TransportStats getStats() {
        return stats;
    }

    /**
     * Get server uptime in seconds.
     *
     * @return Uptime in seconds
     */
    public long getUptimeSeconds() {
        return Instant.now().getEpochSecond() - startedAt.getEpochSecond();
    }

    @Override
    public void close() {
        stop();
    }

    /**
     * Add CORS headers to response if enabled.
     */
    private void addCorsHeaders(HttpExchange exchange) {
        if (enableCors) {
            exchange.getResponseHeaders().add("Access-Control-Allow-Origin", "*");
            exchange.getResponseHeaders().add("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            exchange.getResponseHeaders().add("Access-Control-Allow-Headers", "Content-Type");
        }
    }

    /**
     * Root endpoint handler.
     */
    private class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            addCorsHeaders(exchange);

            String response = "UMICP HTTP Server v0.1.3";
            exchange.getResponseHeaders().add("Content-Type", "text/plain");
            exchange.sendResponseHeaders(200, response.getBytes().length);

            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    /**
     * Health check endpoint handler.
     */
    private class HealthHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            addCorsHeaders(exchange);

            String response = String.format(
                    "{\"status\":\"healthy\",\"uptime_seconds\":%d,\"version\":\"0.1.3\"}",
                    getUptimeSeconds()
            );

            exchange.getResponseHeaders().add("Content-Type", "application/json");
            exchange.sendResponseHeaders(200, response.getBytes().length);

            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    /**
     * Statistics endpoint handler.
     */
    private class StatsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            addCorsHeaders(exchange);

            String response = String.format(
                    "{\"messages_sent\":%d,\"messages_received\":%d,\"bytes_sent\":%d,\"bytes_received\":%d,\"active_connections\":%d,\"total_connections\":%d}",
                    stats.getMessagesSent(),
                    stats.getMessagesReceived(),
                    stats.getBytesSent(),
                    stats.getBytesReceived(),
                    stats.getActiveConnections(),
                    stats.getTotalConnections()
            );

            exchange.getResponseHeaders().add("Content-Type", "application/json");
            exchange.sendResponseHeaders(200, response.getBytes().length);

            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    /**
     * Message/envelope endpoint handler.
     */
    private class MessageHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            addCorsHeaders(exchange);

            // Handle OPTIONS for CORS preflight
            if ("OPTIONS".equals(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(204, -1);
                return;
            }

            // Only accept POST
            if (!"POST".equals(exchange.getRequestMethod())) {
                String response = "{\"error\":\"Method not allowed\"}";
                exchange.sendResponseHeaders(405, response.getBytes().length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            try {
                // Read request body
                InputStream is = exchange.getRequestBody();
                byte[] requestBytes = is.readAllBytes();

                if (requestBytes.length > maxRequestSize) {
                    String response = "{\"error\":\"Request too large\"}";
                    exchange.sendResponseHeaders(413, response.getBytes().length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(response.getBytes(StandardCharsets.UTF_8));
                    }
                    return;
                }

                stats.incrementMessagesReceived();
                stats.addBytesReceived(requestBytes.length);

                // Parse envelope
                String requestBody = new String(requestBytes, StandardCharsets.UTF_8);
                Envelope envelope = Envelope.fromJson(requestBody);

                // Call message handler
                Envelope responseEnvelope = null;
                if (messageHandler != null) {
                    try {
                        responseEnvelope = messageHandler.apply(envelope);
                    } catch (Exception e) {
                        logger.error("Error in message handler", e);
                        String response = String.format("{\"error\":\"Handler error: %s\"}", e.getMessage());
                        exchange.sendResponseHeaders(500, response.getBytes().length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(response.getBytes(StandardCharsets.UTF_8));
                        }
                        return;
                    }
                }

                // Send response
                exchange.getResponseHeaders().add("Content-Type", "application/json");

                String response;
                if (responseEnvelope != null) {
                    response = responseEnvelope.toJson();
                    stats.incrementMessagesSent();
                } else {
                    response = "{\"status\":\"ok\"}";
                }

                byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
                stats.addBytesSent(responseBytes.length);

                exchange.sendResponseHeaders(200, responseBytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(responseBytes);
                }

            } catch (Exception e) {
                logger.error("Error handling message", e);
                String response = String.format("{\"error\":\"%s\"}", e.getMessage());
                exchange.sendResponseHeaders(500, response.getBytes().length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            }
        }
    }
}

