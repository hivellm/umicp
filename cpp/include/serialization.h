/**
 * UMICP Serialization Header
 * JSON and binary serialization support
 */

#ifndef UMICP_SERIALIZATION_H
#define UMICP_SERIALIZATION_H

#include "umicp_types.h"
#include <string>

namespace umicp {

// JSON serialization
class JsonSerializer {
public:
    static Result<std::string> serialize_envelope(const Envelope& envelope);
    static Result<Envelope> deserialize_envelope(const std::string& json_str);
    static Result<std::string> canonicalize_json(const std::string& json_str);
};

// Binary serialization
class BinarySerializer {
public:
    static Result<ByteBuffer> serialize_frame(const Frame& frame);
    static Result<Frame> deserialize_frame(const ByteBuffer& data);
    static Result<ByteBuffer> serialize_envelope_cbor(const Envelope& envelope);
    static Result<Envelope> deserialize_envelope_cbor(const ByteBuffer& data);

    // CBOR-specific functions
    static Result<ByteBuffer> encode_cbor(const JsonObject& data);
    static Result<JsonObject> decode_cbor(const ByteBuffer& data);

    // MessagePack-specific functions
    static Result<ByteBuffer> encode_msgpack(const JsonObject& data);
    static Result<JsonObject> decode_msgpack(const ByteBuffer& data);

private:
    // Helper function for MessagePack string decoding
    static Result<std::string> decode_msgpack_string(const ByteBuffer& data, size_t& pos);
};

// Hash utilities
class HashUtils {
public:
    static Result<std::string> sha256_hex(const ByteBuffer& data);
    static Result<std::string> sha256_hex(const std::string& str);
    static Result<ByteBuffer> sha256_raw(const ByteBuffer& data);
};

// Base64 utilities
class Base64Utils {
public:
    static Result<std::string> encode(const ByteBuffer& data);
    static Result<ByteBuffer> decode(const std::string& encoded);
};

} // namespace umicp

#endif // UMICP_SERIALIZATION_H
