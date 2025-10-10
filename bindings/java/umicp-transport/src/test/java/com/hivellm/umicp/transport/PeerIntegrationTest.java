package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.OperationType;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Integration tests for UMICPWebSocketPeer.
 *
 * <p>These tests verify peer-to-peer communication scenarios.
 */
@Timeout(15)
class PeerIntegrationTest {

    private final List<UMICPWebSocketPeer> peers = new ArrayList<>();

    @AfterEach
    void tearDown() throws Exception {
        for (UMICPWebSocketPeer peer : peers) {
            try {
                peer.close();
            } catch (Exception e) {
                // Ignore
            }
        }
        peers.clear();

        // Give time for ports to be released
        Thread.sleep(300);
    }

    @Test
    void testPeerStartsServer() throws Exception {
        PeerOptions options = PeerOptions.builder("peer-1").build();
        UMICPWebSocketPeer peer = new UMICPWebSocketPeer(options);
        peers.add(peer);

        CountDownLatch latch = new CountDownLatch(1);

        peer.addEventListener(new PeerEventListener() {
            @Override
            public void onServerReady() {
                latch.countDown();
            }
        });

        peer.startServer(20081).get();

        assertTrue(latch.await(3, TimeUnit.SECONDS), "Server should start");
    }

    @Test
    void testTwoPeersConnect() throws Exception {
        CountDownLatch peer1Latch = new CountDownLatch(1);
        CountDownLatch peer2Latch = new CountDownLatch(1);

        // Create Peer 1
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerConnect(PeerConnection connection) {
                peer1Latch.countDown();
            }
        });

        peer1.startServer(20082).get();
        Thread.sleep(200);

        // Create Peer 2
        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        peer2.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerConnect(PeerConnection connection) {
                peer2Latch.countDown();
            }
        });

        peer2.startServer(20083).get();
        Thread.sleep(200);

        // Connect Peer 2 to Peer 1
        peer2.connectToPeer("ws://localhost:20082/umicp", new HashMap<>()).get();

        assertTrue(peer1Latch.await(3, TimeUnit.SECONDS), "Peer 1 should detect connection");
        assertTrue(peer2Latch.await(3, TimeUnit.SECONDS), "Peer 2 should detect connection");
    }

    @Test
    void testPeerSendsMessage() throws Exception {
        CountDownLatch messageLatch = new CountDownLatch(1);
        AtomicReference<Envelope> receivedEnvelope = new AtomicReference<>();

        // Create Peer 1
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.addEventListener(new PeerEventListener() {
            @Override
            public void onMessage(Envelope envelope, PeerConnection connection) {
                receivedEnvelope.set(envelope);
                messageLatch.countDown();
            }
        });

        peer1.startServer(20084).get();
        Thread.sleep(200);

        // Create Peer 2
        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        String connectionId = peer2.connectToPeer("ws://localhost:20084/umicp", new HashMap<>()).get();
        Thread.sleep(200);

        // Send message from Peer 2 to Peer 1
        Envelope message = new Envelope()
            .setFrom("peer-2")
            .setTo("peer-1")
            .setOperation(OperationType.DATA)
            .setMessageId("msg-" + UUID.randomUUID());

        peer2.sendToPeer(connectionId, message).get();

        assertTrue(messageLatch.await(3, TimeUnit.SECONDS), "Peer 1 should receive message");
        assertNotNull(receivedEnvelope.get());
        assertEquals(message.getMessageId(), receivedEnvelope.get().getMessageId());
    }

    @Test
    void testPeerBroadcast() throws Exception {
        CountDownLatch peer2Latch = new CountDownLatch(1);
        CountDownLatch peer3Latch = new CountDownLatch(1);

        // Create Peer 1 (broadcaster)
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.startServer(20085).get();
        Thread.sleep(200);

        // Create Peer 2
        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        peer2.addEventListener(new PeerEventListener() {
            @Override
            public void onMessage(Envelope envelope, PeerConnection connection) {
                peer2Latch.countDown();
            }
        });

        peer2.connectToPeer("ws://localhost:20085/umicp", new HashMap<>()).get();
        Thread.sleep(200);

        // Create Peer 3
        PeerOptions options3 = PeerOptions.builder("peer-3")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer3 = new UMICPWebSocketPeer(options3);
        peers.add(peer3);

        peer3.addEventListener(new PeerEventListener() {
            @Override
            public void onMessage(Envelope envelope, PeerConnection connection) {
                peer3Latch.countDown();
            }
        });

        peer3.connectToPeer("ws://localhost:20085/umicp", new HashMap<>()).get();
        Thread.sleep(200);

        // Broadcast from Peer 1
        Envelope broadcast = new Envelope()
            .setFrom("peer-1")
            .setTo("*")
            .setOperation(OperationType.DATA)
            .setMessageId("broadcast-" + UUID.randomUUID());

        int count = peer1.broadcast(broadcast).get();
        assertEquals(2, count);

        assertTrue(peer2Latch.await(3, TimeUnit.SECONDS), "Peer 2 should receive broadcast");
        assertTrue(peer3Latch.await(3, TimeUnit.SECONDS), "Peer 3 should receive broadcast");
    }

    @Test
    void testAutoHandshake() throws Exception {
        CountDownLatch peer1ReadyLatch = new CountDownLatch(1);
        CountDownLatch peer2ReadyLatch = new CountDownLatch(1);
        AtomicReference<PeerInfo> peer1Info = new AtomicReference<>();
        AtomicReference<PeerInfo> peer2Info = new AtomicReference<>();

        // Create Peer 1 with auto-protocol
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(true)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerReady(PeerConnection connection, PeerInfo info) {
                peer1Info.set(info);
                peer1ReadyLatch.countDown();
            }
        });

        peer1.startServer(20086).get();
        Thread.sleep(200);

        // Create Peer 2 with auto-protocol
        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(true)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        peer2.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerReady(PeerConnection connection, PeerInfo info) {
                peer2Info.set(info);
                peer2ReadyLatch.countDown();
            }
        });

        // Connect with auto-handshake
        peer2.connectToPeer("ws://localhost:20086/umicp", new HashMap<>()).get();

        assertTrue(peer1ReadyLatch.await(5, TimeUnit.SECONDS), "Peer 1 should complete handshake");
        assertTrue(peer2ReadyLatch.await(5, TimeUnit.SECONDS), "Peer 2 should complete handshake");

        assertNotNull(peer1Info.get());
        assertNotNull(peer2Info.get());
        assertEquals("peer-2", peer1Info.get().getPeerId());
        assertEquals("peer-1", peer2Info.get().getPeerId());
    }

    @Test
    void testPeerStatistics() throws Exception {
        // Create Peer 1
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.startServer(20087).get();
        Thread.sleep(200);

        // Create Peer 2
        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        peer2.startServer(20088).get();
        Thread.sleep(200);

        // Connect Peer 1 to Peer 2 (outgoing)
        peer1.connectToPeer("ws://localhost:20088/umicp", new HashMap<>()).get();
        Thread.sleep(200);

        // Connect Peer 2 to Peer 1 (outgoing)
        peer2.connectToPeer("ws://localhost:20087/umicp", new HashMap<>()).get();
        Thread.sleep(200);

        // Check Peer 1 stats: 1 outgoing, 1 incoming
        PeerStats stats1 = peer1.getStats();
        assertEquals(2, stats1.getTotalPeers());
        assertEquals(1, stats1.getIncomingConnections());
        assertEquals(1, stats1.getOutgoingConnections());

        // Check Peer 2 stats: 1 outgoing, 1 incoming
        PeerStats stats2 = peer2.getStats();
        assertEquals(2, stats2.getTotalPeers());
        assertEquals(1, stats2.getIncomingConnections());
        assertEquals(1, stats2.getOutgoingConnections());
    }

    @Test
    void testDisconnectPeer() throws Exception {
        CountDownLatch connectLatch = new CountDownLatch(1);
        CountDownLatch disconnectLatch = new CountDownLatch(1);

        // Create Peer 1
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerConnect(PeerConnection connection) {
                connectLatch.countDown();
            }

            @Override
            public void onPeerDisconnect(PeerConnection connection) {
                disconnectLatch.countDown();
            }
        });

        peer1.startServer(20089).get();
        Thread.sleep(200);

        // Create Peer 2
        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        String connectionId = peer2.connectToPeer("ws://localhost:20089/umicp", new HashMap<>()).get();

        assertTrue(connectLatch.await(3, TimeUnit.SECONDS));

        // Disconnect
        peer2.disconnectPeer(connectionId).get();

        assertTrue(disconnectLatch.await(3, TimeUnit.SECONDS), "Peer 1 should detect disconnection");
    }

    @Test
    void testRingTopology() throws Exception {
        CountDownLatch peer1Latch = new CountDownLatch(2);
        CountDownLatch peer2Latch = new CountDownLatch(2);
        CountDownLatch peer3Latch = new CountDownLatch(2);

        AtomicInteger peer1Messages = new AtomicInteger(0);
        AtomicInteger peer2Messages = new AtomicInteger(0);
        AtomicInteger peer3Messages = new AtomicInteger(0);

        // Create 3 peers
        PeerOptions options1 = PeerOptions.builder("peer-1")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer1 = new UMICPWebSocketPeer(options1);
        peers.add(peer1);

        peer1.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerConnect(PeerConnection connection) {
                peer1Latch.countDown();
            }

            @Override
            public void onData(Envelope envelope, PeerConnection connection) {
                peer1Messages.incrementAndGet();
            }
        });

        PeerOptions options2 = PeerOptions.builder("peer-2")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer2 = new UMICPWebSocketPeer(options2);
        peers.add(peer2);

        peer2.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerConnect(PeerConnection connection) {
                peer2Latch.countDown();
            }

            @Override
            public void onData(Envelope envelope, PeerConnection connection) {
                peer2Messages.incrementAndGet();
            }
        });

        PeerOptions options3 = PeerOptions.builder("peer-3")
            .autoProtocol(false)
            .build();
        UMICPWebSocketPeer peer3 = new UMICPWebSocketPeer(options3);
        peers.add(peer3);

        peer3.addEventListener(new PeerEventListener() {
            @Override
            public void onPeerConnect(PeerConnection connection) {
                peer3Latch.countDown();
            }

            @Override
            public void onData(Envelope envelope, PeerConnection connection) {
                peer3Messages.incrementAndGet();
            }
        });

        // Start all servers
        peer1.startServer(20091).get();
        Thread.sleep(100);
        peer2.startServer(20092).get();
        Thread.sleep(100);
        peer3.startServer(20093).get();
        Thread.sleep(200);

        // Connect in ring: 1 → 2 → 3 → 1
        peer1.connectToPeer("ws://localhost:20092/umicp", new HashMap<>()).get();
        Thread.sleep(100);
        peer2.connectToPeer("ws://localhost:20093/umicp", new HashMap<>()).get();
        Thread.sleep(100);
        peer3.connectToPeer("ws://localhost:20091/umicp", new HashMap<>()).get();
        Thread.sleep(200);

        assertTrue(peer1Latch.await(3, TimeUnit.SECONDS));
        assertTrue(peer2Latch.await(3, TimeUnit.SECONDS));
        assertTrue(peer3Latch.await(3, TimeUnit.SECONDS));

        // Each peer should have 2 connections
        assertEquals(2, peer1.getPeers().size());
        assertEquals(2, peer2.getPeers().size());
        assertEquals(2, peer3.getPeers().size());
    }
}

