package com.hivellm.umicp.examples;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.core.UMICP;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.transport.http.HttpClientOptions;
import com.hivellm.umicp.transport.http.UMICPHttpClient;

import java.net.http.HttpClient;
import java.time.Duration;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;

/**
 * Example demonstrating UMICP HTTP/2 Client
 */
public class Http2Example {
    public static void main(String[] args) {
        System.out.println("=== UMICP HTTP/2 Client Example ===\n");

        // Create HTTP/2 client options
        System.out.println("1. Creating HTTP/2 client...");
        HttpClientOptions options = new HttpClientOptions()
                .setBaseUrl("http://localhost:8080")
                .setUseHttp2(true)
                .setConnectTimeout(Duration.ofSeconds(10))
                .setRequestTimeout(Duration.ofSeconds(30))
                .setFollowRedirects(true)
                .setMaxRetries(3);

        System.out.println("   Configuration:");
        System.out.println("   - Base URL: " + options.getBaseUrl());
        System.out.println("   - HTTP Version: HTTP/2");
        System.out.println("   - Connect timeout: " + options.getConnectTimeout().getSeconds() + "s");
        System.out.println("   - Request timeout: " + options.getRequestTimeout().getSeconds() + "s");
        System.out.println("   - Follow redirects: " + options.isFollowRedirects());
        System.out.println("   - Max retries: " + options.getMaxRetries());
        System.out.println();

        try (UMICPHttpClient client = new UMICPHttpClient(options)) {
            System.out.println("   ✓ HTTP/2 client created");
            System.out.println("   Protocol version: " + client.getHttpVersion());
            System.out.println();

            // Create an envelope
            System.out.println("2. Creating UMICP envelope...");
            Envelope envelope = UMICP.createEnvelope(
                UMICP.envelopeOptions()
                    .from("http-client")
                    .to("http-server")
                    .operation(OperationType.DATA)
                    .messageId(UUID.randomUUID().toString())
                    .capability("content-type", "application/json")
                    .capability("priority", "high")
                    .build()
            );

            System.out.println("   ✓ Envelope created:");
            System.out.println("     - From: " + envelope.getFrom());
            System.out.println("     - To: " + envelope.getTo());
            System.out.println("     - Operation: " + envelope.getOperation());
            System.out.println("     - Message ID: " + envelope.getMessageId());
            System.out.println();

            // Example 3: HTTP/1.1 client for comparison
            System.out.println("3. Creating HTTP/1.1 client for comparison...");
            HttpClientOptions http1Options = new HttpClientOptions()
                    .setBaseUrl("http://localhost:8080")
                    .setUseHttp2(false);

            try (UMICPHttpClient http1Client = new UMICPHttpClient(http1Options)) {
                System.out.println("   ✓ HTTP/1.1 client created");
                System.out.println("   Protocol version: " + http1Client.getHttpVersion());
            }
            System.out.println();

            // Example 4: Configuration validation
            System.out.println("4. Testing configuration options...");
            HttpClientOptions testOptions = new HttpClientOptions();

            System.out.println("   Default HTTP version: " +
                (testOptions.isUseHttp2() ? "HTTP/2" : "HTTP/1.1"));

            testOptions.setUseHttp2(false);
            System.out.println("   Changed to: " +
                (testOptions.isUseHttp2() ? "HTTP/2" : "HTTP/1.1"));
            System.out.println();

            System.out.println("=== HTTP/2 Example Complete ===");
            System.out.println("\nNOTE: This example demonstrates the API structure.");
            System.out.println("For full functionality, send requests to a running HTTP server.");

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}

