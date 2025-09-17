/**
 * UMICP (Universal Matrix Intelligent Communication Protocol) - C API
 * C-compatible interface for FFI bindings
 *
 * This header provides the C API that can be called from other languages
 */

#ifndef UMICP_H
#define UMICP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque types for C API
typedef struct UMICP_Protocol UMICP_Protocol;
typedef struct UMICP_Envelope UMICP_Envelope;
typedef struct UMICP_Frame UMICP_Frame;
typedef struct UMICP_Security UMICP_Security;
typedef struct UMICP_Transport UMICP_Transport;
typedef struct UMICP_Config UMICP_Config;
typedef struct UMICP_Buffer UMICP_Buffer;

// Error codes
typedef enum {
    UMICP_SUCCESS = 0,
    UMICP_ERROR_INVALID_ENVELOPE = 1,
    UMICP_ERROR_INVALID_FRAME = 2,
    UMICP_ERROR_AUTHENTICATION_FAILED = 3,
    UMICP_ERROR_DECRYPTION_FAILED = 4,
    UMICP_ERROR_COMPRESSION_FAILED = 5,
    UMICP_ERROR_SERIALIZATION_FAILED = 6,
    UMICP_ERROR_NETWORK_ERROR = 7,
    UMICP_ERROR_TIMEOUT = 8,
    UMICP_ERROR_BUFFER_OVERFLOW = 9,
    UMICP_ERROR_MEMORY_ALLOCATION = 10,
    UMICP_ERROR_INVALID_ARGUMENT = 11,
    UMICP_ERROR_NOT_IMPLEMENTED = 12
} UMICP_ErrorCode;

// Operation types
typedef enum {
    UMICP_OP_CONTROL = 0,
    UMICP_OP_DATA = 1,
    UMICP_OP_ACK = 2,
    UMICP_OP_ERROR = 3
} UMICP_OperationType;

// Content types
typedef enum {
    UMICP_CONTENT_JSON = 0,
    UMICP_CONTENT_CBOR = 1,
    UMICP_CONTENT_MSGPACK = 2
} UMICP_ContentType;

// Transport types
typedef enum {
    UMICP_TRANSPORT_MATRIX = 0,
    UMICP_TRANSPORT_DIRECT = 1,
    UMICP_TRANSPORT_CUSTOM = 2
} UMICP_TransportType;

// Buffer management
UMICP_Buffer* umicp_buffer_create(size_t capacity);
void umicp_buffer_destroy(UMICP_Buffer* buffer);
uint8_t* umicp_buffer_data(UMICP_Buffer* buffer);
size_t umicp_buffer_size(UMICP_Buffer* buffer);
size_t umicp_buffer_capacity(UMICP_Buffer* buffer);
UMICP_ErrorCode umicp_buffer_resize(UMICP_Buffer* buffer, size_t new_size);
UMICP_ErrorCode umicp_buffer_append(UMICP_Buffer* buffer, const uint8_t* data, size_t size);

// Configuration management
UMICP_Config* umicp_config_create(void);
void umicp_config_destroy(UMICP_Config* config);
UMICP_ErrorCode umicp_config_set_version(UMICP_Config* config, const char* version);
UMICP_ErrorCode umicp_config_set_max_message_size(UMICP_Config* config, size_t size);
UMICP_ErrorCode umicp_config_set_binary_enabled(UMICP_Config* config, bool enabled);
UMICP_ErrorCode umicp_config_set_compression_enabled(UMICP_Config* config, bool enabled);
UMICP_ErrorCode umicp_config_set_auth_required(UMICP_Config* config, bool required);
UMICP_ErrorCode umicp_config_set_encryption_required(UMICP_Config* config, bool required);

// Envelope management
UMICP_Envelope* umicp_envelope_create(void);
void umicp_envelope_destroy(UMICP_Envelope* envelope);
UMICP_ErrorCode umicp_envelope_set_from(UMICP_Envelope* envelope, const char* from);
UMICP_ErrorCode umicp_envelope_set_to(UMICP_Envelope* envelope, const char* to);
UMICP_ErrorCode umicp_envelope_set_operation(UMICP_Envelope* envelope, UMICP_OperationType op);
UMICP_ErrorCode umicp_envelope_set_message_id(UMICP_Envelope* envelope, const char* msg_id);
UMICP_ErrorCode umicp_envelope_serialize_json(UMICP_Envelope* envelope, UMICP_Buffer* output);
UMICP_ErrorCode umicp_envelope_deserialize_json(UMICP_Envelope* envelope, const uint8_t* json_data, size_t json_size);
UMICP_ErrorCode umicp_envelope_validate(UMICP_Envelope* envelope);

// Frame management
UMICP_Frame* umicp_frame_create(void);
void umicp_frame_destroy(UMICP_Frame* frame);
UMICP_ErrorCode umicp_frame_set_type(UMICP_Frame* frame, uint8_t type);
UMICP_ErrorCode umicp_frame_set_stream_id(UMICP_Frame* frame, uint64_t stream_id);
UMICP_ErrorCode umicp_frame_set_sequence(UMICP_Frame* frame, uint32_t sequence);
UMICP_ErrorCode umicp_frame_set_flags(UMICP_Frame* frame, uint16_t flags);
UMICP_ErrorCode umicp_frame_set_payload(UMICP_Frame* frame, const uint8_t* data, size_t size);
UMICP_ErrorCode umicp_frame_serialize(UMICP_Frame* frame, UMICP_Buffer* output);
UMICP_ErrorCode umicp_frame_deserialize(UMICP_Frame* frame, const uint8_t* data, size_t size);
UMICP_ErrorCode umicp_frame_compress(UMICP_Frame* frame, const char* algorithm);
UMICP_ErrorCode umicp_frame_decompress(UMICP_Frame* frame);

// Security management
UMICP_Security* umicp_security_create(const char* local_id);
void umicp_security_destroy(UMICP_Security* security);
UMICP_ErrorCode umicp_security_generate_keypair(UMICP_Security* security, UMICP_Buffer* public_key, UMICP_Buffer* private_key);
UMICP_ErrorCode umicp_security_sign_envelope(UMICP_Security* security, UMICP_Envelope* envelope, const uint8_t* private_key, size_t key_size, UMICP_Buffer* signature);
UMICP_ErrorCode umicp_security_verify_envelope(UMICP_Security* security, UMICP_Envelope* envelope, const uint8_t* public_key, size_t key_size, const uint8_t* signature, size_t sig_size);
UMICP_ErrorCode umicp_security_encrypt_frame(UMICP_Security* security, UMICP_Frame* frame, const uint8_t* key, size_t key_size);
UMICP_ErrorCode umicp_security_decrypt_frame(UMICP_Security* security, UMICP_Frame* frame, const uint8_t* key, size_t key_size);
bool umicp_security_is_authenticated(UMICP_Security* security);

// Transport management
UMICP_Transport* umicp_transport_create_custom(UMICP_TransportType type, const char* endpoint, UMICP_Config* config);
void umicp_transport_destroy(UMICP_Transport* transport);
UMICP_ErrorCode umicp_transport_connect(UMICP_Transport* transport);
UMICP_ErrorCode umicp_transport_disconnect(UMICP_Transport* transport);
bool umicp_transport_is_connected(UMICP_Transport* transport);
UMICP_ErrorCode umicp_transport_send(UMICP_Transport* transport, UMICP_Envelope* envelope, const uint8_t* payload, size_t payload_size);

// Protocol management (main orchestrator)
UMICP_Protocol* umicp_protocol_create(const char* local_id, UMICP_Config* config);
void umicp_protocol_destroy(UMICP_Protocol* protocol);
UMICP_ErrorCode umicp_protocol_connect(UMICP_Protocol* protocol, UMICP_Transport* transport);
UMICP_ErrorCode umicp_protocol_disconnect(UMICP_Protocol* protocol);
UMICP_ErrorCode umicp_protocol_send_control(UMICP_Protocol* protocol, const char* to, const char* command, const char* params_json);
UMICP_ErrorCode umicp_protocol_send_data(UMICP_Protocol* protocol, const char* to, const uint8_t* data, size_t size, const char* metadata_json);
UMICP_ErrorCode umicp_protocol_send_ack(UMICP_Protocol* protocol, const char* to, const char* message_id);
UMICP_ErrorCode umicp_protocol_send_error(UMICP_Protocol* protocol, const char* to, UMICP_ErrorCode error_code, const char* message);
UMICP_ErrorCode umicp_protocol_process_message(UMICP_Protocol* protocol, const uint8_t* message_data, size_t message_size);

// Utility functions
const char* umicp_error_string(UMICP_ErrorCode code);
UMICP_ErrorCode umicp_get_version(char* buffer, size_t buffer_size);
UMICP_ErrorCode umicp_validate_message_format(const uint8_t* data, size_t size);

// Matrix operations (core functionality)
UMICP_ErrorCode umicp_matrix_add(const float* a, const float* b, float* result, size_t rows, size_t cols);
UMICP_ErrorCode umicp_matrix_multiply(const float* a, const float* b, float* result, size_t m, size_t n, size_t p);
UMICP_ErrorCode umicp_matrix_transpose(const float* input, float* output, size_t rows, size_t cols);
UMICP_ErrorCode umicp_matrix_dot_product(const float* a, const float* b, float* result, size_t size);
UMICP_ErrorCode umicp_matrix_normalize(float* matrix, size_t rows, size_t cols);

// Serialization utilities
UMICP_ErrorCode umicp_serialize_cbor(const uint8_t* data, size_t size, UMICP_Buffer* output);
UMICP_ErrorCode umicp_deserialize_cbor(const uint8_t* data, size_t size, UMICP_Buffer* output);
UMICP_ErrorCode umicp_serialize_msgpack(const uint8_t* data, size_t size, UMICP_Buffer* output);
UMICP_ErrorCode umicp_deserialize_msgpack(const uint8_t* data, size_t size, UMICP_Buffer* output);

// Compression utilities
UMICP_ErrorCode umicp_compress_gzip(const uint8_t* data, size_t size, UMICP_Buffer* output);
UMICP_ErrorCode umicp_decompress_gzip(const uint8_t* data, size_t size, UMICP_Buffer* output);
UMICP_ErrorCode umicp_compress_brotli(const uint8_t* data, size_t size, UMICP_Buffer* output);
UMICP_ErrorCode umicp_decompress_brotli(const uint8_t* data, size_t size, UMICP_Buffer* output);

// Hash and crypto utilities
UMICP_ErrorCode umicp_sha256(const uint8_t* data, size_t size, uint8_t* hash_output);
UMICP_ErrorCode umicp_generate_random_bytes(uint8_t* buffer, size_t size);
UMICP_ErrorCode umicp_base64_encode(const uint8_t* data, size_t size, char* output, size_t output_size);
UMICP_ErrorCode umicp_base64_decode(const char* data, uint8_t* output, size_t output_size);

// Memory management helpers
typedef void* (*UMICP_AllocFunc)(size_t size);
typedef void (*UMICP_FreeFunc)(void* ptr);

void umicp_set_allocator(UMICP_AllocFunc alloc_func, UMICP_FreeFunc free_func);

// Library initialization/cleanup
UMICP_ErrorCode umicp_initialize(void);
void umicp_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // UMICP_H
