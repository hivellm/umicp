package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.ConnectionException;
import com.hivellm.umicp.types.SerializationException;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;
import org.jetbrains.annotations.NotNull;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * WebSocket client for UMICP communication.
 *
 * <p>This class provides WebSocket client functionality with auto-reconnection,
 * heartbeat, and message queuing capabilities.
 *
 * <p>Example usage:
 * <pre>{@code
 * ClientOptions options = ClientOptions.builder()
 *     .autoReconnect(true)
 *     .heartbeatInterval(Duration.ofSeconds(30))
 *     .build();
 *
 * UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);
 *
 * // Add event listener
 * client.addEventListener(new TransportEventListener() {
 *     @Override
 *     public void onMessage(Envelope envelope) {
 *         System.out.println("Received: " + envelope.getFrom());
 *     }
 *
 *     @Override
 *     public void onConnected() {
 *         System.out.println("Connected to server");
 *     }
 * });
 *
 * // Connect
 * client.connect().get();
 *
 * // Send message
 * Envelope message = new Envelope()
 *     .setFrom("client")
 *     .setTo("server")
 *     .setOperation(OperationType.DATA);
 *
 * client.send(message).get();
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see ClientOptions
 * @see TransportEventListener
 */
public class UMICPWebSocketClient implements AutoCloseable {

    private static final Logger logger = LoggerFactory.getLogger(UMICPWebSocketClient.class);

    private final String url;
    private final ClientOptions options;
    private final TransportStats stats;
    private final BlockingQueue<Envelope> messageQueue;
    private final CopyOnWriteArrayList<TransportEventListener> listeners;
    private final AtomicBoolean connected = new AtomicBoolean(false);
    private final AtomicInteger reconnectAttempts = new AtomicInteger(0);

    private WebSocketClient wsClient;
    private ScheduledExecutorService scheduler;
    private ScheduledFuture<?> heartbeatTask;

    /**
     * Creates a new WebSocket client with default options.
     *
     * @param url the WebSocket URL (e.g., "ws://localhost:8080/umicp")
     * @throws ConnectionException if the URL is invalid
     */
    public UMICPWebSocketClient(@NotNull String url) throws ConnectionException {
        this(url, ClientOptions.defaults());
    }

    /**
     * Creates a new WebSocket client with the given options.
     *
     * @param url the WebSocket URL
     * @param options the client options
     * @throws ConnectionException if the URL is invalid
     */
    public UMICPWebSocketClient(@NotNull String url, @NotNull ClientOptions options)
            throws ConnectionException {
        this.url = url;
        this.options = options;
        this.stats = new TransportStats();
        this.messageQueue = new LinkedBlockingQueue<>(options.getMaxMessageQueueSize());
        this.listeners = new CopyOnWriteArrayList<>();
        this.scheduler = Executors.newScheduledThreadPool(1);

        initializeWebSocket();
    }

    /**
     * Initializes the WebSocket client.
     */
    private void initializeWebSocket() throws ConnectionException {
        try {
            URI uri = new URI(url);

            wsClient = new WebSocketClient(uri) {
                @Override
                public void onOpen(ServerHandshake handshakedata) {
                    handleOpen();
                }

                @Override
                public void onMessage(String message) {
                    handleMessage(message);
                }

                @Override
                public void onClose(int code, String reason, boolean remote) {
                    handleClose(code, reason, remote);
                }

                @Override
                public void onError(Exception ex) {
                    handleError(ex);
                }
            };

            wsClient.setConnectionLostTimeout((int) options.getConnectTimeout().toSeconds());

        } catch (URISyntaxException e) {
            throw new ConnectionException("Invalid WebSocket URL: " + url, e);
        }
    }

    /**
     * Connects to the WebSocket server.
     *
     * @return a CompletableFuture that completes when connected
     */
    @NotNull
    public CompletableFuture<Boolean> connect() {
        return CompletableFuture.supplyAsync(() -> {
            try {
                logger.info("Connecting to {}", url);
                boolean result = wsClient.connectBlocking(
                    options.getConnectTimeout().toMillis(),
                    TimeUnit.MILLISECONDS
                );

                if (result) {
                    connected.set(true);
                    reconnectAttempts.set(0);
                    startHeartbeat();
                    processMessageQueue();
                    logger.info("Connected to {}", url);
                }

                return result;
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                logger.error("Connection interrupted", e);
                return false;
            }
        });
    }

    /**
     * Disconnects from the WebSocket server.
     *
     * @return a CompletableFuture that completes when disconnected
     */
    @NotNull
    public CompletableFuture<Boolean> disconnect() {
        return CompletableFuture.supplyAsync(() -> {
            try {
                logger.info("Disconnecting from {}", url);
                stopHeartbeat();
                connected.set(false);
                wsClient.closeBlocking();
                logger.info("Disconnected from {}", url);
                return true;
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                logger.error("Disconnection interrupted", e);
                return false;
            }
        });
    }

    /**
     * Sends an envelope to the server.
     *
     * @param envelope the envelope to send
     * @return a CompletableFuture that completes when sent
     */
    @NotNull
    public CompletableFuture<Boolean> send(@NotNull Envelope envelope) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                String json = envelope.serialize();

                if (!isConnected()) {
                    // Queue message if not connected
                    boolean queued = messageQueue.offer(envelope);
                    if (!queued) {
                        logger.warn("Message queue full, message dropped");
                        stats.recordError();
                        return false;
                    }
                    logger.debug("Message queued (not connected)");
                    return true;
                }

                wsClient.send(json);
                stats.recordMessageSent(json.length());
                logger.debug("Sent message: {} -> {}", envelope.getFrom(), envelope.getTo());
                return true;

            } catch (SerializationException e) {
                logger.error("Failed to serialize envelope", e);
                stats.recordError();
                emitError(e);
                return false;
            }
        });
    }

    /**
     * Checks if the client is connected.
     *
     * @return true if connected, false otherwise
     */
    public boolean isConnected() {
        return connected.get() && wsClient != null && wsClient.isOpen();
    }

    /**
     * Returns transport statistics.
     *
     * @return the transport statistics
     */
    @NotNull
    public TransportStats getStats() {
        return stats;
    }

    /**
     * Adds an event listener.
     *
     * @param listener the event listener
     */
    public void addEventListener(@NotNull TransportEventListener listener) {
        listeners.add(listener);
    }

    /**
     * Removes an event listener.
     *
     * @param listener the event listener
     */
    public void removeEventListener(@NotNull TransportEventListener listener) {
        listeners.remove(listener);
    }

    // ==================== Event Handlers ====================

    private void handleOpen() {
        logger.info("WebSocket connection opened");
        connected.set(true);
        reconnectAttempts.set(0);
        emitConnected();
    }

    private void handleMessage(String message) {
        try {
            stats.recordMessageReceived(message.length());
            Envelope envelope = Envelope.deserialize(message);
            emitMessage(envelope);
        } catch (SerializationException e) {
            logger.error("Failed to deserialize message", e);
            stats.recordError();
            emitError(e);
        }
    }

    private void handleClose(int code, String reason, boolean remote) {
        logger.info("WebSocket connection closed: code={}, reason={}, remote={}",
                   code, reason, remote);
        connected.set(false);
        stopHeartbeat();
        emitDisconnected(code, reason);

        if (options.isAutoReconnect() && remote) {
            attemptReconnect();
        }
    }

    private void handleError(Exception ex) {
        logger.error("WebSocket error", ex);
        stats.recordError();
        emitError(ex);
    }

    // ==================== Reconnection Logic ====================

    private void attemptReconnect() {
        int attempt = reconnectAttempts.incrementAndGet();

        if (options.getMaxReconnectAttempts() > 0 &&
            attempt > options.getMaxReconnectAttempts()) {
            logger.warn("Max reconnect attempts ({}) reached", options.getMaxReconnectAttempts());
            emitReconnectFailed();
            return;
        }

        long delay = calculateReconnectDelay(attempt);
        logger.info("Reconnecting in {}ms (attempt {})", delay, attempt);

        scheduler.schedule(() -> {
            try {
                initializeWebSocket();
                connect().get();
            } catch (Exception e) {
                logger.error("Reconnection failed", e);
                attemptReconnect();
            }
        }, delay, TimeUnit.MILLISECONDS);
    }

    private long calculateReconnectDelay(int attempt) {
        long delay = (long) (options.getReconnectDelay().toMillis() *
                            Math.pow(options.getReconnectBackoffMultiplier(), attempt - 1));
        return Math.min(delay, options.getMaxReconnectDelay().toMillis());
    }

    // ==================== Heartbeat ====================

    private void startHeartbeat() {
        stopHeartbeat();

        heartbeatTask = scheduler.scheduleAtFixedRate(() -> {
            if (isConnected()) {
                try {
                    wsClient.sendPing();
                    logger.trace("Heartbeat ping sent");
                } catch (Exception e) {
                    logger.warn("Failed to send heartbeat", e);
                }
            }
        },
        options.getHeartbeatInterval().toMillis(),
        options.getHeartbeatInterval().toMillis(),
        TimeUnit.MILLISECONDS);
    }

    private void stopHeartbeat() {
        if (heartbeatTask != null) {
            heartbeatTask.cancel(false);
            heartbeatTask = null;
        }
    }

    // ==================== Message Queue Processing ====================

    private void processMessageQueue() {
        CompletableFuture.runAsync(() -> {
            while (isConnected() && !messageQueue.isEmpty()) {
                try {
                    Envelope envelope = messageQueue.poll(100, TimeUnit.MILLISECONDS);
                    if (envelope != null) {
                        send(envelope).get();
                        logger.debug("Sent queued message");
                    }
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    break;
                } catch (Exception e) {
                    logger.error("Failed to send queued message", e);
                }
            }
        });
    }

    // ==================== Event Emission ====================

    private void emitConnected() {
        for (TransportEventListener listener : listeners) {
            try {
                listener.onConnected();
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitDisconnected(int code, String reason) {
        for (TransportEventListener listener : listeners) {
            try {
                listener.onDisconnected(code, reason);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitMessage(Envelope envelope) {
        for (TransportEventListener listener : listeners) {
            try {
                listener.onMessage(envelope);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitError(Exception error) {
        for (TransportEventListener listener : listeners) {
            try {
                listener.onError(error);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitReconnectFailed() {
        for (TransportEventListener listener : listeners) {
            try {
                listener.onReconnectFailed();
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    /**
     * Closes the client and releases resources.
     */
    @Override
    public void close() throws Exception {
        logger.info("Closing WebSocket client");
        disconnect().get(5, TimeUnit.SECONDS);
        scheduler.shutdown();
        if (!scheduler.awaitTermination(5, TimeUnit.SECONDS)) {
            scheduler.shutdownNow();
        }
    }
}

