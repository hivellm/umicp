/**
 * UMICP Protocol Implementation
 * Main protocol orchestrator and message handling
 */

#include "umicp_types.h"
#include "protocol.h"
#include "envelope.h"
#include "frame.h"
#include "transport.h"
#include "serialization.h"
#include "security.h"
#include "compression.h"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace umicp {

// Mutex for protecting concurrent access to protocol operations
static std::mutex protocol_mutex;

Protocol::Protocol(std::string local_id)
    : local_id_(std::move(local_id))
    , config_(UMICPConfig{})
    , compression_(std::make_unique<CompressionManager>(CompressionAlgorithm::ZLIB))
    , schema_registry_(std::make_shared<SchemaRegistry>())
    , load_balancing_strategy_(LoadBalancingStrategy::ROUND_ROBIN)
    , failover_enabled_(true)
    , round_robin_index_(0)
    , next_stream_id_(1)
    , stats_{0, 0, 0, 0, 0, std::chrono::steady_clock::now()} {
}

Protocol::~Protocol() = default;

Result<void> Protocol::configure(const UMICPConfig& config) {
    // Validate configuration parameters
    if (config.max_message_size == 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "max_message_size must be greater than 0");
    }

    if (config.connection_timeout == 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "connection_timeout must be greater than 0");
    }

    if (config.heartbeat_interval == 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "heartbeat_interval must be greater than 0");
    }

    // Check if already connected - prevent reconfiguration after connection
    if (is_connected()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Cannot reconfigure after connection");
    }

    config_ = config;

    // Configure compression
    if (compression_) {
        compression_->set_algorithm(config.compression_algorithm);
    }

    return Result<void>();
}



// ============================================================================
// Legacy Single-Transport Methods (for backward compatibility)
// ============================================================================

// Legacy connect method - tries to connect all transports or legacy transport
Result<void> Protocol::connect() {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    // If we have multiple transports configured (more than just the legacy one), use multi-transport logic
    if (transports_.size() > 1) {
        // Multi-transport connect logic
        bool any_connected = false;
        for (auto& [id, transport_info] : transports_) {
            if (!transport_info->connected) {
                auto result = transport_info->transport->connect();
                if (result.is_success()) {
                    transport_info->connected = true;
                    transport_info->last_activity = std::chrono::steady_clock::now();
                    any_connected = true;
                }
            } else {
                any_connected = true;
            }
        }

        if (!any_connected) {
            return Result<void>(ErrorCode::NETWORK_ERROR, "Failed to connect to any transport");
        }

        return Result<void>();
    }

    // Otherwise use legacy single-transport connect
    if (!transport_) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "No transport configured");
    }

    if (transport_->is_connected()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Already connected");
    }

    // Set up transport callbacks
    transport_->set_message_callback([this](const ByteBuffer& data) {
        on_transport_message(data);
    });

    transport_->set_connection_callback([this](bool connected, const std::string& error) {
        if (connected) {
            on_transport_connected();
        } else {
            on_transport_disconnected();
        }
    });

    transport_->set_error_callback([this](ErrorCode code, const std::string& message) {
        on_transport_error(message);
    });

    auto result = transport_->connect();
    if (result.is_success()) {
        return Result<void>();
    } else {
        return Result<void>(result.code, result.error_message.value_or("Connection failed"));
    }
}

// Legacy disconnect method
Result<void> Protocol::disconnect() {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    // If we have multiple transports, disconnect all
    if (transports_.size() > 1) {
        for (auto& [id, transport_info] : transports_) {
            if (transport_info->connected) {
                transport_info->transport->disconnect();
                transport_info->connected = false;
                transport_info->active_connections = 0;
            }
        }
        return Result<void>();
    }

    // Otherwise use legacy single-transport disconnect
    if (!transport_) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "No transport configured");
    }

    auto result = transport_->disconnect();
    if (result.is_success()) {
        on_transport_disconnected();
        return Result<void>();
    } else {
        return Result<void>(result.code, result.error_message.value_or("Disconnection failed"));
    }
}

Result<std::string> Protocol::send_control(const std::string& to, OperationType op,
                                         const std::string& command, const std::string& params) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    // Validate input parameters
    if (to.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Destination 'to' cannot be empty");
    }

    if (command.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Command cannot be empty");
    }

    // Validate operation type
    if (op < OperationType::CONTROL || op > OperationType::ERROR) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Invalid operation type");
    }

    if (!transport_) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "No transport configured");
    }

    if (!transport_->is_connected()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Transport not connected");
    }

    auto envelope_result = create_envelope(to, op);
    if (!envelope_result.is_success()) {
        return Result<std::string>(envelope_result.code, envelope_result.error_message.value());
    }

    // Add control parameters to envelope
    envelope_result.value->capabilities = StringMap{
        {"command", command}
    };

    if (!params.empty()) {
        if (!envelope_result.value->capabilities) {
            envelope_result.value->capabilities = StringMap();
        }
        (*envelope_result.value->capabilities)["params"] = params;
    }

    auto send_result = transport_->send_envelope(*envelope_result.value);
    if (!send_result.is_success()) {
        return Result<std::string>(send_result.code, send_result.error_message.value());
    }

    // Estimate sent bytes for stats (rough approximation)
    auto json_result = JsonSerializer::serialize_envelope(*envelope_result.value);
    if (json_result.is_success()) {
        update_stats_sent(json_result.value->size());
    }

    return Result<std::string>(envelope_result.value->msg_id);
}

Result<std::string> Protocol::send_data(const std::string& to, const ByteBuffer& data,
                                       const PayloadHint& hint) {
    // Validate input parameters
    if (to.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Destination 'to' cannot be empty");
    }

    if (data.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Data cannot be empty");
    }

    if (data.size() > config_.max_message_size) {
        return Result<std::string>(ErrorCode::BUFFER_OVERFLOW,
            "Message size exceeds maximum allowed size");
    }

    auto envelope_result = create_envelope(to, OperationType::DATA);
    if (!envelope_result.is_success()) {
        return Result<std::string>(envelope_result.code, envelope_result.error_message.value());
    }

    // Convert PayloadHint to JsonObject for now
    if (hint.type != PayloadType::METADATA || hint.size || hint.encoding || hint.count) {
        JsonObject hint_obj;
        hint_obj["type"] = std::to_string(static_cast<int>(hint.type));
        if (hint.size) hint_obj["size"] = std::to_string(*hint.size);
        if (hint.encoding) hint_obj["encoding"] = std::to_string(static_cast<int>(*hint.encoding));
        if (hint.count) hint_obj["count"] = std::to_string(*hint.count);
        // envelope_result.value->payload_hint = hint_obj; // TODO: Convert JsonObject to PayloadHint
    }

    // Handle compression if enabled and data is large enough
    ByteBuffer processed_data = data;
    uint16_t flags = 0;

    if (config_.enable_compression &&
        CompressionManager::should_compress(data, config_.compression_threshold, config_.compression_algorithm)) {
        auto compression_result = compression_->compress(data);
        if (compression_result.is_success()) {
            processed_data = *compression_result.value;
            flags |= 0x01; // Set compression flag
        }
    }

    // For data messages, create a frame with the processed data
    Frame frame;
    frame.header.version = 1;
    frame.header.type = static_cast<uint8_t>(OperationType::DATA);
    frame.header.flags = flags;
    frame.header.stream_id = next_stream_id_++;
    frame.header.sequence = 0;
    frame.payload = processed_data;

    auto send_result = transport_->send_frame(frame);
    if (!send_result.is_success()) {
        return Result<std::string>(send_result.code, send_result.error_message.value());
    }

    update_stats_sent(data.size() + UMICP_FRAME_HEADER_SIZE);
    return Result<std::string>(envelope_result.value->msg_id);
}

Result<std::string> Protocol::send_ack(const std::string& to, const std::string& message_id) {
    auto envelope_result = create_envelope(to, OperationType::ACK);
    if (!envelope_result.is_success()) {
        return Result<std::string>(envelope_result.code, envelope_result.error_message.value());
    }

    // Add ACK reference
    envelope_result.value->payload_refs = std::vector<JsonObject>{
        JsonObject{{"message_id", message_id}, {"status", "OK"}}
    };

    auto send_result = transport_->send_envelope(*envelope_result.value);
    if (!send_result.is_success()) {
        return Result<std::string>(send_result.code, send_result.error_message.value());
    }

    // Estimate sent bytes for stats
    auto json_result = JsonSerializer::serialize_envelope(*envelope_result.value);
    if (json_result.is_success()) {
        update_stats_sent(json_result.value->size());
    }

    return Result<std::string>(envelope_result.value->msg_id);
}

Result<std::string> Protocol::send_error(const std::string& to, ErrorCode error,
                                        const std::string& message, const std::string& original_message_id) {
    auto envelope_result = create_envelope(to, OperationType::ERROR);
    if (!envelope_result.is_success()) {
        return Result<std::string>(envelope_result.code, envelope_result.error_message.value());
    }

    // Add error details
    envelope_result.value->payload_refs = std::vector<JsonObject>{
        JsonObject{
            {"error_code", std::to_string(static_cast<int>(error))},
            {"error_message", message}
        }
    };

    if (!original_message_id.empty()) {
        if (envelope_result.value->payload_refs && !envelope_result.value->payload_refs->empty()) {
            (*envelope_result.value->payload_refs)[0]["original_message_id"] = original_message_id;
        }
    }

    auto send_result = transport_->send_envelope(*envelope_result.value);
    if (!send_result.is_success()) {
        return Result<std::string>(send_result.code, send_result.error_message.value());
    }

    // Estimate sent bytes for stats
    auto json_result = JsonSerializer::serialize_envelope(*envelope_result.value);
    if (json_result.is_success()) {
        update_stats_sent(json_result.value->size());
    }

    return Result<std::string>(envelope_result.value->msg_id);
}

void Protocol::register_handler(OperationType op, MessageHandler handler) {
    handlers_[op] = std::move(handler);
}

void Protocol::unregister_handler(OperationType op) {
    handlers_.erase(op);
}

Result<void> Protocol::process_message(const ByteBuffer& message_data) {
    auto deserialize_result = deserialize_message(message_data);
    if (!deserialize_result.is_success()) {
        update_stats_error();
        return Result<void>(deserialize_result.code, deserialize_result.error_message.value());
    }

    const auto& [envelope, payload] = *deserialize_result.value;
    update_stats_received(message_data.size());

    // Call appropriate handler
    auto it = handlers_.find(envelope.op);
    if (it != handlers_.end()) {
        try {
            it->second(envelope, payload.get());
        } catch (const std::exception& e) {
            update_stats_error();
            return Result<void>(ErrorCode::INVALID_ARGUMENT,
                std::string("Handler exception: ") + e.what());
        }
    }

    return Result<void>();
}

Result<void> Protocol::set_security_manager(std::shared_ptr<SecurityManager> security) {
    if (!security) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null security manager provided");
    }

    security_ = security;
    return Result<void>();
}

bool Protocol::is_authenticated() const {
    return security_ && security_->authenticated;
}

Protocol::Stats Protocol::get_stats() const {
    return stats_;
}

void Protocol::reset_stats() {
    stats_.messages_sent = 0;
    stats_.messages_received = 0;
    stats_.bytes_sent = 0;
    stats_.bytes_received = 0;
    stats_.errors_count = 0;
    stats_.start_time = std::chrono::steady_clock::now();
}

// Private methods
Result<std::string> Protocol::generate_message_id() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999);

    std::stringstream ss;
    ss << "msg-" << timestamp << "-" << std::setfill('0') << std::setw(3) << dis(gen);
    return Result<std::string>(ss.str());
}

Result<Envelope> Protocol::create_envelope(const std::string& to, OperationType op) {
    Envelope envelope;
    envelope.version = config_.version;
    envelope.from = local_id_;
    envelope.to = to;
    envelope.op = op;

    auto msg_id_result = generate_message_id();
    if (!msg_id_result.is_success()) {
        return Result<Envelope>(msg_id_result.code, msg_id_result.error_message.value());
    }

    envelope.msg_id = *msg_id_result.value;

    // Set current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3)
       << (std::chrono::duration_cast<std::chrono::milliseconds>(
           now.time_since_epoch()).count() % 1000);
    ss << "Z";
    envelope.ts = ss.str();

    return Result<Envelope>(envelope);
}

Result<ByteBuffer> Protocol::serialize_message(const Envelope& envelope, const ByteBuffer* payload) {
    // Serialize envelope to JSON
    auto json_result = JsonSerializer::serialize_envelope(envelope);
    if (!json_result.is_success()) {
        return Result<ByteBuffer>(json_result.code, json_result.error_message.value());
    }

    const std::string& json_str = *json_result.value;
    ByteBuffer buffer;

    if (payload && !payload->empty()) {
        // For messages with payload, create a frame
        Frame frame;
        frame.header.version = 1;
        frame.header.type = static_cast<uint8_t>(envelope.op);
        frame.header.flags = 0;
        frame.header.stream_id = next_stream_id_++;
        frame.header.sequence = 0;
        frame.payload = *payload;

        auto frame_result = BinarySerializer::serialize_frame(frame);
        if (!frame_result.is_success()) {
            return Result<ByteBuffer>(frame_result.code, frame_result.error_message.value());
        }

        buffer = *frame_result.value;
    } else {
        // For control messages, just use JSON
        buffer.assign(json_str.begin(), json_str.end());
    }

    return Result<ByteBuffer>(buffer);
}

Result<std::pair<Envelope, std::unique_ptr<ByteBuffer>>> Protocol::deserialize_message(const ByteBuffer& data) {
    std::unique_ptr<ByteBuffer> payload;

    // Check if this looks like a binary frame (starts with frame header)
    if (data.size() >= UMICP_FRAME_HEADER_SIZE) {
        // Try to parse as binary frame
        auto frame_result = BinarySerializer::deserialize_frame(data);
        if (frame_result.is_success()) {
            // This is a binary frame - create envelope from frame header
            const Frame& frame = *frame_result.value;

            Envelope envelope;
            envelope.version = std::to_string(frame.header.version);
            envelope.op = static_cast<OperationType>(frame.header.type);
            envelope.msg_id = "frame-" + std::to_string(frame.header.stream_id) + "-" + std::to_string(frame.header.sequence);
            envelope.from = ""; // Would be extracted from frame metadata in full implementation
            envelope.to = local_id_; // Assume it's for us

            // Current timestamp
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
            envelope.ts = ss.str();

            // Handle decompression if compression flag is set
            ByteBuffer processed_payload = frame.payload;
            if (frame.header.flags & 0x01 && compression_) { // Compression flag
                auto decompression_result = compression_->decompress(frame.payload);
                if (decompression_result.is_success()) {
                    processed_payload = *decompression_result.value;
                } else {
                    // If decompression fails, use original payload
                    processed_payload = frame.payload;
                }
            }

            payload = std::make_unique<ByteBuffer>(processed_payload);

            return Result<std::pair<Envelope, std::unique_ptr<ByteBuffer>>>(
                std::make_pair(envelope, std::move(payload)));
        }
    }

    // Try to parse as JSON envelope
    std::string json_str(data.begin(), data.end());
    auto envelope_result = JsonSerializer::deserialize_envelope(json_str);
    if (!envelope_result.is_success()) {
        return Result<std::pair<Envelope, std::unique_ptr<ByteBuffer>>>(
            envelope_result.code, envelope_result.error_message.value());
    }

    // No payload for JSON control messages
    return Result<std::pair<Envelope, std::unique_ptr<ByteBuffer>>>(
        std::make_pair(*envelope_result.value, std::move(payload)));
}

void Protocol::update_stats_sent(size_t bytes) {
    stats_.messages_sent++;
    stats_.bytes_sent += bytes;
}

void Protocol::update_stats_received(size_t bytes) {
    stats_.messages_received++;
    stats_.bytes_received += bytes;
}

void Protocol::update_stats_error() {
    stats_.errors_count++;
}

void Protocol::on_transport_message(const ByteBuffer& data) {
    auto result = process_message(data);
    if (!result.is_success()) {
        // Handle processing error
    }
}

void Protocol::on_transport_connected() {
    // Handle connection established
}

void Protocol::on_transport_disconnected() {
    // Handle disconnection
}

void Protocol::on_transport_error(const std::string& /* error */) {
    // Handle transport error
    update_stats_error();
}

// ============================================================================
// Multi-Transport Management
// ============================================================================

Result<void> Protocol::add_transport(std::shared_ptr<Transport> transport, const std::string& transport_id) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    if (transport_id.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport ID cannot be empty");
    }

    if (transports_.find(transport_id) != transports_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport ID already exists");
    }

    auto transport_info = std::make_shared<TransportInfo>(transport_id, transport, TransportType::WEBSOCKET);
    transports_[transport_id] = transport_info;

    return Result<void>();
}

Result<void> Protocol::add_transport(TransportType type, const TransportConfig& transport_config, const std::string& transport_id) {
    auto transport = TransportFactory::create(type, transport_config, config_);
    if (!transport) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Failed to create transport");
    }

    return add_transport(std::move(transport), transport_id);
}

Result<void> Protocol::remove_transport(const std::string& transport_id) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    auto it = transports_.find(transport_id);
    if (it == transports_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport not found");
    }

    // Disconnect if connected
    if (it->second->connected) {
        it->second->transport->disconnect();
    }

    transports_.erase(it);
    return Result<void>();
}

Result<void> Protocol::connect_transport(const std::string& transport_id) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    auto it = transports_.find(transport_id);
    if (it == transports_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport not found");
    }

    if (it->second->connected) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport already connected");
    }

    auto result = it->second->transport->connect();
    if (result.is_success()) {
        it->second->connected = true;
        it->second->last_activity = std::chrono::steady_clock::now();
    }

    return result;
}

Result<void> Protocol::disconnect_transport(const std::string& transport_id) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    auto it = transports_.find(transport_id);
    if (it == transports_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport not found");
    }

    if (!it->second->connected) {
        return Result<void>();
    }

    auto result = it->second->transport->disconnect();
    it->second->connected = false;
    it->second->active_connections = 0;

    return result;
}


bool Protocol::is_connected() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    for (const auto& [id, transport_info] : transports_) {
        if (transport_info->connected) {
            return true;
        }
    }

    return false;
}

std::vector<std::string> Protocol::get_transport_ids() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    std::vector<std::string> ids;
    ids.reserve(transports_.size());

    for (const auto& [id, transport_info] : transports_) {
        ids.push_back(id);
    }

    return ids;
}

// ============================================================================
// Topic-Based Routing
// ============================================================================

Result<void> Protocol::subscribe_topic(const std::string& topic, const std::string& transport_id) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    if (topic.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Topic cannot be empty");
    }

    if (!transport_id.empty()) {
        // Subscribe to specific transport
        auto it = transports_.find(transport_id);
        if (it == transports_.end()) {
            return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport not found");
        }

        if (!it->second->connected) {
            return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport not connected");
        }

        it->second->subscribed_topics.insert(topic);
    } else {
        // Subscribe to all connected transports
        bool any_subscribed = false;
        for (auto& [id, transport_info] : transports_) {
            if (transport_info->connected) {
                transport_info->subscribed_topics.insert(topic);
                any_subscribed = true;
            }
        }

        if (!any_subscribed) {
            return Result<void>(ErrorCode::INVALID_ARGUMENT, "No connected transports available");
        }
    }

    global_subscribed_topics_.insert(topic);
    return Result<void>();
}

Result<void> Protocol::unsubscribe_topic(const std::string& topic) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    if (topic.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Topic cannot be empty");
    }

    // Remove from all transports
    for (auto& [id, transport_info] : transports_) {
        transport_info->subscribed_topics.erase(topic);
    }

    global_subscribed_topics_.erase(topic);
    return Result<void>();
}

Result<void> Protocol::publish_topic(const std::string& topic, const ByteBuffer& data, const PayloadHint& hint) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    if (topic.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Topic cannot be empty");
    }

    // Find transports that are subscribed to this topic
    std::vector<std::shared_ptr<TransportInfo>> available_transports;

    for (auto& [id, transport_info] : transports_) {
        if (transport_info->connected && transport_info->subscribed_topics.count(topic)) {
            available_transports.push_back(transport_info);
        }
    }

    if (available_transports.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "No transports subscribed to topic");
    }

    // Select transport based on load balancing strategy
    auto selected_transport = select_transport_for_topic(available_transports, topic);

    if (!selected_transport) {
        return Result<void>(ErrorCode::NETWORK_ERROR, "No suitable transport available");
    }

    // Create envelope for topic message
    auto envelope_result = create_envelope(topic, OperationType::DATA);
    if (!envelope_result.is_success()) {
        return Result<void>(envelope_result.code, envelope_result.error_message.value());
    }

    // Add topic information to envelope
    envelope_result.value->capabilities = StringMap{
        {"topic", topic},
        {"type", "publish"}
    };

    // Send via selected transport
    auto send_result = selected_transport->transport->send_envelope(*envelope_result.value);
    if (send_result.is_success()) {
        selected_transport->message_count++;
        selected_transport->last_activity = std::chrono::steady_clock::now();
        update_stats_sent(data.size());
    }

    return send_result;
}

std::vector<std::string> Protocol::get_subscribed_topics() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    return std::vector<std::string>(global_subscribed_topics_.begin(), global_subscribed_topics_.end());
}

// ============================================================================
// Load Balancing and Failover
// ============================================================================

Result<void> Protocol::set_load_balancing_strategy(LoadBalancingStrategy strategy) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    load_balancing_strategy_ = strategy;
    return Result<void>();
}

Result<void> Protocol::set_failover_enabled(bool enabled) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    failover_enabled_ = enabled;
    return Result<void>();
}

LoadBalancingStrategy Protocol::get_load_balancing_strategy() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    return load_balancing_strategy_;
}

bool Protocol::is_failover_enabled() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    return failover_enabled_;
}

// ============================================================================
// Failover Management
// ============================================================================

Result<void> Protocol::mark_transport_failed(const std::string& transport_id) {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    auto it = transports_.find(transport_id);
    if (it == transports_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Transport not found");
    }

    auto& transport_info = it->second;
    transport_info->connected = false;
    transport_info->failed = true;
    transport_info->failure_count++;
    transport_info->last_failure = std::chrono::steady_clock::now();

    // Schedule retry based on exponential backoff
    auto retry_delay = std::chrono::milliseconds(1000 * (1 << std::min(transport_info->retry_count, size_t(6))));
    transport_info->next_retry = transport_info->last_failure + retry_delay;
    transport_info->retry_count++;

    return Result<void>();
}

Result<void> Protocol::retry_failed_transports() {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    if (!failover_enabled_) {
        return Result<void>();
    }

    auto now = std::chrono::steady_clock::now();
    bool any_retry_successful = false;

    for (auto& [id, transport_info] : transports_) {
        if (transport_info->failed && now >= transport_info->next_retry) {
            // Try to reconnect
            auto result = transport_info->transport->connect();
            if (result.is_success()) {
                transport_info->connected = true;
                transport_info->failed = false;
                transport_info->retry_count = 0;
                transport_info->last_activity = now;
                any_retry_successful = true;
            } else {
                // Schedule next retry
                auto retry_delay = std::chrono::milliseconds(1000 * (1 << std::min(transport_info->retry_count, size_t(6))));
                transport_info->next_retry = now + retry_delay;
                transport_info->retry_count++;
            }
        }
    }

    if (!any_retry_successful && transports_.empty()) {
        return Result<void>(ErrorCode::NETWORK_ERROR, "No transports available for retry");
    }

    return Result<void>();
}

std::vector<std::string> Protocol::get_failed_transport_ids() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    std::vector<std::string> failed_ids;

    for (const auto& [id, transport_info] : transports_) {
        if (transport_info->failed) {
            failed_ids.push_back(id);
        }
    }

    return failed_ids;
}

std::vector<std::string> Protocol::get_healthy_transport_ids() const {
    std::lock_guard<std::mutex> lock(protocol_mutex);

    std::vector<std::string> healthy_ids;

    for (const auto& [id, transport_info] : transports_) {
        if (transport_info->connected && !transport_info->failed) {
            healthy_ids.push_back(id);
        }
    }

    return healthy_ids;
}

// ============================================================================
// Helper Methods
// ============================================================================

std::shared_ptr<TransportInfo> Protocol::select_transport_for_topic(
    const std::vector<std::shared_ptr<TransportInfo>>& available_transports,
    const std::string& topic) {

    if (available_transports.empty()) {
        return nullptr;
    }

    // Filter out failed transports if failover is enabled
    std::vector<std::shared_ptr<TransportInfo>> healthy_transports;
    for (const auto& transport : available_transports) {
        if (!transport->failed || !failover_enabled_) {
            healthy_transports.push_back(transport);
        }
    }

    // If no healthy transports and failover is enabled, try to retry failed ones
    if (healthy_transports.empty() && failover_enabled_) {
        retry_failed_transports();

        // Re-filter after retry attempt
        for (const auto& transport : available_transports) {
            if (!transport->failed) {
                healthy_transports.push_back(transport);
            }
        }
    }

    if (healthy_transports.empty()) {
        return nullptr;
    }

    switch (load_balancing_strategy_) {
        case LoadBalancingStrategy::ROUND_ROBIN: {
            size_t index = round_robin_index_ % healthy_transports.size();
            round_robin_index_ = (round_robin_index_ + 1) % healthy_transports.size();
            return healthy_transports[index];
        }

        case LoadBalancingStrategy::LEAST_CONNECTIONS: {
            auto min_transport = healthy_transports[0];
            for (const auto& transport : healthy_transports) {
                if (transport->active_connections < min_transport->active_connections) {
                    min_transport = transport;
                }
            }
            return min_transport;
        }

        case LoadBalancingStrategy::RANDOM: {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> dist(0, healthy_transports.size() - 1);
            return healthy_transports[dist(gen)];
        }

        case LoadBalancingStrategy::WEIGHTED: {
            // Simple weighted based on message count (prefer less used)
            auto min_transport = healthy_transports[0];
            for (const auto& transport : healthy_transports) {
                if (transport->message_count < min_transport->message_count) {
                    min_transport = transport;
                }
            }
            return min_transport;
        }

        default:
            return healthy_transports[0];
    }
}

// ============================================================================
// Schema Registry Integration
// ============================================================================

std::shared_ptr<SchemaRegistry> Protocol::get_schema_registry() {
    return schema_registry_;
}

Result<void> Protocol::validate_message_with_schema(const std::string& schema_id,
                                                  const ByteBuffer& message_data,
                                                  const std::string& content_type) {
    if (!schema_registry_) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema registry not available");
    }

    auto result = schema_registry_->validate_message(schema_id, message_data, content_type);

    if (!result.valid) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT,
                          "Schema validation failed: " + result.error_message);
    }

    return Result<void>();
}

Result<void> Protocol::register_message_schema(const SchemaDefinition& schema) {
    if (!schema_registry_) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema registry not available");
    }

    return schema_registry_->register_schema(schema);
}

} // namespace umicp
