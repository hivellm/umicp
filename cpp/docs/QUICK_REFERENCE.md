# UMICP C++ vs TypeScript - Quick Reference

**Last Updated:** October 10, 2025  
**Purpose:** Quick lookup for feature comparison and migration guidance

---

## Feature Status Quick Reference

### ✅ Complete in Both
- Core Envelope/Frame operations
- Matrix operations (C++ has SIMD advantage)
- Basic WebSocket transport
- JSON serialization
- Binary serialization (CBOR)
- Basic protocol operations

### 🚀 C++ Exclusive Features
- ⚡ **SIMD Matrix Operations** - 5-10x performance
- 🔒 **Advanced Security Suite** - Full crypto, 2-way auth
- 📊 **Schema Registry** - Message validation
- 🔄 **Multi-Transport Load Balancing** - Round-robin, least connections
- 📡 **Topic-based Routing** - Pub/sub pattern
- 🔁 **Automatic Failover** - Transport failover management

### ✅ TypeScript Exclusive Features
- 🌟 **Multiplexed Peer Architecture** - Simultaneous server/client
- 🎯 **EventEmitter Pattern** - Modern async/event-driven
- 🌐 **StreamableHTTP Transport** - Framework-agnostic HTTP
- 🔄 **Auto-Reconnect** - Exponential backoff, resilience
- 📢 **Advanced Broadcast** - Multiple broadcast strategies
- 🤝 **Request-Response Pattern** - RPC-style with timeout
- 📚 **Comprehensive Examples** - 6 detailed examples
- 📖 **Production Documentation** - Complete guides

---

## Code Comparison Cheat Sheet

### Creating a Basic Envelope

**TypeScript:**
```typescript
const envelope = new Envelope({
  from: 'client-001',
  to: 'server-001',
  operation: OperationType.DATA,
  messageId: 'msg-12345'
});
```

**C++ (Current):**
```cpp
Envelope envelope;
envelope.set_from("client-001");
envelope.set_to("server-001");
envelope.set_operation(OperationType::DATA);
envelope.set_message_id("msg-12345");
```

---

### Creating a Multiplexed Peer

**TypeScript:**
```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-agent',
  server: {
    port: 20081,
    path: '/umicp'
  }
});

peer.on('message', async (envelope, peer) => {
  console.log(`Message from ${peer.id}`);
});

await peer.connectToPeer('ws://localhost:20082/umicp');
peer.broadcast(envelope);
```

**C++ (Proposed):**
```cpp
MultiplexedPeer peer({
    .peer_id = "my-agent",
    .server = {
        .port = 20081,
        .path = "/umicp"
    }
});

peer.on("message", [](const Envelope& envelope, const PeerConnection& peer) {
    std::cout << "Message from " << peer.id << std::endl;
});

peer.connect_to_peer("ws://localhost:20082/umicp");
peer.broadcast(envelope);
```

**C++ (Current - No equivalent):**
```cpp
// Not implemented yet
// Must use low-level Protocol API
```

---

### WebSocket Client with Auto-Reconnect

**TypeScript:**
```typescript
const client = new UMICPWebSocketClient({
  url: 'ws://localhost:8080/umicp',
  autoReconnect: true,
  maxReconnectAttempts: 10,
  reconnectDelay: 5000,
  
  onMessage: async (envelope) => {
    console.log('Received:', envelope.getFrom());
  },
  
  onDisconnect: () => {
    console.log('Disconnected, will auto-reconnect...');
  }
});

await client.connect();
```

**C++ (Proposed):**
```cpp
WebSocketClient client({
    .url = "ws://localhost:8080/umicp",
    .auto_reconnect = true,
    .max_reconnect_attempts = 10,
    .reconnect_delay = std::chrono::milliseconds(5000)
});

client.set_message_callback([](const Envelope& envelope) {
    std::cout << "Received: " << envelope.get_from() << std::endl;
});

client.set_disconnection_callback([]() {
    std::cout << "Disconnected, will auto-reconnect..." << std::endl;
});

client.connect();
```

**C++ (Current):**
```cpp
// No auto-reconnect support
WebSocketTransport transport("ws://localhost:8080/umicp");
transport.connect();
// Manual reconnection required
```

---

### HTTP Communication

**TypeScript:**
```typescript
// Server
const httpServer = new StreamableHTTPServer({
  onMessage: async (envelope, req, res) => {
    // Process message
    const response = new Envelope({
      from: 'server',
      to: envelope.getFrom(),
      operation: OperationType.ACK
    });
    res.writeHead(200);
    res.end(response.serialize());
  }
});

// Client
const httpClient = new StreamableHTTPClient({
  baseUrl: 'http://localhost:9080',
  path: '/umicp',
  timeout: 30000
});

const response = await httpClient.send(envelope);
```

**C++ (Proposed):**
```cpp
// Server
StreamableHTTPServer server({
    .on_message = [](const Envelope& envelope, const HTTPRequest& req, HTTPResponse& res) {
        // Process message
        Envelope response;
        response.set_from("server");
        response.set_to(envelope.get_from());
        response.set_operation(OperationType::ACK);
        
        res.set_status(200);
        res.write(response.serialize());
    }
});

// Client
StreamableHTTPClient client({
    .base_url = "http://localhost:9080",
    .path = "/umicp",
    .timeout = std::chrono::milliseconds(30000)
});

auto response = client.send(envelope);
```

**C++ (Current - No equivalent):**
```cpp
// Not implemented
// Must use low-level HTTP/2 transport (incomplete)
```

---

### Broadcasting to Multiple Peers

**TypeScript:**
```typescript
// Broadcast to all connected peers
const count = peer.broadcast(envelope);
console.log(`Sent to ${count} peers`);

// Broadcast to incoming peers only
const countIn = peer.broadcastToType(envelope, 'incoming');

// Broadcast to outgoing peers only
const countOut = peer.broadcastToType(envelope, 'outgoing');

// Broadcast except one peer
const countExcept = peer.broadcast(envelope, 'peer-id-to-skip');
```

**C++ (Proposed):**
```cpp
// Broadcast to all connected peers
size_t count = peer.broadcast(envelope);
std::cout << "Sent to " << count << " peers" << std::endl;

// Broadcast to incoming peers only
size_t count_in = peer.broadcast_to_type(envelope, ConnectionType::INCOMING);

// Broadcast to outgoing peers only
size_t count_out = peer.broadcast_to_type(envelope, ConnectionType::OUTGOING);

// Broadcast except one peer
size_t count_except = peer.broadcast(envelope, "peer-id-to-skip");
```

**C++ (Current - No equivalent):**
```cpp
// Not implemented
// Must manually iterate and send to each transport
```

---

### Request-Response Pattern

**TypeScript:**
```typescript
try {
  const response = await peer.sendAndWait(
    'peer-id',
    requestEnvelope,
    5000 // timeout ms
  );
  console.log('Response:', response.getCapabilities());
} catch (error) {
  console.error('Request timeout or error:', error);
}
```

**C++ (Proposed):**
```cpp
try {
    auto response = peer.send_and_wait(
        "peer-id",
        request_envelope,
        std::chrono::milliseconds(5000)
    );
    
    if (response.has_value()) {
        std::cout << "Response received" << std::endl;
    }
} catch (const std::exception& e) {
    std::cerr << "Request timeout or error: " << e.what() << std::endl;
}
```

**C++ (Current - No equivalent):**
```cpp
// Not implemented
// Must manually track correlation IDs and implement timeout
```

---

### Event Handling

**TypeScript:**
```typescript
peer.on('message', async (envelope, peer) => {
  // Handle message
});

peer.on('peer:connect', (peer) => {
  console.log(`New peer: ${peer.id}`);
});

peer.on('peer:disconnect', (peer) => {
  console.log(`Peer left: ${peer.id}`);
});

peer.on('error', (error, peer) => {
  console.error('Error:', error);
});

peer.once('ready', () => {
  console.log('Peer is ready!');
});
```

**C++ (Proposed):**
```cpp
peer.on("message", [](const Event& event) {
    auto envelope = event.get<Envelope>("envelope");
    auto peer = event.get<PeerConnection>("peer");
    // Handle message
});

peer.on("peer:connect", [](const Event& event) {
    auto peer = event.get<PeerConnection>("peer");
    std::cout << "New peer: " << peer.id << std::endl;
});

peer.on("peer:disconnect", [](const Event& event) {
    auto peer = event.get<PeerConnection>("peer");
    std::cout << "Peer left: " << peer.id << std::endl;
});

peer.on("error", [](const Event& event) {
    auto error = event.get<std::string>("error");
    std::cerr << "Error: " << error << std::endl;
});

peer.once("ready", [](const Event& event) {
    std::cout << "Peer is ready!" << std::endl;
});
```

**C++ (Current - No equivalent):**
```cpp
// Not implemented
// Must use callbacks directly
transport->set_message_callback([](const ByteBuffer& data) {
    // Handle message
});
```

---

## Performance Comparison

| Operation | C++ | TypeScript | Winner |
|-----------|-----|------------|--------|
| Envelope creation | ~5-10 μs | ~50-100 μs | 🏆 C++ (10x) |
| Serialization | ~20-50 μs | ~100-200 μs | 🏆 C++ (5x) |
| Matrix multiply | ~10-50 μs | ~100-500 μs | 🏆 C++ (10x) |
| Message throughput | >50K/s | >10K/s | 🏆 C++ (5x) |
| Memory footprint | ~10-50 MB | ~50-200 MB | 🏆 C++ (5x) |
| Developer experience | 🟡 Good | ✅ Excellent | 🏆 TypeScript |
| Documentation | 🟡 Basic | ✅ Comprehensive | 🏆 TypeScript |
| Examples | 🟡 Few | ✅ Many | 🏆 TypeScript |

---

## Feature Availability Matrix

| Feature | C++ Current | C++ Planned | TypeScript | Priority |
|---------|-------------|-------------|------------|----------|
| **Basic Protocol** | ✅ | ✅ | ✅ | ✅ Done |
| **Matrix Ops** | ✅ | ✅ | ✅ | ✅ Done |
| **Event System** | ❌ | 📅 Phase 1 | ✅ | 🔴 Critical |
| **Multiplexed Peer** | ❌ | 📅 Phase 2 | ✅ | 🔴 Critical |
| **Advanced WebSocket** | ⚠️ | 📅 Phase 3 | ✅ | 🟠 High |
| **StreamableHTTP** | ❌ | 📅 Phase 4 | ✅ | 🟠 High |
| **Broadcast Patterns** | ❌ | 📅 Phase 5 | ✅ | 🟡 Medium |
| **Request-Response** | ❌ | 📅 Phase 6 | ✅ | 🟡 Medium |
| **Auto-Reconnect** | ❌ | 📅 Phase 7 | ✅ | 🟡 Medium |
| **Enhanced Stats** | ⚠️ | 📅 Phase 8 | ✅ | 🟢 Low |
| **Examples** | ⚠️ | 📅 Phase 9 | ✅ | 🟢 Low |
| **Documentation** | ⚠️ | 📅 Phase 10 | ✅ | 🔴 Critical |

---

## Migration Path

### For TypeScript Users Moving to C++

**✅ Available Now:**
- Core envelope/frame operations
- Matrix operations (with performance boost!)
- Basic WebSocket transport
- Serialization (JSON, CBOR)
- Advanced security features (bonus!)
- Multi-transport with load balancing (bonus!)

**⏳ Coming Soon (Phases 1-3):**
- Event-driven architecture
- Multiplexed peer system
- Auto-reconnect logic
- StreamableHTTP transport

**📅 Planned (Phases 4-7):**
- Advanced broadcast patterns
- Request-response pattern
- Comprehensive examples

### For C++ Users Getting TypeScript Features

**🎯 Short Term (0-3 months):**
1. Event system implementation
2. Multiplexed peer architecture
3. Production-grade WebSocket

**🎯 Medium Term (3-6 months):**
1. StreamableHTTP transport
2. Auto-reconnect and resilience
3. Advanced broadcast patterns
4. Request-response pattern

**🎯 Long Term (6+ months):**
1. Complete example suite
2. Comprehensive documentation
3. Production readiness
4. Performance optimization

---

## When to Use Which?

### Use C++ When:
- ⚡ Performance is critical (5-10x faster)
- 💾 Memory efficiency matters (5x smaller)
- 🔒 Advanced security needed
- 📊 Schema validation required
- 🔄 Multi-transport load balancing needed
- 📈 High throughput required (>50K msg/s)

### Use TypeScript When:
- 🚀 Rapid development needed
- 🌟 Multiplexed P2P required (for now)
- 🎯 Event-driven architecture preferred
- 🌐 Framework-agnostic HTTP needed
- 📚 Excellent documentation valued
- 👨‍💻 Better developer experience desired

### Use Both When:
- 🏆 Best performance + best features
- 🔄 TypeScript for orchestration, C++ for processing
- 🌍 Multi-language environment
- 🎭 Different teams with different preferences

---

## Timeline Summary

| Phase | Feature | Weeks | Status |
|-------|---------|-------|--------|
| **1** | Event System | 2-3 | 📅 Planned |
| **2** | Multiplexed Peer | 3-4 | 📅 Planned |
| **3** | Advanced WebSocket | 2-3 | 📅 Planned |
| **4** | StreamableHTTP | 2-3 | 📅 Planned |
| **5** | Broadcast Patterns | 1-2 | 📅 Planned |
| **6** | Request-Response | 1-2 | 📅 Planned |
| **7** | Connection Mgmt | 1-2 | 📅 Planned |
| **8** | Enhanced Stats | 1 | 📅 Planned |
| **9** | Examples | 1-2 | 📅 Planned |
| **10** | Testing/Docs | 3-4 | 📅 Planned |
| | **TOTAL** | **18-25** | **~6 months** |

---

## Quick Links

- **Full Roadmap:** [IMPLEMENTATION_ROADMAP.md](./IMPLEMENTATION_ROADMAP.md)
- **Detailed Comparison:** [FEATURE_COMPARISON.md](./FEATURE_COMPARISON.md)
- **TypeScript Docs:** `../bindings/typescript/docs/`
- **TypeScript Examples:** `../bindings/typescript/examples/`
- **C++ Tests:** `../tests/`

---

## FAQ

### Q: Will the new C++ API break existing code?
**A:** No! The new high-level API will coexist with the current low-level API. Existing code continues to work.

### Q: How long until feature parity?
**A:** Estimated 18-25 weeks (~6 months) for complete parity with all TypeScript features.

### Q: Will C++ maintain performance advantages?
**A:** Yes! The high-level API will be built on top of the optimized core, maintaining 5-10x performance advantages.

### Q: Can I use TypeScript features now?
**A:** Yes! Use the TypeScript bindings for multiplexed peer, event-driven, and HTTP features today.

### Q: Is the TypeScript binding production-ready?
**A:** Yes! TypeScript has comprehensive tests (163 tests), documentation, and production approval.

### Q: When will C++ be production-ready with new features?
**A:** After Phase 10 completion (~6 months), with comprehensive testing and documentation.

---

**Last Updated:** October 10, 2025  
**Document Version:** 1.0  
**See Also:** [IMPLEMENTATION_ROADMAP.md](./IMPLEMENTATION_ROADMAP.md) | [FEATURE_COMPARISON.md](./FEATURE_COMPARISON.md)

