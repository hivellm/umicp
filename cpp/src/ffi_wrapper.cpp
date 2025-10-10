/**
 * UMICP FFI Wrapper - C API for PHP FFI Integration
 *
 * This file provides C-compatible wrappers around the C++ core implementation
 * to enable PHP FFI integration.
 */

#include "../include/envelope.hpp"
#include "../include/matrix.hpp"
#include "../include/frame.hpp"
#include <cstring>
#include <memory>
#include <string>
#include <cmath>

// Opaque pointer types for FFI
struct UMICPEnvelope {
    umicp::Envelope* impl;
};

struct UMICPMatrix {
    umicp::Matrix* impl;
};

struct UMICPFrame {
    umicp::Frame* impl;
};

// Thread-local storage for string returns
static thread_local std::string tls_string_buffer;

// Export C functions
extern "C" {

// ============================================================================
// Envelope API
// ============================================================================

UMICPEnvelope* umicp_envelope_create() {
    try {
        auto* wrapper = new UMICPEnvelope();
        wrapper->impl = new umicp::Envelope();
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

void umicp_envelope_destroy(UMICPEnvelope* envelope) {
    if (envelope) {
        delete envelope->impl;
        delete envelope;
    }
}

void umicp_envelope_set_from(UMICPEnvelope* envelope, const char* from) {
    if (envelope && from) {
        envelope->impl->setFrom(std::string(from));
    }
}

const char* umicp_envelope_get_from(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;

    tls_string_buffer = envelope->impl->getFrom();
    return tls_string_buffer.c_str();
}

void umicp_envelope_set_to(UMICPEnvelope* envelope, const char* to) {
    if (envelope && to) {
        envelope->impl->setTo(std::string(to));
    }
}

const char* umicp_envelope_get_to(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;

    tls_string_buffer = envelope->impl->getTo();
    return tls_string_buffer.c_str();
}

void umicp_envelope_set_operation(UMICPEnvelope* envelope, int operation) {
    if (envelope) {
        envelope->impl->setOperation(static_cast<umicp::OperationType>(operation));
    }
}

int umicp_envelope_get_operation(UMICPEnvelope* envelope) {
    if (!envelope) return -1;
    return static_cast<int>(envelope->impl->getOperation());
}

void umicp_envelope_set_message_id(UMICPEnvelope* envelope, const char* messageId) {
    if (envelope && messageId) {
        envelope->impl->setMessageId(std::string(messageId));
    }
}

const char* umicp_envelope_get_message_id(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;

    tls_string_buffer = envelope->impl->getMessageId();
    return tls_string_buffer.c_str();
}

void umicp_envelope_set_capabilities(UMICPEnvelope* envelope, const char* json) {
    if (envelope && json) {
        envelope->impl->setCapabilitiesFromJson(std::string(json));
    }
}

const char* umicp_envelope_get_capabilities(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;

    tls_string_buffer = envelope->impl->getCapabilitiesToJson();
    return tls_string_buffer.c_str();
}

const char* umicp_envelope_serialize(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;

    tls_string_buffer = envelope->impl->serialize();
    return tls_string_buffer.c_str();
}

UMICPEnvelope* umicp_envelope_deserialize(const char* json) {
    if (!json) return nullptr;

    try {
        auto* wrapper = new UMICPEnvelope();
        wrapper->impl = new umicp::Envelope();
        wrapper->impl->deserialize(std::string(json));
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

int umicp_envelope_validate(UMICPEnvelope* envelope) {
    if (!envelope) return 0;
    return envelope->impl->validate() ? 1 : 0;
}

const char* umicp_envelope_get_hash(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;

    tls_string_buffer = envelope->impl->getHash();
    return tls_string_buffer.c_str();
}

// ============================================================================
// Matrix API
// ============================================================================

UMICPMatrix* umicp_matrix_create() {
    try {
        auto* wrapper = new UMICPMatrix();
        wrapper->impl = new umicp::Matrix();
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

void umicp_matrix_destroy(UMICPMatrix* matrix) {
    if (matrix) {
        delete matrix->impl;
        delete matrix;
    }
}

double umicp_matrix_dot_product(UMICPMatrix* matrix, const float* a, const float* b, int size) {
    if (!matrix || !a || !b || size <= 0) return 0.0;

    return matrix->impl->dotProduct(a, b, size);
}

double umicp_matrix_cosine_similarity(UMICPMatrix* matrix, const float* a, const float* b, int size) {
    if (!matrix || !a || !b || size <= 0) return 0.0;

    return matrix->impl->cosineSimilarity(a, b, size);
}

void umicp_matrix_vector_add(UMICPMatrix* matrix, const float* a, const float* b, float* result, int size) {
    if (!matrix || !a || !b || !result || size <= 0) return;

    matrix->impl->vectorAdd(a, b, result, size);
}

void umicp_matrix_vector_subtract(UMICPMatrix* matrix, const float* a, const float* b, float* result, int size) {
    if (!matrix || !a || !b || !result || size <= 0) return;

    matrix->impl->vectorSubtract(a, b, result, size);
}

void umicp_matrix_vector_scale(UMICPMatrix* matrix, const float* vector, float scalar, float* result, int size) {
    if (!matrix || !vector || !result || size <= 0) return;

    for (int i = 0; i < size; i++) {
        result[i] = vector[i] * scalar;
    }
}

double umicp_matrix_vector_magnitude(UMICPMatrix* matrix, const float* vector, int size) {
    if (!matrix || !vector || size <= 0) return 0.0;

    return matrix->impl->vectorMagnitude(vector, size);
}

void umicp_matrix_vector_normalize(UMICPMatrix* matrix, const float* vector, float* result, int size) {
    if (!matrix || !vector || !result || size <= 0) return;

    double magnitude = matrix->impl->vectorMagnitude(vector, size);
    if (magnitude > 0.0) {
        for (int i = 0; i < size; i++) {
            result[i] = vector[i] / magnitude;
        }
    }
}

void umicp_matrix_multiply(UMICPMatrix* matrix, const float* a, const float* b, float* result, int m, int n, int p) {
    if (!matrix || !a || !b || !result || m <= 0 || n <= 0 || p <= 0) return;

    matrix->impl->matrixMultiply(a, b, result, m, n, p);
}

void umicp_matrix_transpose(UMICPMatrix* matrix, const float* input, float* result, int rows, int cols) {
    if (!matrix || !input || !result || rows <= 0 || cols <= 0) return;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[j * rows + i] = input[i * cols + j];
        }
    }
}

// ============================================================================
// Frame API
// ============================================================================

UMICPFrame* umicp_frame_create() {
    try {
        auto* wrapper = new UMICPFrame();
        wrapper->impl = new umicp::Frame();
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

void umicp_frame_destroy(UMICPFrame* frame) {
    if (frame) {
        delete frame->impl;
        delete frame;
    }
}

void umicp_frame_set_type(UMICPFrame* frame, int type) {
    if (frame) {
        frame->impl->setType(type);
    }
}

int umicp_frame_get_type(UMICPFrame* frame) {
    if (!frame) return -1;
    return frame->impl->getType();
}

void umicp_frame_set_stream_id(UMICPFrame* frame, int streamId) {
    if (frame) {
        frame->impl->setStreamId(streamId);
    }
}

int umicp_frame_get_stream_id(UMICPFrame* frame) {
    if (!frame) return -1;
    return frame->impl->getStreamId();
}

void umicp_frame_set_sequence(UMICPFrame* frame, int sequence) {
    if (frame) {
        frame->impl->setSequence(sequence);
    }
}

int umicp_frame_get_sequence(UMICPFrame* frame) {
    if (!frame) return -1;
    return frame->impl->getSequence();
}

void umicp_frame_set_flags(UMICPFrame* frame, int flags) {
    if (frame) {
        frame->impl->setFlags(flags);
    }
}

int umicp_frame_get_flags(UMICPFrame* frame) {
    if (!frame) return -1;
    return frame->impl->getFlags();
}

const char* umicp_frame_serialize(UMICPFrame* frame) {
    if (!frame) return nullptr;

    tls_string_buffer = frame->impl->serialize();
    return tls_string_buffer.c_str();
}

UMICPFrame* umicp_frame_deserialize(const char* data) {
    if (!data) return nullptr;

    try {
        auto* wrapper = new UMICPFrame();
        wrapper->impl = new umicp::Frame();
        wrapper->impl->deserialize(std::string(data));
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

// ============================================================================
// Version and Information
// ============================================================================

const char* umicp_get_version() {
    tls_string_buffer = "1.0.0";
    return tls_string_buffer.c_str();
}

const char* umicp_get_build_info() {
    tls_string_buffer = "UMICP Core C++ with FFI wrapper - Built for PHP";
    return tls_string_buffer.c_str();
}

} // extern "C"

