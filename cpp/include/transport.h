/**
 * UMICP Transport Layer Interface
 * Abstract transport implementation for multiple protocols
 */

#ifndef UMICP_TRANSPORT_H
#define UMICP_TRANSPORT_H

#include "umicp_types.h"
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace umicp {

// Transport callbacks
using MessageCallback = ::std::function<void(const ByteBuffer&)>;
using ConnectionCallback = ::std::function<void(bool connected, const ::std::string& error)>;
using ErrorCallback = ::std::function<void(ErrorCode code, const ::std::string& message)>;

// Transport statistics
struct TransportStats {
    uint64_t bytes_sent{0};
    uint64_t bytes_received{0};
    uint64_t messages_sent{0};
    uint64_t messages_received{0};
    uint64_t connection_count{0};
    ::std::chrono::steady_clock::time_point last_activity;
};

// Abstract transport interface
class Transport {
public:
    virtual ~Transport() = default;

    // Connection management
    virtual Result<void> connect() = 0;
    virtual Result<void> disconnect() = 0;
    virtual bool is_connected() const = 0;

    // Message sending
    virtual Result<void> send(const ByteBuffer& data) = 0;
    virtual Result<void> send_envelope(const Envelope& envelope) = 0;
    virtual Result<void> send_frame(const Frame& frame) = 0;

    // Configuration
    virtual Result<void> configure(const TransportConfig& config) = 0;
    virtual TransportConfig get_config() const = 0;

    // Callbacks
    virtual void set_message_callback(MessageCallback callback) = 0;
    virtual void set_connection_callback(ConnectionCallback callback) = 0;
    virtual void set_error_callback(ErrorCallback callback) = 0;

    // Statistics
    virtual TransportStats get_stats() const = 0;
    virtual void reset_stats() = 0;

    // Transport-specific info
    virtual TransportType get_type() const = 0;
    virtual ::std::string get_endpoint() const = 0;
};

// Transport implementations are now in bindings, not in core

// Transport factory
class TransportFactory {
public:
    static ::std::unique_ptr<Transport> create(TransportType type, const TransportConfig& config);
    static ::std::unique_ptr<Transport> create(TransportType type, const TransportConfig& config, const UMICPConfig& umicp_config);
    static TransportConfig apply_umicp_config(const TransportConfig& transport_config, const UMICPConfig& umicp_config);
};

} // namespace umicp

#endif // UMICP_TRANSPORT_H
