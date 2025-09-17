/**
 * UMICP C API Implementation
 * C-compatible interface for FFI bindings
 */

#include "umicp.h"
#include "umicp_types.h"
#include "envelope.h"
#include "frame.h"
#include "matrix_ops.h"
#include "protocol.h"
#include "config.h"
#include "buffer.h"
#include <cstring>
#include <memory>
#include <new>

// Opaque type implementations
struct UMICP_Buffer {
    umicp::ByteBuffer buffer;
};

struct UMICP_Envelope {
    umicp::Envelope envelope;
};

struct UMICP_Frame {
    umicp::Frame frame;
};

struct UMICP_Config {
    umicp::UMICPConfig config;
};

struct UMICP_Protocol {
    std::unique_ptr<umicp::Protocol> protocol;
};

struct UMICP_Security {
    umicp::SecurityContext context;
};

struct UMICP_Transport {
    // Placeholder for transport implementation
    bool connected;
};

// Global memory management
static UMICP_AllocFunc g_alloc_func = nullptr;
static UMICP_FreeFunc g_free_func = nullptr;

// Helper functions
static void* allocate_memory(size_t size) {
    if (g_alloc_func) {
        return g_alloc_func(size);
    }
    return malloc(size);
}

static void free_memory(void* ptr) {
    if (g_free_func) {
        g_free_func(ptr);
        return;
    }
    free(ptr);
}

template<typename T>
static T* create_object() {
    try {
        return new (allocate_memory(sizeof(T))) T();
    } catch (...) {
        return nullptr;
    }
}

template<typename T>
static void destroy_object(T* obj) {
    if (obj) {
        obj->~T();
        free_memory(obj);
    }
}

// Buffer management
UMICP_Buffer* umicp_buffer_create(size_t capacity) {
    if (capacity == 0) {
        capacity = 4096; // Default capacity
    }

    auto* buffer = create_object<UMICP_Buffer>();
    if (!buffer) {
        return nullptr;
    }

    try {
        buffer->buffer.reserve(capacity);
        return buffer;
    } catch (...) {
        destroy_object(buffer);
        return nullptr;
    }
}

void umicp_buffer_destroy(UMICP_Buffer* buffer) {
    destroy_object(buffer);
}

uint8_t* umicp_buffer_data(UMICP_Buffer* buffer) {
    if (!buffer) return nullptr;
    return buffer->buffer.data();
}

size_t umicp_buffer_size(UMICP_Buffer* buffer) {
    if (!buffer) return 0;
    return buffer->buffer.size();
}

size_t umicp_buffer_capacity(UMICP_Buffer* buffer) {
    if (!buffer) return 0;
    return buffer->buffer.capacity();
}

UMICP_ErrorCode umicp_buffer_resize(UMICP_Buffer* buffer, size_t new_size) {
    if (!buffer) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        buffer->buffer.resize(new_size);
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_MEMORY_ALLOCATION;
    }
}

UMICP_ErrorCode umicp_buffer_append(UMICP_Buffer* buffer, const uint8_t* data, size_t size) {
    if (!buffer || (!data && size > 0)) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        buffer->buffer.insert(buffer->buffer.end(), data, data + size);
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_MEMORY_ALLOCATION;
    }
}

// Configuration management
UMICP_Config* umicp_config_create(void) {
    auto* config = create_object<UMICP_Config>();
    if (!config) {
        return nullptr;
    }

    // Initialize with defaults
    config->config = umicp::UMICPConfig{};
    return config;
}

void umicp_config_destroy(UMICP_Config* config) {
    destroy_object(config);
}

UMICP_ErrorCode umicp_config_set_version(UMICP_Config* config, const char* version) {
    if (!config || !version) return UMICP_ERROR_INVALID_ARGUMENT;
    config->config.version = version;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_config_set_max_message_size(UMICP_Config* config, size_t size) {
    if (!config) return UMICP_ERROR_INVALID_ARGUMENT;
    config->config.max_message_size = size;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_config_set_binary_enabled(UMICP_Config* config, bool enabled) {
    if (!config) return UMICP_ERROR_INVALID_ARGUMENT;
    config->config.enable_binary = enabled;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_config_set_compression_enabled(UMICP_Config* config, bool enabled) {
    if (!config) return UMICP_ERROR_INVALID_ARGUMENT;
    config->config.enable_compression = enabled;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_config_set_auth_required(UMICP_Config* config, bool required) {
    if (!config) return UMICP_ERROR_INVALID_ARGUMENT;
    config->config.require_auth = required;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_config_set_encryption_required(UMICP_Config* config, bool required) {
    if (!config) return UMICP_ERROR_INVALID_ARGUMENT;
    config->config.require_encryption = required;
    return UMICP_SUCCESS;
}

// Envelope management
UMICP_Envelope* umicp_envelope_create(void) {
    auto* envelope = create_object<UMICP_Envelope>();
    if (!envelope) {
        return nullptr;
    }

    // Initialize with defaults
    envelope->envelope.version = "1.0";
    envelope->envelope.op = umicp::OperationType::CONTROL;
    return envelope;
}

void umicp_envelope_destroy(UMICP_Envelope* envelope) {
    destroy_object(envelope);
}

UMICP_ErrorCode umicp_envelope_set_from(UMICP_Envelope* envelope, const char* from) {
    if (!envelope || !from) return UMICP_ERROR_INVALID_ARGUMENT;
    envelope->envelope.from = from;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_envelope_set_to(UMICP_Envelope* envelope, const char* to) {
    if (!envelope || !to) return UMICP_ERROR_INVALID_ARGUMENT;
    envelope->envelope.to = to;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_envelope_set_operation(UMICP_Envelope* envelope, UMICP_OperationType op) {
    if (!envelope) return UMICP_ERROR_INVALID_ARGUMENT;
    envelope->envelope.op = static_cast<umicp::OperationType>(op);
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_envelope_set_message_id(UMICP_Envelope* envelope, const char* msg_id) {
    if (!envelope || !msg_id) return UMICP_ERROR_INVALID_ARGUMENT;
    envelope->envelope.msg_id = msg_id;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_envelope_serialize_json(UMICP_Envelope* envelope, UMICP_Buffer* output) {
    if (!envelope || !output) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        auto result = umicp::EnvelopeProcessor::serialize(envelope->envelope);
        if (!result.is_success()) {
            return UMICP_ERROR_SERIALIZATION_FAILED;
        }

        const std::string& json_str = *result.value;
        output->buffer.assign(json_str.begin(), json_str.end());
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_SERIALIZATION_FAILED;
    }
}

UMICP_ErrorCode umicp_envelope_deserialize_json(UMICP_Envelope* envelope, const uint8_t* json_data, size_t json_size) {
    if (!envelope || !json_data) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        std::string json_str(reinterpret_cast<const char*>(json_data), json_size);
        auto result = umicp::EnvelopeProcessor::deserialize(json_str);
        if (!result.is_success()) {
            return UMICP_ERROR_SERIALIZATION_FAILED;
        }

        envelope->envelope = *result.value;
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_SERIALIZATION_FAILED;
    }
}

UMICP_ErrorCode umicp_envelope_validate(UMICP_Envelope* envelope) {
    if (!envelope) return UMICP_ERROR_INVALID_ARGUMENT;

    auto result = umicp::EnvelopeProcessor::validate(envelope->envelope);
    return result.is_success() ? UMICP_SUCCESS : UMICP_ERROR_INVALID_ENVELOPE;
}

// Frame management
UMICP_Frame* umicp_frame_create(void) {
    auto* frame = create_object<UMICP_Frame>();
    if (!frame) {
        return nullptr;
    }

    // Initialize with defaults
    frame->frame.header.version = 1;
    frame->frame.header.type = 0;
    frame->frame.header.flags = 0;
    return frame;
}

void umicp_frame_destroy(UMICP_Frame* frame) {
    destroy_object(frame);
}

UMICP_ErrorCode umicp_frame_set_type(UMICP_Frame* frame, uint8_t type) {
    if (!frame) return UMICP_ERROR_INVALID_ARGUMENT;
    frame->frame.header.type = type;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_frame_set_stream_id(UMICP_Frame* frame, uint64_t stream_id) {
    if (!frame) return UMICP_ERROR_INVALID_ARGUMENT;
    frame->frame.header.stream_id = stream_id;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_frame_set_sequence(UMICP_Frame* frame, uint32_t sequence) {
    if (!frame) return UMICP_ERROR_INVALID_ARGUMENT;
    frame->frame.header.sequence = sequence;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_frame_set_flags(UMICP_Frame* frame, uint16_t flags) {
    if (!frame) return UMICP_ERROR_INVALID_ARGUMENT;
    frame->frame.header.flags = flags;
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_frame_set_payload(UMICP_Frame* frame, const uint8_t* data, size_t size) {
    if (!frame || (!data && size > 0)) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        frame->frame.payload.assign(data, data + size);
        frame->frame.header.length = static_cast<uint32_t>(size);
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_MEMORY_ALLOCATION;
    }
}

UMICP_ErrorCode umicp_frame_serialize(UMICP_Frame* frame, UMICP_Buffer* output) {
    if (!frame || !output) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        auto result = umicp::FrameProcessor::serialize(frame->frame);
        if (!result.is_success()) {
            return UMICP_ERROR_SERIALIZATION_FAILED;
        }

        output->buffer = std::move(*result.value);
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_SERIALIZATION_FAILED;
    }
}

UMICP_ErrorCode umicp_frame_deserialize(UMICP_Frame* frame, const uint8_t* data, size_t size) {
    if (!frame || !data) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        umicp::ByteBuffer input(data, data + size);
        auto result = umicp::FrameProcessor::deserialize(input);
        if (!result.is_success()) {
            return UMICP_ERROR_SERIALIZATION_FAILED;
        }

        frame->frame = *result.value;
        return UMICP_SUCCESS;
    } catch (...) {
        return UMICP_ERROR_SERIALIZATION_FAILED;
    }
}

// Protocol management
UMICP_Protocol* umicp_protocol_create(const char* local_id, UMICP_Config* config) {
    if (!local_id) return nullptr;

    auto* protocol = create_object<UMICP_Protocol>();
    if (!protocol) {
        return nullptr;
    }

    try {
        umicp::UMICPConfig protocol_config = config ? config->config : umicp::UMICPConfig{};
        protocol->protocol = std::make_unique<umicp::Protocol>(local_id);
        protocol->protocol->configure(protocol_config);
        return protocol;
    } catch (...) {
        destroy_object(protocol);
        return nullptr;
    }
}

void umicp_protocol_destroy(UMICP_Protocol* protocol) {
    destroy_object(protocol);
}

UMICP_ErrorCode umicp_protocol_send_control(UMICP_Protocol* protocol, const char* to,
                                          const char* command, const char* params_json) {
    if (!protocol || !to || !command) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        std::string params = params_json ? params_json : "";
        auto result = protocol->protocol->send_control(to, umicp::OperationType::CONTROL, command, params);
        return result.is_success() ? UMICP_SUCCESS : UMICP_ERROR_NETWORK_ERROR;
    } catch (...) {
        return UMICP_ERROR_NETWORK_ERROR;
    }
}

UMICP_ErrorCode umicp_protocol_send_data(UMICP_Protocol* protocol, const char* to,
                                       const uint8_t* data, size_t size, const char* metadata_json) {
    if (!protocol || !to || (!data && size > 0)) return UMICP_ERROR_INVALID_ARGUMENT;

    try {
        umicp::ByteBuffer payload(data, data + size);
        umicp::PayloadHint hint;
        // Parse metadata_json if provided (simplified for now)
        auto result = protocol->protocol->send_data(to, payload, hint);
        return result.is_success() ? UMICP_SUCCESS : UMICP_ERROR_NETWORK_ERROR;
    } catch (...) {
        return UMICP_ERROR_NETWORK_ERROR;
    }
}

// Placeholder implementations for other functions
UMICP_ErrorCode umicp_frame_compress(UMICP_Frame* frame, const char* algorithm) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_frame_decompress(UMICP_Frame* frame) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_Security* umicp_security_create(const char* local_id) {
    return nullptr; // Not implemented
}

void umicp_security_destroy(UMICP_Security* security) {
    // Not implemented
}

UMICP_ErrorCode umicp_security_generate_keypair(UMICP_Security* security, UMICP_Buffer* public_key, UMICP_Buffer* private_key) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_security_sign_envelope(UMICP_Security* security, UMICP_Envelope* envelope, const uint8_t* private_key, size_t key_size, UMICP_Buffer* signature) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_security_verify_envelope(UMICP_Security* security, UMICP_Envelope* envelope, const uint8_t* public_key, size_t key_size, const uint8_t* signature, size_t sig_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_security_encrypt_frame(UMICP_Security* security, UMICP_Frame* frame, const uint8_t* key, size_t key_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_security_decrypt_frame(UMICP_Security* security, UMICP_Frame* frame, const uint8_t* key, size_t key_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

bool umicp_security_is_authenticated(UMICP_Security* security) {
    return false;
}

UMICP_Transport* umicp_transport_create_custom(UMICP_TransportType type, const char* endpoint, UMICP_Config* config) {
    // Custom transports are implemented in bindings, not in core
    return nullptr; // Not implemented in core - use bindings
}

void umicp_transport_destroy(UMICP_Transport* transport) {
    // Not implemented
}

UMICP_ErrorCode umicp_transport_connect(UMICP_Transport* transport) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_transport_disconnect(UMICP_Transport* transport) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

bool umicp_transport_is_connected(UMICP_Transport* transport) {
    return false;
}

UMICP_ErrorCode umicp_transport_send(UMICP_Transport* transport, UMICP_Envelope* envelope, const uint8_t* payload, size_t payload_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_protocol_connect(UMICP_Protocol* protocol, UMICP_Transport* transport) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_protocol_disconnect(UMICP_Protocol* protocol) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_protocol_send_ack(UMICP_Protocol* protocol, const char* to, const char* message_id) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_protocol_send_error(UMICP_Protocol* protocol, const char* to, UMICP_ErrorCode error_code, const char* message) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_protocol_process_message(UMICP_Protocol* protocol, const uint8_t* message_data, size_t message_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_matrix_add(const float* a, const float* b, float* result, size_t rows, size_t cols) {
    if (!a || !b || !result) return UMICP_ERROR_INVALID_ARGUMENT;

    auto res = umicp::MatrixOps::add(a, b, result, rows, cols);
    return res.is_success() ? UMICP_SUCCESS : UMICP_ERROR_INVALID_ARGUMENT;
}

UMICP_ErrorCode umicp_matrix_multiply(const float* a, const float* b, float* result, size_t m, size_t n, size_t p) {
    if (!a || !b || !result) return UMICP_ERROR_INVALID_ARGUMENT;

    auto res = umicp::MatrixOps::multiply(a, b, result, m, n, p);
    return res.is_success() ? UMICP_SUCCESS : UMICP_ERROR_INVALID_ARGUMENT;
}

UMICP_ErrorCode umicp_matrix_transpose(const float* input, float* output, size_t rows, size_t cols) {
    if (!input || !output) return UMICP_ERROR_INVALID_ARGUMENT;

    auto res = umicp::MatrixOps::transpose(input, output, rows, cols);
    return res.is_success() ? UMICP_SUCCESS : UMICP_ERROR_INVALID_ARGUMENT;
}

UMICP_ErrorCode umicp_matrix_dot_product(const float* a, const float* b, float* result, size_t size) {
    if (!a || !b || !result) return UMICP_ERROR_INVALID_ARGUMENT;

    auto res = umicp::MatrixOps::dot_product(a, b, result, size);
    return res.is_success() ? UMICP_SUCCESS : UMICP_ERROR_INVALID_ARGUMENT;
}

// Placeholder implementations
UMICP_ErrorCode umicp_serialize_cbor(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_deserialize_cbor(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_serialize_msgpack(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_deserialize_msgpack(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_compress_gzip(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_decompress_gzip(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_compress_brotli(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_decompress_brotli(const uint8_t* data, size_t size, UMICP_Buffer* output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_sha256(const uint8_t* data, size_t size, uint8_t* hash_output) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_generate_random_bytes(uint8_t* buffer, size_t size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_base64_encode(const uint8_t* data, size_t size, char* output, size_t output_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

UMICP_ErrorCode umicp_base64_decode(const char* data, uint8_t* output, size_t output_size) {
    return UMICP_ERROR_NOT_IMPLEMENTED;
}

void umicp_set_allocator(UMICP_AllocFunc alloc_func, UMICP_FreeFunc free_func) {
    g_alloc_func = alloc_func;
    g_free_func = free_func;
}

UMICP_ErrorCode umicp_initialize(void) {
    // Initialize any global state
    return UMICP_SUCCESS;
}

void umicp_cleanup(void) {
    // Clean up global state
    g_alloc_func = nullptr;
    g_free_func = nullptr;
}

const char* umicp_error_string(UMICP_ErrorCode code) {
    switch (code) {
        case UMICP_SUCCESS: return "Success";
        case UMICP_ERROR_INVALID_ENVELOPE: return "Invalid envelope";
        case UMICP_ERROR_INVALID_FRAME: return "Invalid frame";
        case UMICP_ERROR_AUTHENTICATION_FAILED: return "Authentication failed";
        case UMICP_ERROR_DECRYPTION_FAILED: return "Decryption failed";
        case UMICP_ERROR_COMPRESSION_FAILED: return "Compression failed";
        case UMICP_ERROR_SERIALIZATION_FAILED: return "Serialization failed";
        case UMICP_ERROR_NETWORK_ERROR: return "Network error";
        case UMICP_ERROR_TIMEOUT: return "Timeout";
        case UMICP_ERROR_BUFFER_OVERFLOW: return "Buffer overflow";
        case UMICP_ERROR_MEMORY_ALLOCATION: return "Memory allocation failed";
        case UMICP_ERROR_INVALID_ARGUMENT: return "Invalid argument";
        case UMICP_ERROR_NOT_IMPLEMENTED: return "Not implemented";
        default: return "Unknown error";
    }
}

UMICP_ErrorCode umicp_get_version(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 4) return UMICP_ERROR_INVALID_ARGUMENT;

    const char* version = "1.0";
    size_t len = strlen(version);
    if (len >= buffer_size) return UMICP_ERROR_BUFFER_OVERFLOW;

    strcpy(buffer, version);
    return UMICP_SUCCESS;
}

UMICP_ErrorCode umicp_validate_message_format(const uint8_t* data, size_t size) {
    if (!data || size == 0) return UMICP_ERROR_INVALID_ARGUMENT;

    // Basic validation - check if it looks like a valid message
    // This is a placeholder implementation
    return UMICP_SUCCESS;
}
