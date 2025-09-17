/**
 * UMICP Protocol Header
 * Main protocol orchestrator and message handling
 */

#ifndef UMICP_PROTOCOL_H
#define UMICP_PROTOCOL_H

#include "umicp_types.h"
#include "schema_registry.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>

namespace umicp {

// Forward declarations
class Transport;
class SecurityManager;
class CompressionManager;

// Message handler type
using MessageHandler = std::function<void(const Envelope&, const ByteBuffer*)>;

// Load balancing strategies
enum class LoadBalancingStrategy {
    ROUND_ROBIN = 0,
    LEAST_CONNECTIONS = 1,
    RANDOM = 2,
    WEIGHTED = 3
};

// Transport info structure
struct TransportInfo {
    std::string id;
    std::shared_ptr<Transport> transport;
    TransportType type;
    bool connected;
    size_t active_connections;
    size_t message_count;
    std::chrono::steady_clock::time_point last_activity;
    std::unordered_set<std::string> subscribed_topics;

    // Failover support
    bool failed;
    size_t failure_count;
    std::chrono::steady_clock::time_point last_failure;
    std::chrono::steady_clock::time_point next_retry;
    size_t retry_count;

    TransportInfo(std::string transport_id, std::shared_ptr<Transport> t, TransportType tp)
        : id(std::move(transport_id)), transport(std::move(t)), type(tp),
          connected(false), active_connections(0), message_count(0),
          last_activity(std::chrono::steady_clock::now()),
          failed(false), failure_count(0), retry_count(0) {}
};

// Protocol orchestrator class
class Protocol {
public:
    explicit Protocol(std::string local_id);
    ~Protocol();

    // Configuration
    Result<void> configure(const UMICPConfig& config);

    // Transport management - Multi-transport support
    Result<void> add_transport(std::shared_ptr<Transport> transport, const std::string& transport_id);
    Result<void> add_transport(TransportType type, const TransportConfig& transport_config, const std::string& transport_id);
    Result<void> remove_transport(const std::string& transport_id);
    Result<void> connect_transport(const std::string& transport_id);
    Result<void> disconnect_transport(const std::string& transport_id);
    Result<void> connect();
    Result<void> disconnect();
    bool is_connected() const;
    std::vector<std::string> get_transport_ids() const;

    // Topic-based routing
    Result<void> subscribe_topic(const std::string& topic, const std::string& transport_id = "");
    Result<void> unsubscribe_topic(const std::string& topic);
    Result<void> publish_topic(const std::string& topic, const ByteBuffer& data, const PayloadHint& hint = PayloadHint());
    std::vector<std::string> get_subscribed_topics() const;

    // Load balancing and failover
    Result<void> set_load_balancing_strategy(LoadBalancingStrategy strategy);
    Result<void> set_failover_enabled(bool enabled);
    LoadBalancingStrategy get_load_balancing_strategy() const;
    bool is_failover_enabled() const;

    // Failover management
    Result<void> mark_transport_failed(const std::string& transport_id);
    Result<void> retry_failed_transports();
    std::vector<std::string> get_failed_transport_ids() const;
    std::vector<std::string> get_healthy_transport_ids() const;

    // Schema Registry integration
    std::shared_ptr<SchemaRegistry> get_schema_registry();
    Result<void> validate_message_with_schema(const std::string& schema_id,
                                            const ByteBuffer& message_data,
                                            const std::string& content_type = "json");
    Result<void> register_message_schema(const SchemaDefinition& schema);

    // Message sending
    Result<std::string> send_control(const std::string& to, OperationType op, const std::string& command,
                                   const std::string& params = "");
    Result<std::string> send_data(const std::string& to, const ByteBuffer& data,
                                const PayloadHint& hint = PayloadHint());
    Result<std::string> send_ack(const std::string& to, const std::string& message_id);
    Result<std::string> send_error(const std::string& to, ErrorCode error, const std::string& message,
                                 const std::string& original_message_id = "");

    // Message handling
    void register_handler(OperationType op, MessageHandler handler);
    void unregister_handler(OperationType op);
    Result<void> process_message(const ByteBuffer& message_data);

    // Security
    Result<void> set_security_manager(std::shared_ptr<SecurityManager> security);
    bool is_authenticated() const;

    // Statistics and monitoring
    struct Stats {
        size_t messages_sent;
        size_t messages_received;
        size_t bytes_sent;
        size_t bytes_received;
        size_t errors_count;
        std::chrono::steady_clock::time_point start_time;
    };

    Stats get_stats() const;
    void reset_stats();

private:
    std::string local_id_;
    UMICPConfig config_;

    // Multi-transport support
    std::unordered_map<std::string, std::shared_ptr<TransportInfo>> transports_;

    // Topic-based routing
    std::unordered_set<std::string> global_subscribed_topics_;

    // Legacy support (backward compatibility)
    std::shared_ptr<Transport> transport_; // Kept for backward compatibility

    std::shared_ptr<SecurityManager> security_;
    std::unique_ptr<CompressionManager> compression_;
    std::shared_ptr<SchemaRegistry> schema_registry_;

    // Load balancing and failover
    LoadBalancingStrategy load_balancing_strategy_;
    bool failover_enabled_;
    mutable size_t round_robin_index_;
    std::unordered_map<OperationType, MessageHandler> handlers_;
    uint64_t next_stream_id_;
    Stats stats_;

    // Internal methods
    Result<std::string> generate_message_id();
    Result<Envelope> create_envelope(const std::string& to, OperationType op);
    Result<ByteBuffer> serialize_message(const Envelope& envelope, const ByteBuffer* payload = nullptr);
    Result<std::pair<Envelope, std::unique_ptr<ByteBuffer>>> deserialize_message(const ByteBuffer& data);

    void update_stats_sent(size_t bytes);
    void update_stats_received(size_t bytes);
    void update_stats_error();

    // Transport callback
    void on_transport_message(const ByteBuffer& data);
    void on_transport_connected();
    void on_transport_disconnected();
    void on_transport_error(const std::string& error);

    // Helper methods
    std::shared_ptr<TransportInfo> select_transport_for_topic(
        const std::vector<std::shared_ptr<TransportInfo>>& available_transports,
        const std::string& topic);
};

} // namespace umicp

#endif // UMICP_PROTOCOL_H
