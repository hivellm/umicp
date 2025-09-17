/**
 * UMICP Serialization Implementation
 * JSON and binary serialization support
 */

#include "serialization.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <openssl/evp.h>
#include <cbor.h>

namespace umicp {

// ===============================================
// JSON Serialization
// ===============================================

Result<std::string> JsonSerializer::serialize_envelope(const Envelope& envelope) {
    // Validate envelope
    if (envelope.msg_id.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "Message ID cannot be empty");
    }

    if (envelope.from.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "From field cannot be empty");
    }

    if (envelope.to.empty()) {
        return Result<std::string>(ErrorCode::INVALID_ARGUMENT, "To field cannot be empty");
    }

    std::ostringstream json;
    json << "{";
    json << "\"v\":\"" << envelope.version << "\",";
    json << "\"msg_id\":\"" << envelope.msg_id << "\",";
    json << "\"ts\":\"" << envelope.ts << "\",";
    json << "\"from\":\"" << envelope.from << "\",";
    json << "\"to\":\"" << envelope.to << "\",";
    json << "\"op\":" << static_cast<int>(envelope.op);

    // Add optional fields
    if (envelope.capabilities && !envelope.capabilities->empty()) {
        json << ",\"capabilities\":{";
        bool first = true;
        for (const auto& [key, value] : *envelope.capabilities) {
            if (!first) json << ",";
            json << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        json << "}";
    }

    if (envelope.schema_uri) {
        json << ",\"schema_uri\":\"" << *envelope.schema_uri << "\"";
    }

    if (envelope.accept && !envelope.accept->empty()) {
        json << ",\"accept\":[";
        for (size_t i = 0; i < envelope.accept->size(); ++i) {
            if (i > 0) json << ",";
            json << "\"" << (*envelope.accept)[i] << "\"";
        }
        json << "]";
    }

    if (envelope.payload_hint) {
        json << ",\"payload_hint\":{";
        // bool first = true; // TODO: Use this when implementing PayloadHint serialization
        // TODO: Serialize PayloadHint struct properly
        // for (const auto& [key, value] : *envelope.payload_hint) {
        //     if (!first) json << ",";
        //     json << "\"" << key << "\":\"" << value << "\"";
        //     first = false;
        // }
        json << "}";
    }

    if (envelope.payload_refs && !envelope.payload_refs->empty()) {
        json << ",\"payload_refs\":[";
        for (size_t i = 0; i < envelope.payload_refs->size(); ++i) {
            if (i > 0) json << ",";
            json << "{";
            bool first = true;
            for (const auto& [key, value] : (*envelope.payload_refs)[i]) {
                if (!first) json << ",";
                json << "\"" << key << "\":\"" << value << "\"";
                first = false;
            }
            json << "}";
        }
        json << "]";
    }

    json << "}";

    return Result<std::string>(json.str());
}

Result<Envelope> JsonSerializer::deserialize_envelope(const std::string& json_str) {
    // Validate input
    if (json_str.empty()) {
        return Result<Envelope>(ErrorCode::INVALID_ARGUMENT, "JSON string cannot be empty");
    }

    // Basic JSON validation - check for basic JSON structure
    if (json_str.front() != '{' || json_str.back() != '}') {
        return Result<Envelope>(ErrorCode::SERIALIZATION_FAILED, "Invalid JSON format");
    }

    // Basic JSON parsing for MVP
    // In production, would use a proper JSON library like nlohmann/json
    Envelope envelope;

    // Extract version
    auto v_pos = json_str.find("\"v\":\"");
    if (v_pos != std::string::npos) {
        auto start = v_pos + 5;
        auto end = json_str.find("\"", start);
        if (end != std::string::npos) {
            envelope.version = json_str.substr(start, end - start);
        }
    }

    // Extract msg_id
    auto msg_id_pos = json_str.find("\"msg_id\":\"");
    if (msg_id_pos != std::string::npos) {
        auto start = msg_id_pos + 10;
        auto end = json_str.find("\"", start);
        if (end != std::string::npos) {
            envelope.msg_id = json_str.substr(start, end - start);
        }
    }

    // Extract from
    auto from_pos = json_str.find("\"from\":\"");
    if (from_pos != std::string::npos) {
        auto start = from_pos + 8;
        auto end = json_str.find("\"", start);
        if (end != std::string::npos) {
            envelope.from = json_str.substr(start, end - start);
        }
    }

    // Extract to
    auto to_pos = json_str.find("\"to\":\"");
    if (to_pos != std::string::npos) {
        auto start = to_pos + 6;
        auto end = json_str.find("\"", start);
        if (end != std::string::npos) {
            envelope.to = json_str.substr(start, end - start);
        }
    }

    // Extract timestamp
    auto ts_pos = json_str.find("\"ts\":\"");
    if (ts_pos != std::string::npos) {
        auto start = ts_pos + 6;
        auto end = json_str.find("\"", start);
        if (end != std::string::npos) {
            envelope.ts = json_str.substr(start, end - start);
        }
    }

    // Extract operation
    auto op_pos = json_str.find("\"op\":");
    if (op_pos != std::string::npos) {
        auto start = op_pos + 5;
        auto end = json_str.find_first_of(",}", start);
        if (end != std::string::npos) {
            auto op_str = json_str.substr(start, end - start);
            try {
                envelope.op = static_cast<OperationType>(std::stoi(op_str));
            } catch (const std::exception&) {
                return Result<Envelope>(ErrorCode::SERIALIZATION_FAILED, "Invalid operation type");
            }
        }
    }

    // Validate required fields
    if (envelope.msg_id.empty()) {
        return Result<Envelope>(ErrorCode::SERIALIZATION_FAILED, "Message ID is required");
    }

    if (envelope.from.empty()) {
        return Result<Envelope>(ErrorCode::SERIALIZATION_FAILED, "From field is required");
    }

    if (envelope.to.empty()) {
        return Result<Envelope>(ErrorCode::SERIALIZATION_FAILED, "To field is required");
    }

    return Result<Envelope>(envelope);
}

Result<std::string> JsonSerializer::canonicalize_json(const std::string& json_str) {
    // Basic canonicalization - in production would use JCS (JSON Canonicalization Scheme)
    // For MVP, just remove unnecessary whitespace
    std::string result;
    result.reserve(json_str.size());

    bool in_string = false;
    bool escaped = false;

    for (char c : json_str) {
        if (escaped) {
            result += c;
            escaped = false;
            continue;
        }

        if (c == '\\' && in_string) {
            escaped = true;
            result += c;
            continue;
        }

        if (c == '"') {
            in_string = !in_string;
            result += c;
            continue;
        }

        if (!in_string && (c == ' ' || c == '\n' || c == '\r' || c == '\t')) {
            continue; // Skip whitespace outside strings
        }

        result += c;
    }

    return Result<std::string>(result);
}

// ===============================================
// Binary Serialization
// ===============================================

Result<ByteBuffer> BinarySerializer::serialize_frame(const Frame& frame) {
    // Validate frame type
    if (frame.header.type > 3) { // Maximum valid OperationType is 3 (ERROR)
        return Result<ByteBuffer>(ErrorCode::INVALID_ARGUMENT, "Invalid frame type");
    }

    ByteBuffer data;
    data.reserve(UMICP_FRAME_HEADER_SIZE + frame.payload.size());

    // Serialize header (16 bytes)
    data.push_back(frame.header.version);
    data.push_back(frame.header.type);

    // Flags (2 bytes, little-endian)
    data.push_back(frame.header.flags & 0xFF);
    data.push_back((frame.header.flags >> 8) & 0xFF);

    // Stream ID (8 bytes, little-endian)
    uint64_t stream_id = frame.header.stream_id;
    for (int i = 0; i < 8; ++i) {
        data.push_back((stream_id >> (i * 8)) & 0xFF);
    }

    // Sequence (4 bytes, little-endian)
    uint32_t sequence = frame.header.sequence;
    for (int i = 0; i < 4; ++i) {
        data.push_back((sequence >> (i * 8)) & 0xFF);
    }

    // Length (4 bytes, little-endian)
    uint32_t length = static_cast<uint32_t>(frame.payload.size());
    for (int i = 0; i < 4; ++i) {
        data.push_back((length >> (i * 8)) & 0xFF);
    }

    // Payload
    data.insert(data.end(), frame.payload.begin(), frame.payload.end());

    return Result<ByteBuffer>(data);
}

Result<Frame> BinarySerializer::deserialize_frame(const ByteBuffer& data) {
    if (data.size() < UMICP_FRAME_HEADER_SIZE) {
        return Result<Frame>(ErrorCode::SERIALIZATION_FAILED, "Frame too small");
    }

    Frame frame;
    size_t offset = 0;

    // Parse header
    frame.header.version = data[offset++];
    frame.header.type = data[offset++];

    // Flags (2 bytes, little-endian)
    frame.header.flags = static_cast<uint16_t>(data[offset] | (data[offset + 1] << 8));
    offset += 2;

    // Stream ID (8 bytes, little-endian)
    frame.header.stream_id = 0;
    for (int i = 0; i < 8; ++i) {
        frame.header.stream_id |= static_cast<uint64_t>(data[offset + i]) << (i * 8);
    }
    offset += 8;

    // Sequence (4 bytes, little-endian)
    frame.header.sequence = 0;
    for (int i = 0; i < 4; ++i) {
        frame.header.sequence |= static_cast<uint32_t>(data[offset + i]) << (i * 8);
    }
    offset += 4;

    // Length (4 bytes, little-endian)
    uint32_t length = 0;
    for (int i = 0; i < 4; ++i) {
        length |= static_cast<uint32_t>(data[offset + i]) << (i * 8);
    }
    offset += 4;

    // Validate frame size
    if (data.size() != UMICP_FRAME_HEADER_SIZE + length) {
        return Result<Frame>(ErrorCode::SERIALIZATION_FAILED, "Frame size mismatch");
    }

    // Extract payload
    frame.payload.assign(data.begin() + offset, data.end());

    return Result<Frame>(frame);
}

Result<ByteBuffer> BinarySerializer::serialize_envelope_cbor(const Envelope& envelope) {
    // For MVP, just use JSON serialization and convert to bytes
    // In production, would use proper CBOR library
    auto json_result = JsonSerializer::serialize_envelope(envelope);
    if (!json_result.is_success()) {
        return Result<ByteBuffer>(json_result.code, json_result.error_message.value());
    }

    const std::string& json_str = *json_result.value;
    ByteBuffer data(json_str.begin(), json_str.end());

    return Result<ByteBuffer>(data);
}

Result<Envelope> BinarySerializer::deserialize_envelope_cbor(const ByteBuffer& data) {
    // For MVP, assume it's JSON in bytes
    std::string json_str(data.begin(), data.end());
    return JsonSerializer::deserialize_envelope(json_str);
}

Result<ByteBuffer> BinarySerializer::encode_cbor(const JsonObject& data) {
    try {
        // Create CBOR map
        cbor_item_t* map = cbor_new_definite_map(data.size());
        size_t index = 0;

        for (const auto& [key, value] : data) {
            // Create key string item
            cbor_item_t* key_item = cbor_new_definite_string();
            unsigned char* key_data = new unsigned char[key.length()];
            std::memcpy(key_data, key.c_str(), key.length());
            cbor_string_set_handle(key_item, key_data, key.length());

            // Create value string item
            cbor_item_t* value_item = cbor_new_definite_string();
            unsigned char* value_data = new unsigned char[value.length()];
            std::memcpy(value_data, value.c_str(), value.length());
            cbor_string_set_handle(value_item, value_data, value.length());

            if (!cbor_map_add(map, cbor_pair{cbor_move(key_item), cbor_move(value_item)})) {
                cbor_decref(&map);
                return Result<ByteBuffer>(ErrorCode::SERIALIZATION_FAILED, "Failed to add pair to CBOR map");
            }
            index++;
        }

        // Serialize to bytes
        unsigned char* buffer;
        size_t buffer_size;
        size_t length = cbor_serialize_alloc(map, &buffer, &buffer_size);

        if (length == 0) {
            cbor_decref(&map);
            return Result<ByteBuffer>(ErrorCode::SERIALIZATION_FAILED, "CBOR serialization failed");
        }

        ByteBuffer result(buffer, buffer + length);
        free(buffer);
        cbor_decref(&map);

        return Result<ByteBuffer>(result);
    } catch (const std::exception& e) {
        return Result<ByteBuffer>(ErrorCode::SERIALIZATION_FAILED, "CBOR encoding error: " + std::string(e.what()));
    }
}

Result<JsonObject> BinarySerializer::decode_cbor(const ByteBuffer& data) {
    try {
        // Parse CBOR data
        struct cbor_load_result result;
        cbor_item_t* item = cbor_load(data.data(), data.size(), &result);

        if (result.error.code != CBOR_ERR_NONE) {
            return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "CBOR parsing error: " + std::to_string(static_cast<int>(result.error.code)));
        }

        if (!cbor_isa_map(item)) {
            cbor_decref(&item);
            return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "CBOR data is not a map");
        }

        // Convert to JsonObject
        JsonObject json_obj;
        struct cbor_pair* pairs = cbor_map_handle(item);
        size_t size = cbor_map_size(item);

        for (size_t i = 0; i < size; i++) {
            if (cbor_isa_string(pairs[i].key) && cbor_isa_string(pairs[i].value)) {
                auto key_data = cbor_string_handle(pairs[i].key);
                auto key_length = cbor_string_length(pairs[i].key);
                std::string key(reinterpret_cast<const char*>(key_data), key_length);

                auto value_data = cbor_string_handle(pairs[i].value);
                auto value_length = cbor_string_length(pairs[i].value);
                std::string value(reinterpret_cast<const char*>(value_data), value_length);

                json_obj[key] = value;
            }
        }

        cbor_decref(&item);
        return Result<JsonObject>(json_obj);
    } catch (const std::exception& e) {
        return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "CBOR decoding error: " + std::string(e.what()));
    }
}

Result<ByteBuffer> BinarySerializer::encode_msgpack(const JsonObject& data) {
    try {
        // Simple MessagePack-like encoding for strings only
        // This is a minimal implementation for testing purposes
        ByteBuffer result;

        // Map header (fixmap with size)
        if (data.size() <= 15) {
            result.push_back(0x80 | data.size()); // fixmap
        } else {
            result.push_back(0xde); // map 16
            result.push_back((data.size() >> 8) & 0xff);
            result.push_back(data.size() & 0xff);
        }

        for (const auto& [key, value] : data) {
            // Encode key
            if (key.length() <= 31) {
                result.push_back(0xa0 | key.length()); // fixstr
            } else {
                result.push_back(0xdb); // str 32
                result.push_back((key.length() >> 24) & 0xff);
                result.push_back((key.length() >> 16) & 0xff);
                result.push_back((key.length() >> 8) & 0xff);
                result.push_back(key.length() & 0xff);
            }
            result.insert(result.end(), key.begin(), key.end());

            // Encode value
            if (value.length() <= 31) {
                result.push_back(0xa0 | value.length()); // fixstr
            } else {
                result.push_back(0xdb); // str 32
                result.push_back((value.length() >> 24) & 0xff);
                result.push_back((value.length() >> 16) & 0xff);
                result.push_back((value.length() >> 8) & 0xff);
                result.push_back(value.length() & 0xff);
            }
            result.insert(result.end(), value.begin(), value.end());
        }

        return Result<ByteBuffer>(result);
    } catch (const std::exception& e) {
        return Result<ByteBuffer>(ErrorCode::SERIALIZATION_FAILED, "MessagePack encoding error: " + std::string(e.what()));
    }
}

Result<JsonObject> BinarySerializer::decode_msgpack(const ByteBuffer& data) {
    try {
        JsonObject result;
        size_t pos = 0;

        if (pos >= data.size()) {
            return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "Empty MessagePack data");
        }

        // Read map header
        uint8_t map_type = data[pos++];
        size_t map_size;

        if ((map_type & 0xf0) == 0x80) {
            // fixmap
            map_size = map_type & 0x0f;
        } else if (map_type == 0xde) {
            // map 16
            if (pos + 2 > data.size()) {
                return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "Incomplete map size");
            }
            map_size = (data[pos] << 8) | data[pos + 1];
            pos += 2;
        } else {
            return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "Invalid map type");
        }

        // Read key-value pairs
        for (size_t i = 0; i < map_size; i++) {
            // Read key
            auto key_result = decode_msgpack_string(data, pos);
            if (!key_result.is_success()) {
                return Result<JsonObject>(key_result.code, key_result.error_message.value());
            }
            std::string key = *key_result.value;

            // Read value
            auto value_result = decode_msgpack_string(data, pos);
            if (!value_result.is_success()) {
                return Result<JsonObject>(value_result.code, value_result.error_message.value());
            }
            std::string value = *value_result.value;

            result[key] = value;
        }

        return Result<JsonObject>(result);
    } catch (const std::exception& e) {
        return Result<JsonObject>(ErrorCode::SERIALIZATION_FAILED, "MessagePack decoding error: " + std::string(e.what()));
    }
}

// Helper function for MessagePack string decoding
Result<std::string> BinarySerializer::decode_msgpack_string(const ByteBuffer& data, size_t& pos) {
    if (pos >= data.size()) {
        return Result<std::string>(ErrorCode::SERIALIZATION_FAILED, "Incomplete string header");
    }

    uint8_t str_type = data[pos++];
    size_t str_length;

    if ((str_type & 0xe0) == 0xa0) {
        // fixstr
        str_length = str_type & 0x1f;
    } else if (str_type == 0xdb) {
        // str 32
        if (pos + 4 > data.size()) {
            return Result<std::string>(ErrorCode::SERIALIZATION_FAILED, "Incomplete string length");
        }
        str_length = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;
    } else {
        return Result<std::string>(ErrorCode::SERIALIZATION_FAILED, "Invalid string type");
    }

    if (pos + str_length > data.size()) {
        return Result<std::string>(ErrorCode::SERIALIZATION_FAILED, "Incomplete string data");
    }

    std::string result(data.begin() + pos, data.begin() + pos + str_length);
    pos += str_length;

    return Result<std::string>(result);
}

// ===============================================
// Hash Utilities
// ===============================================

Result<std::string> HashUtils::sha256_hex(const ByteBuffer& data) {
    auto raw_result = sha256_raw(data);
    if (!raw_result.is_success()) {
        return Result<std::string>(raw_result.code, raw_result.error_message.value());
    }

    const auto& hash_bytes = *raw_result.value;
    std::ostringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');

    for (uint8_t byte : hash_bytes) {
        hex_stream << std::setw(2) << static_cast<int>(byte);
    }

    return Result<std::string>(hex_stream.str());
}

Result<std::string> HashUtils::sha256_hex(const std::string& str) {
    ByteBuffer data(str.begin(), str.end());
    return sha256_hex(data);
}

Result<ByteBuffer> HashUtils::sha256_raw(const ByteBuffer& data) {
    ByteBuffer hash(32); // SHA256_DIGEST_LENGTH = 32

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, data.data(), data.size());
    EVP_DigestFinal_ex(mdctx, hash.data(), NULL);
    EVP_MD_CTX_free(mdctx);

    return Result<ByteBuffer>(hash);
}

// ===============================================
// Base64 Utilities
// ===============================================

Result<std::string> Base64Utils::encode(const ByteBuffer& data) {
    if (data.empty()) {
        return Result<std::string>("");
    }

    const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t triple = data[i] << 16;
        if (i + 1 < data.size()) triple |= data[i + 1] << 8;
        if (i + 2 < data.size()) triple |= data[i + 2];

        result += base64_chars[(triple >> 18) & 0x3F];
        result += base64_chars[(triple >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? base64_chars[(triple >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? base64_chars[triple & 0x3F] : '=';
    }

    return Result<std::string>(result);
}

Result<ByteBuffer> Base64Utils::decode(const std::string& encoded) {
    if (encoded.empty()) {
        return Result<ByteBuffer>(ByteBuffer{});
    }

    // Create decode table
    int decode_table[256];
    std::fill(decode_table, decode_table + 256, -1);

    const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (int i = 0; i < 64; ++i) {
        decode_table[static_cast<unsigned char>(base64_chars[i])] = i;
    }

    ByteBuffer result;
    result.reserve((encoded.size() * 3) / 4);

    for (size_t i = 0; i < encoded.size(); i += 4) {
        if (i + 3 >= encoded.size()) break;

        int values[4];
        for (int j = 0; j < 4; ++j) {
            if (encoded[i + j] == '=') {
                values[j] = 0;
            } else {
                values[j] = decode_table[static_cast<unsigned char>(encoded[i + j])];
                if (values[j] == -1) {
                    return Result<ByteBuffer>(ErrorCode::SERIALIZATION_FAILED, "Invalid base64 character");
                }
            }
        }

        uint32_t triple = (values[0] << 18) | (values[1] << 12) | (values[2] << 6) | values[3];

        result.push_back((triple >> 16) & 0xFF);
        if (encoded[i + 2] != '=') result.push_back((triple >> 8) & 0xFF);
        if (encoded[i + 3] != '=') result.push_back(triple & 0xFF);
    }

    return Result<ByteBuffer>(result);
}

} // namespace umicp
