#ifndef UMICP_FFI_H
#define UMICP_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Opaque Types
 * ============================================================================ */

typedef struct UMICPEnvelope UMICPEnvelope;
typedef struct UMICPMatrix UMICPMatrix;
typedef struct UMICPFrame UMICPFrame;

/* ============================================================================
 * Envelope Functions
 * ============================================================================ */

/**
 * Create a new envelope instance
 * @return Pointer to envelope or NULL on failure
 */
UMICPEnvelope* umicp_envelope_create(void);

/**
 * Destroy envelope instance
 * @param envelope Envelope to destroy
 */
void umicp_envelope_destroy(UMICPEnvelope* envelope);

/**
 * Set envelope sender
 * @param envelope Target envelope
 * @param from Sender identifier (null-terminated string)
 */
void umicp_envelope_set_from(UMICPEnvelope* envelope, const char* from);

/**
 * Get envelope sender
 * @param envelope Source envelope
 * @return Sender identifier (caller must not free, thread-local storage)
 */
const char* umicp_envelope_get_from(UMICPEnvelope* envelope);

/**
 * Set envelope recipient
 * @param envelope Target envelope
 * @param to Recipient identifier
 */
void umicp_envelope_set_to(UMICPEnvelope* envelope, const char* to);

/**
 * Get envelope recipient
 * @param envelope Source envelope
 * @return Recipient identifier
 */
const char* umicp_envelope_get_to(UMICPEnvelope* envelope);

/**
 * Set operation type
 * @param envelope Target envelope
 * @param operation Operation type (0=CONTROL, 1=DATA, 2=ACK, 3=ERROR, 4=REQUEST, 5=RESPONSE)
 */
void umicp_envelope_set_operation(UMICPEnvelope* envelope, int operation);

/**
 * Get operation type
 * @param envelope Source envelope
 * @return Operation type as integer
 */
int umicp_envelope_get_operation(UMICPEnvelope* envelope);

/**
 * Set message identifier
 * @param envelope Target envelope
 * @param messageId Message ID
 */
void umicp_envelope_set_message_id(UMICPEnvelope* envelope, const char* messageId);

/**
 * Get message identifier
 * @param envelope Source envelope
 * @return Message ID
 */
const char* umicp_envelope_get_message_id(UMICPEnvelope* envelope);

/**
 * Set capabilities (as JSON string)
 * @param envelope Target envelope
 * @param json JSON string with capabilities
 */
void umicp_envelope_set_capabilities(UMICPEnvelope* envelope, const char* json);

/**
 * Get capabilities (as JSON string)
 * @param envelope Source envelope
 * @return JSON string with capabilities (thread-local storage)
 */
const char* umicp_envelope_get_capabilities(UMICPEnvelope* envelope);

/**
 * Serialize envelope to JSON
 * @param envelope Source envelope
 * @return JSON string (thread-local storage, caller must not free)
 */
const char* umicp_envelope_serialize(UMICPEnvelope* envelope);

/**
 * Deserialize envelope from JSON
 * @param json JSON string
 * @return New envelope instance or NULL on failure
 */
UMICPEnvelope* umicp_envelope_deserialize(const char* json);

/**
 * Validate envelope
 * @param envelope Envelope to validate
 * @return 1 if valid, 0 if invalid
 */
int umicp_envelope_validate(UMICPEnvelope* envelope);

/**
 * Get envelope hash
 * @param envelope Source envelope
 * @return Hash string (thread-local storage)
 */
const char* umicp_envelope_get_hash(UMICPEnvelope* envelope);

/* ============================================================================
 * Matrix Functions
 * ============================================================================ */

/**
 * Create matrix instance
 * @return Pointer to matrix or NULL on failure
 */
UMICPMatrix* umicp_matrix_create(void);

/**
 * Destroy matrix instance
 * @param matrix Matrix to destroy
 */
void umicp_matrix_destroy(UMICPMatrix* matrix);

/**
 * Calculate dot product
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return Dot product result
 */
double umicp_matrix_dot_product(UMICPMatrix* matrix, const float* a, const float* b, int size);

/**
 * Calculate cosine similarity
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return Cosine similarity (-1 to 1)
 */
double umicp_matrix_cosine_similarity(UMICPMatrix* matrix, const float* a, const float* b, int size);

/**
 * Vector addition
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param result Result vector (must be pre-allocated)
 * @param size Vector size
 */
void umicp_matrix_vector_add(UMICPMatrix* matrix, const float* a, const float* b, float* result, int size);

/**
 * Vector subtraction
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param result Result vector (must be pre-allocated)
 * @param size Vector size
 */
void umicp_matrix_vector_subtract(UMICPMatrix* matrix, const float* a, const float* b, float* result, int size);

/**
 * Vector scaling
 * @param matrix Matrix instance
 * @param vector Input vector
 * @param scalar Scalar value
 * @param result Result vector (must be pre-allocated)
 * @param size Vector size
 */
void umicp_matrix_vector_scale(UMICPMatrix* matrix, const float* vector, float scalar, float* result, int size);

/**
 * Vector magnitude (L2 norm)
 * @param matrix Matrix instance
 * @param vector Input vector
 * @param size Vector size
 * @return Magnitude
 */
double umicp_matrix_vector_magnitude(UMICPMatrix* matrix, const float* vector, int size);

/**
 * Normalize vector to unit length
 * @param matrix Matrix instance
 * @param vector Input vector
 * @param result Normalized vector (must be pre-allocated)
 * @param size Vector size
 */
void umicp_matrix_vector_normalize(UMICPMatrix* matrix, const float* vector, float* result, int size);

/**
 * Matrix multiplication (A * B = C)
 * @param matrix Matrix instance
 * @param a First matrix (flat array, row-major)
 * @param b Second matrix (flat array, row-major)
 * @param result Result matrix (must be pre-allocated)
 * @param m Rows of A
 * @param n Columns of A / Rows of B
 * @param p Columns of B
 */
void umicp_matrix_multiply(UMICPMatrix* matrix, const float* a, const float* b, float* result, int m, int n, int p);

/**
 * Matrix transpose
 * @param matrix Matrix instance
 * @param input Input matrix (flat array, row-major)
 * @param result Result matrix (must be pre-allocated)
 * @param rows Number of rows
 * @param cols Number of columns
 */
void umicp_matrix_transpose(UMICPMatrix* matrix, const float* input, float* result, int rows, int cols);

/* ============================================================================
 * Frame Functions
 * ============================================================================ */

/**
 * Create frame instance
 * @return Pointer to frame or NULL on failure
 */
UMICPFrame* umicp_frame_create(void);

/**
 * Destroy frame instance
 * @param frame Frame to destroy
 */
void umicp_frame_destroy(UMICPFrame* frame);

/**
 * Set frame type
 * @param frame Target frame
 * @param type Frame type
 */
void umicp_frame_set_type(UMICPFrame* frame, int type);

/**
 * Get frame type
 * @param frame Source frame
 * @return Frame type
 */
int umicp_frame_get_type(UMICPFrame* frame);

/**
 * Set stream ID
 * @param frame Target frame
 * @param streamId Stream identifier
 */
void umicp_frame_set_stream_id(UMICPFrame* frame, int streamId);

/**
 * Get stream ID
 * @param frame Source frame
 * @return Stream identifier
 */
int umicp_frame_get_stream_id(UMICPFrame* frame);

/**
 * Set sequence number
 * @param frame Target frame
 * @param sequence Sequence number
 */
void umicp_frame_set_sequence(UMICPFrame* frame, int sequence);

/**
 * Get sequence number
 * @param frame Source frame
 * @return Sequence number
 */
int umicp_frame_get_sequence(UMICPFrame* frame);

/**
 * Set frame flags
 * @param frame Target frame
 * @param flags Frame flags
 */
void umicp_frame_set_flags(UMICPFrame* frame, int flags);

/**
 * Get frame flags
 * @param frame Source frame
 * @return Frame flags
 */
int umicp_frame_get_flags(UMICPFrame* frame);

/**
 * Serialize frame
 * @param frame Source frame
 * @return Serialized data (thread-local storage)
 */
const char* umicp_frame_serialize(UMICPFrame* frame);

/**
 * Deserialize frame
 * @param data Serialized data
 * @return New frame instance or NULL on failure
 */
UMICPFrame* umicp_frame_deserialize(const char* data);

/* ============================================================================
 * Version and Information
 * ============================================================================ */

/**
 * Get UMICP version string
 * @return Version string (e.g., "1.0.0")
 */
const char* umicp_get_version(void);

/**
 * Get build information
 * @return Build info string
 */
const char* umicp_get_build_info(void);

#ifdef __cplusplus
}
#endif

#endif /* UMICP_FFI_H */

