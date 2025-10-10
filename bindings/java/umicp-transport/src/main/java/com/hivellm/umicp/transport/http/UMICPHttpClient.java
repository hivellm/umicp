package com.hivellm.umicp.transport.http;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.core.UMICP;
import com.hivellm.umicp.types.ConnectionException;
import com.hivellm.umicp.types.SerializationException;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.concurrent.CompletableFuture;

/**
 * HTTP/2 client for UMICP
 * Uses Java 11+ HttpClient with HTTP/2 support
 */
public class UMICPHttpClient implements AutoCloseable {
    private final HttpClient httpClient;
    private final HttpClientOptions options;
    private final HttpClient.Version httpVersion;

    /**
     * Create HTTP client with default options
     */
    public UMICPHttpClient() {
        this(new HttpClientOptions());
    }

    /**
     * Create HTTP client with custom options
     *
     * @param options Client configuration
     */
    public UMICPHttpClient(HttpClientOptions options) {
        if (options == null) {
            throw new IllegalArgumentException("options cannot be null");
        }

        this.options = options;
        this.httpVersion = options.isUseHttp2() ?
            HttpClient.Version.HTTP_2 : HttpClient.Version.HTTP_1_1;

        this.httpClient = HttpClient.newBuilder()
                .version(httpVersion)
                .connectTimeout(options.getConnectTimeout())
                .followRedirects(options.isFollowRedirects() ?
                    HttpClient.Redirect.NORMAL : HttpClient.Redirect.NEVER)
                .build();
    }

    /**
     * Send envelope via HTTP POST
     *
     * @param endpoint Endpoint path (e.g., "/umicp/message")
     * @param envelope Envelope to send
     * @return Response body as string
     * @throws ConnectionException if request fails
     */
    public String sendEnvelope(String endpoint, Envelope envelope) throws ConnectionException {
        try {
            String json = UMICP.serialize(envelope);
            String url = buildUrl(endpoint);

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .timeout(options.getRequestTimeout())
                    .header("Content-Type", "application/json")
                    .header("User-Agent", "UMICP-Java/0.1.2")
                    .POST(HttpRequest.BodyPublishers.ofString(json))
                    .build();

            HttpResponse<String> response = httpClient.send(
                request,
                HttpResponse.BodyHandlers.ofString()
            );

            if (response.statusCode() >= 400) {
                throw new ConnectionException("HTTP error: " + response.statusCode());
            }

            return response.body();

        } catch (SerializationException e) {
            throw new ConnectionException("Failed to serialize envelope", e);
        } catch (Exception e) {
            throw new ConnectionException("HTTP request failed: " + e.getMessage(), e);
        }
    }

    /**
     * Send envelope asynchronously
     *
     * @param endpoint Endpoint path
     * @param envelope Envelope to send
     * @return CompletableFuture with response
     */
    public CompletableFuture<String> sendEnvelopeAsync(String endpoint, Envelope envelope) {
        try {
            String json = UMICP.serialize(envelope);
            String url = buildUrl(endpoint);

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .timeout(options.getRequestTimeout())
                    .header("Content-Type", "application/json")
                    .header("User-Agent", "UMICP-Java/0.1.2")
                    .POST(HttpRequest.BodyPublishers.ofString(json))
                    .build();

            return httpClient.sendAsync(request, HttpResponse.BodyHandlers.ofString())
                    .thenApply(HttpResponse::body);

        } catch (SerializationException e) {
            CompletableFuture<String> future = new CompletableFuture<>();
            future.completeExceptionally(e);
            return future;
        }
    }

    /**
     * Send POST request with raw data
     *
     * @param endpoint Endpoint path
     * @param data Request body
     * @return Response body
     * @throws ConnectionException if request fails
     */
    public String post(String endpoint, String data) throws ConnectionException {
        try {
            String url = buildUrl(endpoint);

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .timeout(options.getRequestTimeout())
                    .header("Content-Type", "application/json")
                    .POST(HttpRequest.BodyPublishers.ofString(data))
                    .build();

            HttpResponse<String> response = httpClient.send(
                request,
                HttpResponse.BodyHandlers.ofString()
            );

            if (response.statusCode() >= 400) {
                throw new ConnectionException("HTTP error: " + response.statusCode());
            }

            return response.body();

        } catch (Exception e) {
            throw new ConnectionException("HTTP request failed: " + e.getMessage(), e);
        }
    }

    /**
     * Send GET request
     *
     * @param endpoint Endpoint path
     * @return Response body
     * @throws ConnectionException if request fails
     */
    public String get(String endpoint) throws ConnectionException {
        try {
            String url = buildUrl(endpoint);

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .timeout(options.getRequestTimeout())
                    .header("User-Agent", "UMICP-Java/0.1.2")
                    .GET()
                    .build();

            HttpResponse<String> response = httpClient.send(
                request,
                HttpResponse.BodyHandlers.ofString()
            );

            if (response.statusCode() >= 400) {
                throw new ConnectionException("HTTP error: " + response.statusCode());
            }

            return response.body();

        } catch (Exception e) {
            throw new ConnectionException("HTTP request failed: " + e.getMessage(), e);
        }
    }

    /**
     * Get HTTP protocol version being used
     *
     * @return HTTP version (HTTP_1_1 or HTTP_2)
     */
    public HttpClient.Version getHttpVersion() {
        return httpVersion;
    }

    /**
     * Get client options
     *
     * @return Client configuration
     */
    public HttpClientOptions getOptions() {
        return options;
    }

    /**
     * Build full URL from endpoint
     *
     * @param endpoint Endpoint path
     * @return Full URL
     */
    private String buildUrl(String endpoint) {
        if (options.getBaseUrl() == null) {
            throw new IllegalStateException("baseUrl not configured");
        }

        String base = options.getBaseUrl();
        if (base.endsWith("/") && endpoint.startsWith("/")) {
            return base + endpoint.substring(1);
        } else if (!base.endsWith("/") && !endpoint.startsWith("/")) {
            return base + "/" + endpoint;
        } else {
            return base + endpoint;
        }
    }

    @Override
    public void close() {
        // HttpClient doesn't need explicit closing in Java 11+
    }
}

