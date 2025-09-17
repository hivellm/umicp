/**
 * UMICP Envelope Implementation
 * JSON control plane message handling with canonical serialization
 */

#include "umicp_types.h"
#include "envelope.h"
#include <json-c/json.h>
#include <openssl/evp.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <uuid/uuid.h>

namespace umicp {

// Envelope implementation
class EnvelopeImpl {
public:
    Envelope envelope;

    EnvelopeImpl() {
        envelope.version = "1.0";
        envelope.msg_id = generate_uuid();
        envelope.ts = get_current_timestamp();
        envelope.op = OperationType::CONTROL;
    }

    std::string generate_uuid() {
        uuid_t uuid;
        char uuid_str[37];
        uuid_generate(uuid);
        uuid_unparse_lower(uuid, uuid_str);
        return std::string(uuid_str);
    }

    std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3)
           << (std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch()).count() % 1000);
        ss << "Z";
        return ss.str();
    }

    Result<std::string> serialize() {
        json_object* root = json_object_new_object();

        // Required fields
        json_object_object_add(root, "v", json_object_new_string(envelope.version.c_str()));
        json_object_object_add(root, "msg_id", json_object_new_string(envelope.msg_id.c_str()));
        json_object_object_add(root, "ts", json_object_new_string(envelope.ts.c_str()));
        json_object_object_add(root, "from", json_object_new_string(envelope.from.c_str()));
        json_object_object_add(root, "to", json_object_new_string(envelope.to.c_str()));
        json_object_object_add(root, "op", json_object_new_string(operation_to_string(envelope.op).c_str()));

        // Optional fields
        if (envelope.capabilities) {
            json_object* caps = json_object_new_object();
            for (const auto& [key, value] : *envelope.capabilities) {
                json_object_object_add(caps, key.c_str(), json_object_new_string(value.c_str()));
            }
            json_object_object_add(root, "capabilities", caps);
        }

        if (envelope.schema_uri) {
            json_object_object_add(root, "schema_uri", json_object_new_string(envelope.schema_uri->c_str()));
        }

        if (envelope.accept) {
            json_object* accept_array = json_object_new_array();
            for (const auto& type : *envelope.accept) {
                json_object_array_add(accept_array, json_object_new_string(type.c_str()));
            }
            json_object_object_add(root, "accept", accept_array);
        }

        if (envelope.payload_hint) {
            json_object* hint = json_object_new_object();
            json_object_object_add(hint, "type", json_object_new_string(payload_type_to_string(envelope.payload_hint.value().type).c_str()));
            if (envelope.payload_hint.value().size) {
                json_object_object_add(hint, "size", json_object_new_int64(*envelope.payload_hint.value().size));
            }
            if (envelope.payload_hint.value().encoding) {
                json_object_object_add(hint, "encoding", json_object_new_string(encoding_type_to_string(*envelope.payload_hint.value().encoding).c_str()));
            }
            if (envelope.payload_hint.value().count) {
                json_object_object_add(hint, "count", json_object_new_int64(*envelope.payload_hint.value().count));
            }
            json_object_object_add(root, "payload_hint", hint);
        }

        if (envelope.payload_refs) {
            json_object* refs_array = json_object_new_array();
            for (const auto& ref : *envelope.payload_refs) {
                json_object* ref_obj = json_object_new_object();
                json_object_object_add(ref_obj, "stream_id", json_object_new_string(ref.at("stream_id").c_str()));
                json_object_object_add(ref_obj, "offset", json_object_new_int64(std::stoll(ref.at("offset"))));
                json_object_object_add(ref_obj, "length", json_object_new_int64(std::stoll(ref.at("length"))));
                json_object_object_add(ref_obj, "checksum", json_object_new_string(ref.at("checksum").c_str()));
                json_object_array_add(refs_array, ref_obj);
            }
            json_object_object_add(root, "payload_refs", refs_array);
        }

        const char* json_str = json_object_to_json_string(root);
        std::string result = json_str;

        json_object_put(root);
        return Result<std::string>(result);
    }

    Result<void> deserialize(const std::string& json_str) {
        json_object* root = json_tokener_parse(json_str.c_str());
        if (!root) {
            return Result<void>(ErrorCode::SERIALIZATION_FAILED, "Invalid JSON format");
        }

        // Parse required fields
        json_object* v_obj;
        if (json_object_object_get_ex(root, "v", &v_obj)) {
            envelope.version = json_object_get_string(v_obj);
        }

        json_object* msg_id_obj;
        if (json_object_object_get_ex(root, "msg_id", &msg_id_obj)) {
            envelope.msg_id = json_object_get_string(msg_id_obj);
        }

        json_object* ts_obj;
        if (json_object_object_get_ex(root, "ts", &ts_obj)) {
            envelope.ts = json_object_get_string(ts_obj);
        }

        json_object* from_obj;
        if (json_object_object_get_ex(root, "from", &from_obj)) {
            envelope.from = json_object_get_string(from_obj);
        }

        json_object* to_obj;
        if (json_object_object_get_ex(root, "to", &to_obj)) {
            envelope.to = json_object_get_string(to_obj);
        }

        json_object* op_obj;
        if (json_object_object_get_ex(root, "op", &op_obj)) {
            envelope.op = string_to_operation(json_object_get_string(op_obj));
        }

        // Parse optional fields
        json_object* caps_obj;
        if (json_object_object_get_ex(root, "capabilities", &caps_obj)) {
            envelope.capabilities = StringMap();
            json_object_object_foreach(caps_obj, key, val) {
                (*envelope.capabilities)[key] = json_object_get_string(val);
            }
        }

        json_object* schema_obj;
        if (json_object_object_get_ex(root, "schema_uri", &schema_obj)) {
            envelope.schema_uri = json_object_get_string(schema_obj);
        }

        json_object* accept_obj;
        if (json_object_object_get_ex(root, "accept", &accept_obj)) {
            envelope.accept = std::vector<std::string>();
            for (size_t i = 0; i < json_object_array_length(accept_obj); i++) {
                json_object* item = json_object_array_get_idx(accept_obj, i);
                envelope.accept->push_back(json_object_get_string(item));
            }
        }

        json_object_put(root);
        return Result<void>();
    }

    Result<void> validate() {
        // Required field validation
        if (envelope.version.empty()) {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Missing version field");
        }

        if (envelope.msg_id.empty()) {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Missing message ID field");
        }

        if (envelope.ts.empty()) {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Missing timestamp field");
        }

        if (envelope.from.empty()) {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Missing sender field");
        }

        if (envelope.to.empty()) {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Missing recipient field");
        }

        // Version validation
        if (envelope.version != "1.0") {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Unsupported protocol version");
        }

        // Timestamp validation (basic format check)
        if (envelope.ts.length() < 20) {
            return Result<void>(ErrorCode::INVALID_ENVELOPE, "Invalid timestamp format");
        }

        return Result<void>();
    }

private:
    std::string operation_to_string(OperationType op) {
        switch (op) {
            case OperationType::CONTROL: return "CONTROL";
            case OperationType::DATA: return "DATA";
            case OperationType::ACK: return "ACK";
            case OperationType::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    OperationType string_to_operation(const std::string& str) {
        if (str == "CONTROL") return OperationType::CONTROL;
        if (str == "DATA") return OperationType::DATA;
        if (str == "ACK") return OperationType::ACK;
        if (str == "ERROR") return OperationType::ERROR;
        return OperationType::CONTROL;
    }

    std::string payload_type_to_string(PayloadType type) {
        switch (type) {
            case PayloadType::VECTOR: return "vector";
            case PayloadType::TEXT: return "text";
            case PayloadType::METADATA: return "metadata";
            case PayloadType::BINARY: return "binary";
            default: return "metadata";
        }
    }

    std::string encoding_type_to_string(EncodingType type) {
        switch (type) {
            case EncodingType::FLOAT32: return "float32";
            case EncodingType::FLOAT64: return "float64";
            case EncodingType::INT32: return "int32";
            case EncodingType::INT64: return "int64";
            case EncodingType::UINT8: return "uint8";
            case EncodingType::UINT16: return "uint16";
            case EncodingType::UINT32: return "uint32";
            case EncodingType::UINT64: return "uint64";
            default: return "float32";
        }
    }
};

// Public API implementation
EnvelopeBuilder::EnvelopeBuilder() : impl_(std::make_unique<EnvelopeImpl>()) {}

EnvelopeBuilder::~EnvelopeBuilder() = default;

EnvelopeBuilder& EnvelopeBuilder::from(const std::string& from) {
    impl_->envelope.from = from;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::to(const std::string& to) {
    impl_->envelope.to = to;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::operation(OperationType op) {
    impl_->envelope.op = op;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::message_id(const std::string& msg_id) {
    impl_->envelope.msg_id = msg_id;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::capabilities(const StringMap& caps) {
    impl_->envelope.capabilities = caps;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::payload_hint(const PayloadHint& hint) {
    impl_->envelope.payload_hint = hint;
    return *this;
}

Result<Envelope> EnvelopeBuilder::build() {
    auto validation = impl_->validate();
    if (!validation.is_success()) {
        return Result<Envelope>(validation.code, validation.error_message.value_or("Validation failed"));
    }
    return Result<Envelope>(impl_->envelope);
}

// EnvelopeProcessor implementation
Result<std::string> EnvelopeProcessor::serialize(const Envelope& envelope) {
    EnvelopeImpl impl;
    impl.envelope = envelope;
    return impl.serialize();
}

Result<Envelope> EnvelopeProcessor::deserialize(const std::string& json_str) {
    EnvelopeImpl impl;
    auto result = impl.deserialize(json_str);
    if (!result.is_success()) {
        return Result<Envelope>(result.code, result.error_message.value_or("Deserialization failed"));
    }
    return Result<Envelope>(impl.envelope);
}

Result<void> EnvelopeProcessor::validate(const Envelope& envelope) {
    EnvelopeImpl impl;
    impl.envelope = envelope;
    return impl.validate();
}

std::string EnvelopeProcessor::hash(const Envelope& envelope) {
    auto serialized = serialize(envelope);
    if (!serialized.is_success()) {
        return "";
    }

    unsigned char hash[32]; // SHA256_DIGEST_LENGTH = 32
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, serialized.value->c_str(), serialized.value->length());
    EVP_DigestFinal_ex(mdctx, hash, NULL);
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

} // namespace umicp
