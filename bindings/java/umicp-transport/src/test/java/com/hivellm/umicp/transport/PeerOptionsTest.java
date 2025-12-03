package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;

import java.time.Duration;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for PeerOptions.
 */
class PeerOptionsTest {

    @Test
    void testBuilder_withDefaults() {
        PeerOptions options = PeerOptions.builder("peer-1").build();

        assertEquals("peer-1", options.getPeerId());
        assertTrue(options.isAutoProtocol());
        assertEquals(Duration.ofSeconds(10), options.getHandshakeTimeout());
        assertNotNull(options.getServerOptions());
        assertNotNull(options.getClientOptions());
    }

    @Test
    void testBuilder_withCustomAutoProtocol() {
        PeerOptions options = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();

        assertEquals("peer-1", options.getPeerId());
        assertFalse(options.isAutoProtocol());
    }

    @Test
    void testBuilder_withCustomHandshakeTimeout() {
        Duration timeout = Duration.ofSeconds(30);

        PeerOptions options = PeerOptions.builder("peer-1")
            .handshakeTimeout(timeout)
            .build();

        assertEquals(timeout, options.getHandshakeTimeout());
    }

    @Test
    void testBuilder_withCustomServerOptions() {
        ServerOptions serverOpts = ServerOptions.builder()
            .maxClients(100)
            .build();

        PeerOptions options = PeerOptions.builder("peer-1")
            .serverOptions(serverOpts)
            .build();

        assertEquals(serverOpts, options.getServerOptions());
    }

    @Test
    void testBuilder_withCustomClientOptions() {
        ClientOptions clientOpts = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        PeerOptions options = PeerOptions.builder("peer-1")
            .clientOptions(clientOpts)
            .build();

        assertEquals(clientOpts, options.getClientOptions());
    }

    @Test
    void testBuilder_withAllOptions() {
        ServerOptions serverOpts = ServerOptions.builder()
            .maxClients(50)
            .build();

        ClientOptions clientOpts = ClientOptions.builder()
            .autoReconnect(true)
            .build();

        Duration timeout = Duration.ofSeconds(15);

        PeerOptions options = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .handshakeTimeout(timeout)
            .serverOptions(serverOpts)
            .clientOptions(clientOpts)
            .build();

        assertEquals("peer-1", options.getPeerId());
        assertFalse(options.isAutoProtocol());
        assertEquals(timeout, options.getHandshakeTimeout());
        assertEquals(serverOpts, options.getServerOptions());
        assertEquals(clientOpts, options.getClientOptions());
    }

    @Test
    void testBuilder_fluent() {
        PeerOptions.Builder builder = PeerOptions.builder("peer-1");

        // Builder methods should return the builder
        assertSame(builder.getClass(), builder.autoProtocol(true).getClass());
        assertSame(builder.getClass(), builder.handshakeTimeout(Duration.ofSeconds(5)).getClass());
        assertSame(builder.getClass(), builder.serverOptions(null).getClass());
        assertSame(builder.getClass(), builder.clientOptions(null).getClass());
    }

    @Test
    void testEquals_sameOptions() {
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(true)
            .handshakeTimeout(Duration.ofSeconds(10))
            .build();

        PeerOptions options2 = PeerOptions.builder("peer-1")
            .autoProtocol(true)
            .handshakeTimeout(Duration.ofSeconds(10))
            .build();

        assertEquals(options1, options2);
    }

    @Test
    void testEquals_differentPeerId() {
        PeerOptions options1 = PeerOptions.builder("peer-1").build();
        PeerOptions options2 = PeerOptions.builder("peer-2").build();

        assertNotEquals(options1, options2);
    }

    @Test
    void testEquals_differentAutoProtocol() {
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(true)
            .build();

        PeerOptions options2 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();

        assertNotEquals(options1, options2);
    }

    @Test
    void testEquals_differentTimeout() {
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .handshakeTimeout(Duration.ofSeconds(10))
            .build();

        PeerOptions options2 = PeerOptions.builder("peer-1")
            .handshakeTimeout(Duration.ofSeconds(20))
            .build();

        assertNotEquals(options1, options2);
    }

    @Test
    void testEquals_withNull() {
        PeerOptions options = PeerOptions.builder("peer-1").build();
        assertNotEquals(null, options);
    }

    @Test
    void testEquals_withSelf() {
        PeerOptions options = PeerOptions.builder("peer-1").build();
        assertEquals(options, options);
    }

    @Test
    void testHashCode_sameOptions() {
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(true)
            .handshakeTimeout(Duration.ofSeconds(10))
            .build();

        PeerOptions options2 = PeerOptions.builder("peer-1")
            .autoProtocol(true)
            .handshakeTimeout(Duration.ofSeconds(10))
            .build();

        assertEquals(options1.hashCode(), options2.hashCode());
    }

    @Test
    void testBuilder_nullServerOptions_usesDefaults() {
        PeerOptions options = PeerOptions.builder("peer-1")
            .serverOptions(null)
            .build();

        assertNotNull(options.getServerOptions());
    }

    @Test
    void testBuilder_nullClientOptions_usesDefaults() {
        PeerOptions options = PeerOptions.builder("peer-1")
            .clientOptions(null)
            .build();

        assertNotNull(options.getClientOptions());
    }
}

