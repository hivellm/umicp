/**
 * Transport layer implementations for UMICP.
 *
 * <p>This package contains WebSocket and HTTP/2 transport implementations:
 * <ul>
 *   <li>{@link com.hivellm.umicp.transport.UMICPWebSocketClient} - WebSocket client</li>
 *   <li>{@link com.hivellm.umicp.transport.UMICPWebSocketServer} - WebSocket server</li>
 *   <li>{@link com.hivellm.umicp.transport.UMICPWebSocketPeer} - Multiplexed peer (server + clients)</li>
 *   <li>{@link com.hivellm.umicp.transport.ClientOptions} - Client configuration</li>
 *   <li>{@link com.hivellm.umicp.transport.ServerOptions} - Server configuration</li>
 *   <li>{@link com.hivellm.umicp.transport.TransportStats} - Connection statistics</li>
 * </ul>
 *
 * <p>Example usage:
 * <pre>{@code
 * // Create WebSocket client
 * ClientOptions clientOpts = ClientOptions.builder()
 *     .autoReconnect(true)
 *     .build();
 *
 * UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080", clientOpts);
 * client.connect().get();
 *
 * // Create WebSocket server
 * ServerOptions serverOpts = ServerOptions.builder()
 *     .maxClients(1000)
 *     .build();
 *
 * UMICPWebSocketServer server = new UMICPWebSocketServer(8080, serverOpts);
 * server.start().get();
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
package com.hivellm.umicp.transport;

