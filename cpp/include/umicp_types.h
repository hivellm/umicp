/**
 * UMICP (Universal Matrix Intelligent Communication Protocol) - C++ Core
 * Type definitions and constants
 *
 * Based on BIP-05 specifications
 */

#ifndef UMICP_TYPES_H
#define UMICP_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <chrono>

namespace umicp {

// Protocol constants
static constexpr uint8_t UMICP_VERSION_MAJOR = 1;
static constexpr uint8_t UMICP_VERSION_MINOR = 0;
static constexpr size_t UMICP_FRAME_HEADER_SIZE = 20;
static constexpr size_t UMICP_MAX_MESSAGE_SIZE = 1024 * 1024; // 1MB
static constexpr size_t UMICP_DEFAULT_BUFFER_SIZE = 4096;

// Operation types
enum class OperationType {
    CONTROL = 0,
    DATA = 1,
    ACK = 2,
    ERROR = 3
};

// Content types
enum class ContentType {
    JSON = 0,
    CBOR = 1,
    MSGPACK = 2
};

// Payload types
enum class PayloadType {
    VECTOR = 0,
    TEXT = 1,
    METADATA = 2,
    BINARY = 3
};

// Encoding types
enum class EncodingType {
    FLOAT32 = 0,
    FLOAT64 = 1,
    INT32 = 2,
    INT64 = 3,
    UINT8 = 4,
    UINT16 = 5,
    UINT32 = 6,
    UINT64 = 7
};

// Frame flags bitmask
enum FrameFlags : uint16_t {
    COMPRESSED_GZIP = 1 << 0,
    COMPRESSED_BROTLI = 1 << 1,
    ENCRYPTED_XCHACHA20 = 1 << 2,
    FRAGMENT_START = 1 << 3,
    FRAGMENT_CONTINUE = 1 << 4,
    FRAGMENT_END = 1 << 5,
    STREAM_START = 1 << 6,
    STREAM_END = 1 << 7
};

// Transport types
enum class TransportType {
    WEBSOCKET = 0,
    HTTP2 = 1,
    MATRIX = 2,
    DIRECT = 3
};

// Compression algorithms
enum class CompressionAlgorithm {
    NONE = 0,
    ZLIB = 1,
    GZIP = 2,
    LZ4 = 3
};

// Error codes
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_ENVELOPE = 1,
    INVALID_FRAME = 2,
    AUTHENTICATION_FAILED = 3,
    DECRYPTION_FAILED = 4,
    COMPRESSION_FAILED = 5,
    DECOMPRESSION_FAILED = 6,
    SERIALIZATION_FAILED = 7,
    NETWORK_ERROR = 8,
    TIMEOUT = 9,
    BUFFER_OVERFLOW = 10,
    MEMORY_ALLOCATION = 11,
    INVALID_ARGUMENT = 12,
    NOT_IMPLEMENTED = 13
};

// Forward declarations
struct Envelope;
struct Frame;
struct SecurityContext;
struct TransportConfig;
struct UMICPConfig;
class SecurityManager;

// Payload hint structure
struct PayloadHint {
    PayloadType type;
    std::optional<size_t> size;
    std::optional<EncodingType> encoding;
    std::optional<size_t> count;

    PayloadHint() : type(PayloadType::METADATA) {}

    PayloadHint(PayloadType t, size_t s, EncodingType e, size_t c)
        : type(t), size(s), encoding(e), count(c) {}
};

// Type aliases
using ByteBuffer = std::vector<uint8_t>;
using StringMap = std::unordered_map<std::string, std::string>;
using JsonObject = std::unordered_map<std::string, std::string>; // Simplified for MVP

// Envelope structure (JSON control plane)
struct Envelope {
    std::string version;
    std::string msg_id;
    std::string ts;
    std::string from;
    std::string to;
    OperationType op;
    std::optional<StringMap> capabilities;
    std::optional<std::string> schema_uri;
    std::optional<std::vector<std::string>> accept;
    std::optional<PayloadHint> payload_hint;
    std::optional<std::vector<JsonObject>> payload_refs;

    Envelope() : op(OperationType::CONTROL) {}
};

// Frame header (16 bytes)
struct FrameHeader {
    uint8_t version;
    uint8_t type;
    uint16_t flags;
    uint64_t stream_id;
    uint32_t sequence;
    uint32_t length;
};

// Binary frame structure (data plane)
struct Frame {
    FrameHeader header;
    ByteBuffer payload;

    Frame() = default;
    Frame(FrameHeader hdr, ByteBuffer data)
        : header(hdr), payload(std::move(data)) {}
};

// Security context
struct SecurityContext {
    std::string local_id;
    std::optional<std::string> remote_id;
    bool authenticated;
    std::optional<ByteBuffer> encryption_key;
    std::optional<ByteBuffer> signing_key;
    std::optional<ByteBuffer> peer_public_key;
    std::optional<std::string> session_id;

    SecurityContext(std::string id) : local_id(id), authenticated(false) {}
};

// Configuration structure
struct UMICPConfig {
    std::string version;
    size_t max_message_size;
    uint32_t connection_timeout;
    uint32_t heartbeat_interval;
    bool enable_binary;
    ContentType preferred_format;
    bool enable_compression;
    size_t compression_threshold;
    CompressionAlgorithm compression_algorithm;
    bool require_auth;
    bool require_encryption;
    bool validate_certificates;

    UMICPConfig()
        : version("1.0")
        , max_message_size(UMICP_MAX_MESSAGE_SIZE)
        , connection_timeout(30000)
        , heartbeat_interval(30000)
        , enable_binary(true)
        , preferred_format(ContentType::CBOR)
        , enable_compression(true)
        , compression_threshold(1024)
        , compression_algorithm(CompressionAlgorithm::ZLIB)
        , require_auth(true)
        , require_encryption(false)
        , validate_certificates(true)
    {}
};

// SSL/TLS configuration structure with advanced certificate validation
struct SSLConfig {
    bool enable_ssl;
    bool verify_peer;
    bool verify_host;
    bool check_certificate_revocation;
    bool require_client_certificate;
    std::string ca_file;
    std::string ca_path;
    std::string cert_file;
    std::string key_file;
    std::string key_password;
    std::string cipher_list;
    std::string certificate_policies;
    std::vector<std::string> trusted_certificates;
    int minimum_tls_version;
    std::string ciphersuites;
    bool enable_ocsp_stapling;
    bool enable_certificate_transparency;

    SSLConfig()
        : enable_ssl(false)
        , verify_peer(true)
        , verify_host(true)
        , check_certificate_revocation(true)
        , require_client_certificate(false)
        , minimum_tls_version(12) // TLS 1.2 minimum
        , enable_ocsp_stapling(false)
        , enable_certificate_transparency(false)
    {}
};

// Transport configuration
struct TransportConfig {
    TransportType type;
    std::string host;
    uint16_t port;
    std::string path;
    StringMap headers;
    std::optional<size_t> max_payload_size;
    std::optional<SSLConfig> ssl_config;

    TransportConfig()
        : type(TransportType::WEBSOCKET)
        , host("localhost")
        , port(8080)
        , max_payload_size(UMICP_MAX_MESSAGE_SIZE)
    {}
};

// Result structure for operations
template<typename T>
struct Result {
    ErrorCode code;
    std::optional<T> value;
    std::optional<std::string> error_message;

    Result() : code(ErrorCode::SUCCESS) {}
    Result(T val) : code(ErrorCode::SUCCESS), value(std::move(val)) {}
    Result(ErrorCode err, std::string msg)
        : code(err), error_message(std::move(msg)) {}

    bool is_success() const { return code == ErrorCode::SUCCESS; }
    bool has_value() const { return value.has_value(); }
};

// Specialization for void
template<>
struct Result<void> {
    ErrorCode code;
    std::optional<std::string> error_message;

    Result() : code(ErrorCode::SUCCESS) {}
    Result(ErrorCode err) : code(err) {}
    Result(ErrorCode err, std::string msg)
        : code(err), error_message(std::move(msg)) {}

    bool is_success() const { return code == ErrorCode::SUCCESS; }
    bool has_value() const { return is_success(); }
};

} // namespace umicp

#endif // UMICP_TYPES_H
