# UMICP C++ Implementation Roadmap

**Last Updated:** October 10, 2025  
**Status:** Planning Phase  
**Objective:** Achieve feature parity between C++ and TypeScript implementations

---

## Executive Summary

The TypeScript binding has evolved significantly beyond the C++ core implementation, introducing advanced high-level features focused on peer-to-peer networking, multiplexed connections, and framework-agnostic transports. This roadmap outlines the path to bring these features to the C++ implementation.

### Current Status

| Component | C++ Status | TypeScript Status | Gap |
|-----------|-----------|-------------------|-----|
| **Core Protocol** | ✅ Complete | ✅ Complete | None |
| **Envelope/Frame** | ✅ Complete | ✅ Complete | None |
| **Matrix Operations** | ✅ Complete (SIMD) | ✅ Complete | None |
| **Security** | ✅ Advanced | ✅ Basic | C++ ahead |
| **WebSocket Transport** | ⚠️ Basic | ✅ Advanced | TypeScript ahead |
| **HTTP Transport** | ⚠️ Basic | ✅ StreamableHTTP | TypeScript ahead |
| **Multiplexed Peer** | ❌ Missing | ✅ Complete | TypeScript ahead |
| **Event-Driven Pattern** | ❌ Missing | ✅ Complete | TypeScript ahead |
| **Broadcast Patterns** | ❌ Missing | ✅ Complete | TypeScript ahead |
| **Auto-Reconnect** | ❌ Missing | ✅ Complete | TypeScript ahead |
| **Peer Management** | ❌ Missing | ✅ Complete | TypeScript ahead |
| **Request-Response** | ❌ Missing | ✅ Complete | TypeScript ahead |
| **Schema Registry** | ✅ Complete | ❌ Missing | C++ ahead |
| **Multi-Transport LB** | ✅ Complete | ❌ Missing | C++ ahead |
| **Topic Routing** | ✅ Complete | ❌ Missing | C++ ahead |

---

## Phase 1: Event-Driven Architecture (Priority: Critical)

**Goal:** Implement event-driven patterns to enable asynchronous communication.

### 1.1 Event System Core
**Status:** Not Started  
**Estimated Effort:** 2-3 weeks

**Requirements:**
- Implement C++ event emitter/listener pattern
- Thread-safe event dispatching
- Support for multiple event types
- Lambda/function callback support

**Files to Create:**
```
include/event_emitter.h
src/event_emitter.cpp
tests/test_event_emitter.cpp
```

**API Design:**
```cpp
class EventEmitter {
public:
    using EventCallback = std::function<void(const Event&)>;
    
    // Register event handler
    size_t on(const std::string& event_name, EventCallback callback);
    
    // Register one-time event handler
    size_t once(const std::string& event_name, EventCallback callback);
    
    // Remove event handler
    void off(const std::string& event_name, size_t handler_id);
    
    // Emit event
    void emit(const std::string& event_name, const Event& event);
    
    // Remove all handlers for event
    void removeAllListeners(const std::string& event_name);
};
```

**TypeScript Reference:**
- `src/transports/websocket-transport.ts`: Lines 142-958 (EventEmitter usage)
- Events: `message`, `connect`, `disconnect`, `error`, `peer:connect`, `peer:disconnect`

---

## Phase 2: Multiplexed Peer Architecture (Priority: Critical)

**Goal:** Implement the multiplexed peer system where each node can act as both server and client simultaneously.

### 2.1 Peer Connection Management
**Status:** Not Started  
**Estimated Effort:** 3-4 weeks

**Requirements:**
- Track incoming and outgoing connections separately
- Unified peer map with connection metadata
- Connection type tracking (`incoming`/`outgoing`)
- Peer lifecycle management (connect, disconnect, reconnect)

**Files to Create:**
```
include/peer_connection.h
include/multiplexed_peer.h
src/peer_connection.cpp
src/multiplexed_peer.cpp
tests/test_multiplexed_peer.cpp
```

**API Design:**
```cpp
namespace umicp {

struct PeerConnection {
    std::string id;
    ConnectionType type; // INCOMING or OUTGOING
    std::optional<std::string> url;
    std::shared_ptr<Transport> transport;
    std::unordered_map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point connected_at;
    bool is_connected;
};

class MultiplexedPeer : public EventEmitter {
public:
    explicit MultiplexedPeer(const MultiplexedPeerOptions& options);
    ~MultiplexedPeer();
    
    // Server component - accept incoming connections
    Result<void> start_server();
    Result<void> stop_server();
    
    // Client component - connect to remote peers
    Result<std::string> connect_to_peer(const std::string& url, 
                                         const std::unordered_map<std::string, std::string>& metadata = {});
    
    // Peer management
    Result<void> disconnect_peer(const std::string& peer_id);
    Result<void> disconnect_peer_by_url(const std::string& url);
    
    // Message sending
    Result<void> send_to_peer(const std::string& peer_id, const Envelope& envelope);
    Result<void> send_to_peer_by_url(const std::string& url, const Envelope& envelope);
    
    // Broadcast
    size_t broadcast(const Envelope& envelope, const std::string& exclude_peer_id = "");
    size_t broadcast_to_type(const Envelope& envelope, ConnectionType type, 
                              const std::string& exclude_peer_id = "");
    
    // Request-response pattern
    Result<Envelope> send_and_wait(const std::string& peer_id, const Envelope& envelope, 
                                    std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    
    // Peer queries
    std::vector<PeerConnection> get_peers() const;
    std::vector<PeerConnection> get_peers_by_type(ConnectionType type) const;
    std::optional<PeerConnection> get_peer(const std::string& peer_id) const;
    std::optional<PeerConnection> find_peer_by_metadata(const std::string& key, 
                                                         const std::string& value) const;
    
    // Statistics
    struct PeerStats {
        size_t total_peers;
        size_t incoming_connections;
        size_t outgoing_connections;
        size_t messages_sent;
        size_t messages_received;
    };
    
    PeerStats get_stats() const;
    
    // Lifecycle
    Result<void> shutdown();
};

} // namespace umicp
```

**TypeScript Reference:**
- `src/transports/websocket-transport.ts`: Lines 545-958 (`UMICPWebSocketPeer` class)
- `docs/MULTIPLEXED_PEER.md`: Complete architecture documentation
- `test/multiplexed-peer.test.ts`: 38 comprehensive tests

---

## Phase 3: Advanced WebSocket Transport (Priority: High)

**Goal:** Enhance WebSocket transport with high-level features.

### 3.1 WebSocket Peer Implementation
**Status:** Partial (basic WebSocket exists)  
**Estimated Effort:** 2-3 weeks

**Requirements:**
- High-level WebSocket server with peer management
- High-level WebSocket client with auto-reconnect
- Handshake protocol (automatic CONTROL/ACK)
- Ping/pong heartbeat mechanism
- Connection timeout handling
- Per-message deflate compression

**Files to Modify/Create:**
```
include/websocket_peer.h
src/websocket_peer.cpp
tests/test_websocket_peer.cpp
```

**Features to Implement:**
1. **Automatic Handshake Protocol**
   - Send CONTROL message on connection
   - Wait for ACK response
   - Timeout if no ACK received
   
2. **Auto-Reconnect Logic**
   - Exponential backoff
   - Maximum retry attempts
   - Reconnect delay configuration
   - Event emission on reconnect attempts

3. **Heartbeat Mechanism**
   - Configurable ping interval
   - Connection timeout on missed pongs
   - Automatic disconnection on timeout

**TypeScript Reference:**
- `src/transports/websocket-transport.ts`: Lines 142-543
- `test/websocket-transport.test.ts`: Lines 1-450

---

## Phase 4: StreamableHTTP Transport (Priority: High)

**Goal:** Implement framework-agnostic HTTP transport layer.

### 4.1 HTTP Server/Client Implementation
**Status:** Not Started  
**Estimated Effort:** 2-3 weeks

**Requirements:**
- Framework-agnostic HTTP server
- Streaming HTTP client
- UMICP envelope over HTTP POST
- Health check endpoint (GET)
- Request/response correlation
- Concurrent request handling

**Files to Create:**
```
include/streamable_http.h
include/http_peer.h
src/streamable_http.cpp
src/http_peer.cpp
tests/test_streamable_http.cpp
```

**API Design:**
```cpp
namespace umicp {

class StreamableHTTPServer {
public:
    explicit StreamableHTTPServer(const HTTPServerOptions& options);
    
    Result<void> start();
    Result<void> stop();
    
    // Handle HTTP request
    Result<void> handle_request(const HTTPRequest& req, HTTPResponse& res);
    
    // Callbacks
    using MessageHandler = std::function<void(const Envelope&, const HTTPRequest&, HTTPResponse&)>;
    void set_message_handler(MessageHandler handler);
    
    // Statistics
    size_t get_active_connections() const;
};

class StreamableHTTPClient {
public:
    explicit StreamableHTTPClient(const HTTPClientOptions& options);
    
    Result<Envelope> send(const Envelope& envelope, 
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(30000));
    
    Result<void> send_async(const Envelope& envelope,
                            std::function<void(const Envelope&)> callback);
    
    // Health check
    Result<HealthCheckResponse> health_check();
};

class StreamableHTTPPeer : public EventEmitter {
public:
    explicit StreamableHTTPPeer(const HTTPPeerOptions& options);
    
    // Server component
    Result<void> start_server();
    
    // Client component
    Result<std::string> add_peer(const std::string& base_url, 
                                  const std::unordered_map<std::string, std::string>& metadata = {});
    
    // Message sending
    Result<void> send_to_peer(const std::string& peer_id, const Envelope& envelope);
    
    // Broadcast
    size_t broadcast(const Envelope& envelope, const std::string& exclude_peer_id = "");
    
    // Peer management
    std::vector<HTTPPeerConnection> get_peers() const;
};

} // namespace umicp
```

**TypeScript Reference:**
- `src/transports/streamable-http.ts`: Complete implementation
- `test/streamable-http.test.ts`: 50+ tests
- Examples: `examples/03-http-communication.ts`

---

## Phase 5: Broadcast and Routing Patterns (Priority: Medium)

**Goal:** Implement advanced message routing and broadcast capabilities.

### 5.1 Broadcast Strategies
**Status:** Not Started  
**Estimated Effort:** 1-2 weeks

**Requirements:**
- Broadcast to all peers
- Broadcast to incoming peers only
- Broadcast to outgoing peers only
- Selective broadcast (exclude specific peers)
- Role-based broadcast
- Conditional broadcast (filter by metadata)

**API Design:**
```cpp
namespace umicp {

class BroadcastManager {
public:
    // Broadcast to all
    size_t broadcast_all(const Envelope& envelope);
    
    // Broadcast to type
    size_t broadcast_to_incoming(const Envelope& envelope);
    size_t broadcast_to_outgoing(const Envelope& envelope);
    
    // Selective broadcast
    size_t broadcast_except(const Envelope& envelope, 
                            const std::vector<std::string>& exclude_peer_ids);
    
    // Role-based broadcast
    size_t broadcast_to_role(const Envelope& envelope, const std::string& role);
    
    // Conditional broadcast
    using BroadcastFilter = std::function<bool(const PeerConnection&)>;
    size_t broadcast_filtered(const Envelope& envelope, BroadcastFilter filter);
};

} // namespace umicp
```

**TypeScript Reference:**
- `examples/05-broadcast-patterns.ts`: Complete broadcast examples
- `test/multiplexed-peer.test.ts`: Lines 700-900 (broadcast tests)

---

## Phase 6: Request-Response Pattern (Priority: Medium)

**Goal:** Implement RPC-like request-response with correlation.

### 6.1 Request-Response Handler
**Status:** Not Started  
**Estimated Effort:** 1-2 weeks

**Requirements:**
- Send request and wait for response
- Correlation ID (`reply_to` capability)
- Timeout handling
- Promise/Future-like API
- Response caching
- Error handling

**API Design:**
```cpp
namespace umicp {

class RequestResponseManager {
public:
    // Send and wait for response
    Result<Envelope> request(const std::string& peer_id, 
                             const Envelope& request,
                             std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    
    // Async request with callback
    Result<void> request_async(const std::string& peer_id,
                               const Envelope& request,
                               std::function<void(const Result<Envelope>&)> callback,
                               std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    
    // Register request handler
    using RequestHandler = std::function<Envelope(const Envelope&)>;
    void register_handler(const std::string& operation, RequestHandler handler);
    
private:
    // Pending request tracking
    struct PendingRequest {
        std::string request_id;
        std::promise<Envelope> response_promise;
        std::chrono::steady_clock::time_point expiry;
    };
    
    std::unordered_map<std::string, PendingRequest> pending_requests_;
};

} // namespace umicp
```

**TypeScript Reference:**
- `examples/04-request-response-pattern.ts`: Complete RPC examples
- `test/multiplexed-peer.test.ts`: Lines 1000-1100 (send and wait tests)

---

## Phase 7: Advanced Connection Management (Priority: Medium)

**Goal:** Implement production-grade connection resilience.

### 7.1 Auto-Reconnect System
**Status:** Not Started  
**Estimated Effort:** 1-2 weeks

**Requirements:**
- Exponential backoff strategy
- Maximum retry attempts configuration
- Reconnect delay configuration
- Connection state machine
- Event emission on reconnect attempts
- Persistent connection monitoring

**API Design:**
```cpp
namespace umicp {

struct ReconnectConfig {
    bool enabled = true;
    size_t max_attempts = 0; // 0 = unlimited
    std::chrono::milliseconds initial_delay = std::chrono::milliseconds(1000);
    std::chrono::milliseconds max_delay = std::chrono::milliseconds(30000);
    double backoff_multiplier = 2.0;
};

class ReconnectManager {
public:
    explicit ReconnectManager(const ReconnectConfig& config);
    
    // Attempt reconnection
    Result<void> attempt_reconnect(std::shared_ptr<Transport> transport);
    
    // Reset reconnect state
    void reset(const std::string& peer_id);
    
    // Query reconnect status
    size_t get_attempt_count(const std::string& peer_id) const;
    std::chrono::milliseconds get_next_delay(const std::string& peer_id) const;
    
private:
    struct ReconnectState {
        size_t attempt_count = 0;
        std::chrono::milliseconds current_delay;
        std::chrono::steady_clock::time_point last_attempt;
    };
    
    std::unordered_map<std::string, ReconnectState> states_;
};

} // namespace umicp
```

**TypeScript Reference:**
- `src/transports/websocket-transport.ts`: Lines 200-300 (auto-reconnect logic)
- `test/reconnection-resilience.test.ts`: Comprehensive reconnection tests

---

## Phase 8: Statistics and Monitoring (Priority: Low)

**Goal:** Comprehensive monitoring and observability.

### 8.1 Enhanced Statistics
**Status:** Partial (basic stats exist)  
**Estimated Effort:** 1 week

**Requirements:**
- Per-peer statistics
- Connection duration tracking
- Message latency tracking
- Bandwidth monitoring
- Error rate tracking
- Exponential moving averages

**API Design:**
```cpp
namespace umicp {

struct PeerStatistics {
    std::string peer_id;
    ConnectionType type;
    
    // Message stats
    size_t messages_sent = 0;
    size_t messages_received = 0;
    size_t bytes_sent = 0;
    size_t bytes_received = 0;
    
    // Timing stats
    std::chrono::system_clock::time_point connected_at;
    std::chrono::milliseconds connection_duration;
    double avg_latency_ms = 0.0;
    
    // Health stats
    size_t error_count = 0;
    double error_rate = 0.0;
    std::chrono::system_clock::time_point last_activity;
};

class StatisticsManager {
public:
    // Record message
    void record_message_sent(const std::string& peer_id, size_t bytes);
    void record_message_received(const std::string& peer_id, size_t bytes);
    
    // Record latency
    void record_latency(const std::string& peer_id, std::chrono::milliseconds latency);
    
    // Record error
    void record_error(const std::string& peer_id);
    
    // Query stats
    PeerStatistics get_peer_stats(const std::string& peer_id) const;
    std::vector<PeerStatistics> get_all_stats() const;
    
    // Reset stats
    void reset_peer_stats(const std::string& peer_id);
    void reset_all_stats();
};

} // namespace umicp
```

**TypeScript Reference:**
- `src/transports/websocket-transport.ts`: Lines 850-950 (getStats implementation)
- `test/multiplexed-peer.test.ts`: Lines 1100-1150 (statistics tests)

---

## Phase 9: Integration Examples (Priority: Low)

**Goal:** Provide comprehensive usage examples.

### 9.1 Example Programs
**Status:** Basic examples exist  
**Estimated Effort:** 1-2 weeks

**Examples to Create:**
1. **Basic Peer Communication** (`examples/01-basic-peer-communication.cpp`)
   - Simple hub-worker pattern
   - Message exchange
   - Acknowledgments

2. **Mesh Network** (`examples/02-mesh-network.cpp`)
   - 3-node interconnected mesh
   - Ring topology
   - Broadcasting

3. **HTTP Communication** (`examples/03-http-communication.cpp`)
   - HTTP transport usage
   - Health checks
   - Request-response

4. **Request-Response Pattern** (`examples/04-request-response-pattern.cpp`)
   - RPC-style communication
   - Timeout handling
   - Error responses

5. **Broadcast Patterns** (`examples/05-broadcast-patterns.cpp`)
   - Various broadcast strategies
   - Selective broadcasting
   - Role-based broadcasting

6. **Federated Learning Simulation** (`examples/06-federated-learning-simulation.cpp`)
   - Model distribution
   - Aggregation pattern
   - Matrix operations

**TypeScript Reference:**
- Complete examples directory: `bindings/typescript/examples/`
- Documentation: `bindings/typescript/docs/EXAMPLES.md`

---

## Phase 10: Testing and Documentation (Priority: Critical)

**Goal:** Achieve >95% test coverage and comprehensive documentation.

### 10.1 Test Suite Expansion
**Status:** Good coverage on core, missing for new features  
**Estimated Effort:** 3-4 weeks

**Test Files to Create:**
```
tests/test_event_emitter.cpp
tests/test_multiplexed_peer.cpp
tests/test_websocket_peer.cpp
tests/test_streamable_http.cpp
tests/test_broadcast_patterns.cpp
tests/test_request_response.cpp
tests/test_reconnect_manager.cpp
tests/test_statistics_manager.cpp
```

**Test Coverage Goals:**
- Unit tests: >95% code coverage
- Integration tests: All major workflows
- Performance tests: Latency and throughput
- Stress tests: Connection limits and message rates
- Resilience tests: Reconnection and failover

**TypeScript Reference:**
- 19 comprehensive test files
- 163 total tests
- ~95-98% test coverage
- See: `bindings/typescript/docs/TEST_COVERAGE.md`

### 10.2 Documentation
**Status:** Basic documentation exists  
**Estimated Effort:** 2 weeks

**Documentation to Create:**
```
docs/MULTIPLEXED_PEER.md
docs/QUICKSTART.md
docs/API_REFERENCE.md
docs/EXAMPLES.md
docs/ARCHITECTURE.md
docs/PERFORMANCE_TUNING.md
docs/TROUBLESHOOTING.md
```

**Documentation Goals:**
- API reference with all classes and methods
- Quick start guide (5-minute setup)
- Comprehensive examples with explanations
- Architecture diagrams
- Performance benchmarks
- Troubleshooting guide

**TypeScript Reference:**
- Extensive documentation: `bindings/typescript/docs/`
- 8 documentation files
- Production approval reports
- Technical review reports

---

## Implementation Priority Matrix

| Phase | Priority | Effort | Dependencies | Impact |
|-------|----------|--------|--------------|--------|
| **1. Event System** | Critical | 2-3w | None | High - Foundation for all async features |
| **2. Multiplexed Peer** | Critical | 3-4w | Phase 1 | High - Core P2P functionality |
| **3. Advanced WebSocket** | High | 2-3w | Phase 1, 2 | High - Production-grade WebSocket |
| **4. StreamableHTTP** | High | 2-3w | Phase 1, 2 | Medium - Alternative transport |
| **5. Broadcast Patterns** | Medium | 1-2w | Phase 2 | Medium - Advanced messaging |
| **6. Request-Response** | Medium | 1-2w | Phase 2 | Medium - RPC pattern |
| **7. Connection Mgmt** | Medium | 1-2w | Phase 2, 3 | High - Production resilience |
| **8. Statistics** | Low | 1w | Phase 2 | Low - Observability |
| **9. Examples** | Low | 1-2w | All phases | Medium - Developer experience |
| **10. Testing/Docs** | Critical | 3-4w | All phases | High - Production readiness |

**Total Estimated Effort:** 18-25 weeks (4.5-6 months)

---

## Key Differences: C++ vs TypeScript

### TypeScript Advantages (To Implement)
1. ✅ **Multiplexed Peer Architecture** - True P2P with simultaneous server/client
2. ✅ **EventEmitter Pattern** - Async event-driven architecture
3. ✅ **StreamableHTTP Transport** - Framework-agnostic HTTP layer
4. ✅ **Advanced Broadcast** - Multiple broadcast strategies
5. ✅ **Auto-Reconnect** - Production-grade connection resilience
6. ✅ **Request-Response** - RPC-like messaging pattern
7. ✅ **Comprehensive Examples** - 6 detailed examples with docs
8. ✅ **Extensive Testing** - 163 tests, ~95% coverage

### C++ Advantages (Already Implemented)
1. ✅ **SIMD Optimization** - Hardware-accelerated matrix operations
2. ✅ **Advanced Security** - Full crypto suite with 2-way auth
3. ✅ **Schema Registry** - Message validation with schemas
4. ✅ **Multi-Transport Load Balancing** - Round-robin, least connections, etc.
5. ✅ **Topic-Based Routing** - Pub/sub messaging pattern
6. ✅ **Failover Management** - Automatic transport failover
7. ✅ **Compression Manager** - Multiple compression algorithms (LZ4, CBOR)

---

## Success Criteria

### Phase Completion Criteria
- [ ] All API functions implemented and tested
- [ ] Test coverage >95% for new code
- [ ] Documentation complete with examples
- [ ] Performance benchmarks meet targets
- [ ] Zero memory leaks (Valgrind clean)
- [ ] Thread-safe with stress testing

### Overall Success Metrics
- [ ] Feature parity with TypeScript binding achieved
- [ ] Performance: <1ms latency for local messages
- [ ] Performance: >10,000 messages/sec throughput
- [ ] Stability: 72-hour stress test without crashes
- [ ] Quality: Zero critical/high severity bugs
- [ ] Documentation: 100% API coverage

---

## Risk Assessment

### Technical Risks
1. **Event System Complexity**
   - Risk: Thread-safety issues in C++
   - Mitigation: Use proven patterns (Boost.Signals2 or custom with mutex)
   
2. **Memory Management**
   - Risk: Memory leaks in peer connection management
   - Mitigation: Smart pointers, RAII, Valgrind testing
   
3. **Performance Overhead**
   - Risk: Event system and callbacks add latency
   - Mitigation: Benchmark at each phase, optimize hot paths

4. **API Complexity**
   - Risk: C++ API more complex than TypeScript
   - Mitigation: Provide simplified wrappers, clear examples

### Project Risks
1. **Timeline**
   - Risk: 6-month timeline is aggressive
   - Mitigation: Prioritize critical features, iterate

2. **Testing Coverage**
   - Risk: Insufficient testing of new features
   - Mitigation: TDD approach, automated CI/CD

3. **Documentation**
   - Risk: Documentation lags implementation
   - Mitigation: Document as you code, examples first

---

## Next Steps

### Immediate Actions (Week 1-2)
1. Review and approve this roadmap
2. Set up project tracking (GitHub Issues/Projects)
3. Create feature branches for each phase
4. Begin Phase 1: Event System implementation

### Short Term (Month 1)
1. Complete Phase 1: Event System
2. Start Phase 2: Multiplexed Peer
3. Set up CI/CD for continuous testing
4. Create initial examples

### Medium Term (Month 2-4)
1. Complete Phases 2-5
2. Comprehensive integration testing
3. Performance benchmarking
4. Documentation sprint

### Long Term (Month 5-6)
1. Complete Phases 6-9
2. Final testing and bug fixes
3. Production readiness review
4. Release preparation

---

## References

### TypeScript Implementation
- **Location:** `umicp/bindings/typescript/`
- **Key Files:**
  - `src/transports/websocket-transport.ts` - Multiplexed peer implementation
  - `src/transports/streamable-http.ts` - HTTP transport
  - `docs/MULTIPLEXED_PEER.md` - Architecture documentation
  - `docs/TEST_COVERAGE.md` - Testing strategy
  - `examples/` - 6 comprehensive examples

### C++ Current Implementation
- **Location:** `umicp/cpp/`
- **Key Files:**
  - `include/protocol.h` - Protocol orchestrator
  - `include/transport.h` - Transport interface
  - `src/protocol.cpp` - Multi-transport support
  - `tests/` - Current test suite

### External Resources
- **Boost.Signals2:** https://www.boost.org/doc/libs/1_83_0/doc/html/signals2.html
- **WebSocket++:** https://github.com/zaphoyd/websocketpp
- **cpp-httplib:** https://github.com/yhirose/cpp-httplib

---

## Appendix A: API Comparison Matrix

| Feature | TypeScript API | C++ Current | C++ Proposed |
|---------|---------------|-------------|--------------|
| Create Peer | `new UMICPWebSocketPeer({...})` | N/A | `MultiplexedPeer peer(options)` |
| Event Handler | `peer.on('message', callback)` | N/A | `peer.on("message", callback)` |
| Connect to Peer | `await peer.connectToPeer(url)` | N/A | `peer.connect_to_peer(url)` |
| Send Message | `peer.sendToPeer(id, envelope)` | N/A | `peer.send_to_peer(id, envelope)` |
| Broadcast | `peer.broadcast(envelope)` | N/A | `peer.broadcast(envelope)` |
| Get Statistics | `peer.getStats()` | Partial | `peer.get_stats()` |
| Request-Response | `await peer.sendAndWait(id, envelope)` | N/A | `peer.send_and_wait(id, envelope)` |

---

## Appendix B: Migration Guide

For users of the current C++ implementation, the new API will be backwards compatible with the existing low-level API. High-level features will be opt-in.

### Current Code (Still Supported)
```cpp
// Low-level protocol API (unchanged)
Protocol protocol("client-id");
protocol.configure(config);
protocol.add_transport(transport, "ws-1");
protocol.connect();
protocol.send_data("server-id", data);
```

### New Code (Recommended)
```cpp
// High-level multiplexed peer API (new)
MultiplexedPeer peer({
    .peer_id = "client-id",
    .server = {
        .port = 8080,
        .path = "/umicp"
    }
});

peer.on("message", [](const Envelope& envelope, const PeerConnection& peer) {
    // Handle message
});

peer.connect_to_peer("ws://server:8080/umicp");
peer.send_to_peer("peer-id", envelope);
```

---

**Document Version:** 1.0  
**Author:** AI Assistant  
**Review Status:** Draft  
**Next Review:** After Phase 1 completion

