/* UMICP FFI Header - Clean C only for PHP */

typedef struct UMICP_Envelope UMICP_Envelope;
typedef struct UMICP_Matrix UMICP_Matrix;
typedef struct UMICP_Frame UMICP_Frame;

/* Envelope Functions */
UMICP_Envelope* umicp_envelope_create(void);
void umicp_envelope_destroy(UMICP_Envelope* envelope);
int umicp_envelope_set_from(UMICP_Envelope* envelope, const char* from);
int umicp_envelope_set_to(UMICP_Envelope* envelope, const char* to);
int umicp_envelope_set_message_id(UMICP_Envelope* envelope, const char* message_id);
int umicp_envelope_set_operation(UMICP_Envelope* envelope, int operation);
int umicp_envelope_set_capabilities(UMICP_Envelope* envelope, const char* capabilities);
char* umicp_envelope_get_capabilities(UMICP_Envelope* envelope);
char* umicp_envelope_get_from(UMICP_Envelope* envelope);
char* umicp_envelope_get_to(UMICP_Envelope* envelope);
char* umicp_envelope_get_message_id(UMICP_Envelope* envelope);
int umicp_envelope_serialize(UMICP_Envelope* envelope, unsigned char* buffer, int buffer_size);
char* umicp_php_envelope_to_json(UMICP_Envelope* envelope);
UMICP_Envelope* umicp_php_envelope_from_json(const char* json);
char* umicp_php_envelope_compute_hash(UMICP_Envelope* envelope);
int umicp_envelope_validate(UMICP_Envelope* envelope);

/* Matrix Functions */
UMICP_Matrix* umicp_matrix_create(const float* data, int rows, int cols);
void umicp_matrix_destroy(UMICP_Matrix* matrix);
float umicp_php_matrix_dot_product(const float* a, const float* b, int size);
float umicp_php_matrix_cosine_similarity(const float* a, const float* b, int size);
float* umicp_php_matrix_add(const float* a, const float* b, int size);
float* umicp_php_matrix_scale(const float* vector, float scalar, int size);
float umicp_php_matrix_magnitude(const float* vector, int size);
float* umicp_php_matrix_normalize(const float* vector, int size);

/* Frame Functions */
UMICP_Frame* umicp_frame_create(void);
void umicp_frame_destroy(UMICP_Frame* frame);
int umicp_frame_set_type(UMICP_Frame* frame, int type);
int umicp_frame_set_payload(UMICP_Frame* frame, const unsigned char* data, int size);
unsigned char* umicp_php_frame_get_payload(UMICP_Frame* frame, int* out_size);

/* Memory Management */
void umicp_php_free_string(char* str);
void umicp_php_free_float_array(float* array);
void umicp_php_free_byte_array(unsigned char* array);
