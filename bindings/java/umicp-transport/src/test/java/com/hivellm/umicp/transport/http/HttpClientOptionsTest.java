package com.hivellm.umicp.transport.http;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

import java.time.Duration;

/**
 * Unit tests for HttpClientOptions
 */
class HttpClientOptionsTest {

    @Test
    void testDefaultConstructor() {
        HttpClientOptions options = new HttpClientOptions();

        assertNull(options.getBaseUrl());
        assertEquals(Duration.ofSeconds(10), options.getConnectTimeout());
        assertEquals(Duration.ofSeconds(30), options.getRequestTimeout());
        assertTrue(options.isFollowRedirects());
        assertTrue(options.isUseHttp2());
        assertEquals(3, options.getMaxRetries());
    }

    @Test
    void testFluentAPI() {
        HttpClientOptions options = new HttpClientOptions()
                .setBaseUrl("http://test:9000")
                .setConnectTimeout(Duration.ofSeconds(5))
                .setRequestTimeout(Duration.ofSeconds(20))
                .setFollowRedirects(false)
                .setUseHttp2(false)
                .setMaxRetries(5);

        assertEquals("http://test:9000", options.getBaseUrl());
        assertEquals(Duration.ofSeconds(5), options.getConnectTimeout());
        assertEquals(Duration.ofSeconds(20), options.getRequestTimeout());
        assertFalse(options.isFollowRedirects());
        assertFalse(options.isUseHttp2());
        assertEquals(5, options.getMaxRetries());
    }

    @Test
    void testSetBaseUrl() {
        HttpClientOptions options = new HttpClientOptions();
        options.setBaseUrl("http://localhost:8080");

        assertEquals("http://localhost:8080", options.getBaseUrl());
    }

    @Test
    void testSetConnectTimeout() {
        HttpClientOptions options = new HttpClientOptions();
        options.setConnectTimeout(Duration.ofSeconds(15));

        assertEquals(Duration.ofSeconds(15), options.getConnectTimeout());
    }

    @Test
    void testSetRequestTimeout() {
        HttpClientOptions options = new HttpClientOptions();
        options.setRequestTimeout(Duration.ofSeconds(60));

        assertEquals(Duration.ofSeconds(60), options.getRequestTimeout());
    }

    @Test
    void testSetFollowRedirects() {
        HttpClientOptions options = new HttpClientOptions();
        assertTrue(options.isFollowRedirects());

        options.setFollowRedirects(false);
        assertFalse(options.isFollowRedirects());
    }

    @Test
    void testSetUseHttp2() {
        HttpClientOptions options = new HttpClientOptions();
        assertTrue(options.isUseHttp2());

        options.setUseHttp2(false);
        assertFalse(options.isUseHttp2());
    }

    @Test
    void testSetMaxRetries() {
        HttpClientOptions options = new HttpClientOptions();
        options.setMaxRetries(10);

        assertEquals(10, options.getMaxRetries());
    }

    @Test
    void testSetMaxRetriesZero() {
        HttpClientOptions options = new HttpClientOptions();
        options.setMaxRetries(0);

        assertEquals(0, options.getMaxRetries());
    }

    @Test
    void testSetMaxRetriesNegative() {
        HttpClientOptions options = new HttpClientOptions();

        assertThrows(IllegalArgumentException.class, () -> {
            options.setMaxRetries(-1);
        });
    }
}

