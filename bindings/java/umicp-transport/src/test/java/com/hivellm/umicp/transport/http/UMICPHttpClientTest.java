package com.hivellm.umicp.transport.http;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.core.UMICP;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.ConnectionException;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;

import java.net.http.HttpClient;
import java.time.Duration;
import java.util.UUID;

/**
 * Unit tests for UMICPHttpClient
 */
class UMICPHttpClientTest {
    private HttpClientOptions options;
    private UMICPHttpClient client;

    @BeforeEach
    void setUp() {
        options = new HttpClientOptions()
                .setBaseUrl("http://localhost:8080")
                .setUseHttp2(true)
                .setConnectTimeout(Duration.ofSeconds(5))
                .setRequestTimeout(Duration.ofSeconds(10));

        client = new UMICPHttpClient(options);
    }

    @Test
    void testCreation() {
        assertNotNull(client);
        assertEquals(options, client.getOptions());
        assertEquals(HttpClient.Version.HTTP_2, client.getHttpVersion());
    }

    @Test
    void testCreationWithNullOptions() {
        assertThrows(IllegalArgumentException.class, () -> {
            new UMICPHttpClient(null);
        });
    }

    @Test
    void testDefaultOptions() {
        UMICPHttpClient defaultClient = new UMICPHttpClient();
        assertNotNull(defaultClient);
        assertNotNull(defaultClient.getOptions());
        assertEquals(HttpClient.Version.HTTP_2, defaultClient.getHttpVersion());
    }

    @Test
    void testHttp1Configuration() {
        HttpClientOptions http1Options = new HttpClientOptions()
                .setBaseUrl("http://localhost:8080")
                .setUseHttp2(false);

        UMICPHttpClient http1Client = new UMICPHttpClient(http1Options);
        assertEquals(HttpClient.Version.HTTP_1_1, http1Client.getHttpVersion());
    }

    @Test
    void testOptionsConfiguration() {
        HttpClientOptions customOptions = new HttpClientOptions()
                .setBaseUrl("http://test:9000")
                .setConnectTimeout(Duration.ofSeconds(15))
                .setRequestTimeout(Duration.ofSeconds(30))
                .setFollowRedirects(false)
                .setUseHttp2(true)
                .setMaxRetries(5);

        assertEquals("http://test:9000", customOptions.getBaseUrl());
        assertEquals(Duration.ofSeconds(15), customOptions.getConnectTimeout());
        assertEquals(Duration.ofSeconds(30), customOptions.getRequestTimeout());
        assertFalse(customOptions.isFollowRedirects());
        assertTrue(customOptions.isUseHttp2());
        assertEquals(5, customOptions.getMaxRetries());
    }

    @Test
    void testInvalidMaxRetries() {
        assertThrows(IllegalArgumentException.class, () -> {
            new HttpClientOptions().setMaxRetries(-1);
        });
    }

    @Test
    void testClose() {
        // Should not throw
        assertDoesNotThrow(() -> client.close());
    }

    // Note: Full integration tests would require a real HTTP server
    // These tests validate the API structure and configuration
}

