/**
 * UMICP Envelope Implementation
 * JSON control plane message handling with canonical serialization
 *
 * Version 1.1.0 - Native JSON type support with nlohmann/json
 */

#include "umicp_types.h"
#include "envelope.h"
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <uuid/uuid.h>

using json = nlohmann::json;

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
        json root;

        // Required fields
        root["v"] = envelope.version;
        root["msg_id"] = envelope.msg_id;
        root["ts"] = envelope.ts;
        root["from"] = envelope.from;
        root["to"] = envelope.to;
        root["op"] = operation_to_string(envelope.op);

        // Optional fields - nlohmann/json handles native types automatically!
        if (envelope.capabilities) {
            json caps_json = json::object();
            for (const auto& [key, value] : *envelope.capabilities) {
                caps_json[key] = value;  // Direct assignment - supports all JSON types!
            }
            root["capabilities"] = caps_json;
        }

        if (envelope.schema_uri) {
            root["schema_uri"] = *envelope.schema_uri;
        }

        if (envelope.accept) {
            root["accept"] = *envelope.accept;
        }

        if (envelope.payload_hint) {
            json hint;
            hint["type"] = payload_type_to_string(envelope.payload_hint->type);
            if (envelope.payload_hint->size) {
                hint["size"] = *envelope.payload_hint->size;
            }
            if (envelope.payload_hint->encoding) {
                hint["encoding"] = encoding_type_to_string(*envelope.payload_hint->encoding);
            }
            if (envelope.payload_hint->count) {
                hint["count"] = *envelope.payload_hint->count;
            }
            root["payload_hint"] = hint;
        }

        if (envelope.payload_refs) {
            json refs_array = json::array();
            for (const auto& ref : *envelope.payload_refs) {
                json ref_obj;
                ref_obj["stream_id"] = ref.at("stream_id");
                ref_obj["offset"] = std::stoll(ref.at("offset"));
                ref_obj["length"] = std::stoll(ref.at("length"));
                ref_obj["checksum"] = ref.at("checksum");
                refs_array.push_back(ref_obj);
            }
            root["payload_refs"] = refs_array;
        }

        return Result<std::string>(root.dump());
    }

    Result<void> deserialize(const std::string& json_str) {
        try {
            json root = json::parse(json_str);

            // Parse required fields
            if (root.contains("v")) {
                envelope.version = root["v"].get<std::string>();
            }
            if (root.contains("msg_id")) {
                envelope.msg_id = root["msg_id"].get<std::string>();
            }
            if (root.contains("ts")) {
                envelope.ts = root["ts"].get<std::string>();
            }
            if (root.contains("from")) {
                envelope.from = root["from"].get<std::string>();
            }
            if (root.contains("to")) {
                envelope.to = root["to"].get<std::string>();
            }
            if (root.contains("op")) {
                envelope.op = string_to_operation(root["op"].get<std::string>());
            }

            // Parse optional fields - native types preserved!
            if (root.contains("capabilities") && root["capabilities"].is_object()) {
                CapabilitiesMap caps;
                for (auto& [key, value] : root["capabilities"].items()) {
                    caps[key] = value;  // Preserves native types!
                }
                envelope.capabilities = caps;
            }

            if (root.contains("schema_uri")) {
                envelope.schema_uri = root["schema_uri"].get<std::string>();
            }

            if (root.contains("accept") && root["accept"].is_array()) {
                envelope.accept = root["accept"].get<std::vector<std::string>>();
            }

            if (root.contains("payload_hint") && root["payload_hint"].is_object()) {
                // Parse payload hint (if needed)
            }

            return Result<void>();

        } catch (const json::exception& e) {
            return Result<void>(ErrorCode::SERIALIZATION_FAILED,
                std::string("JSON parse error: ") + e.what());
        }
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

EnvelopeBuilder& EnvelopeBuilder::capabilities(const CapabilitiesMap& caps) {
    impl_->envelope.capabilities = caps;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::capability(const std::string& key, const json& value) {
    if (!impl_->envelope.capabilities) {
        impl_->envelope.capabilities = CapabilitiesMap();
    }
    (*impl_->envelope.capabilities)[key] = value;
    return *this;
}

EnvelopeBuilder& EnvelopeBuilder::capability_str(const std::string& key, const std::string& value) {
    return capability(key, json(value));
}

EnvelopeBuilder& EnvelopeBuilder::capability_int(const std::string& key, int64_t value) {
    return capability(key, json(value));
}

EnvelopeBuilder& EnvelopeBuilder::capability_bool(const std::string& key, bool value) {
    return capability(key, json(value));
}

EnvelopeBuilder& EnvelopeBuilder::capability_double(const std::string& key, double value) {
    return capability(key, json(value));
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

// Envelope helper method implementations
std::string Envelope::to_json() const {
    auto result = EnvelopeProcessor::serialize(*this);
    if (result.is_success() && result.value.has_value()) {
        return result.value.value();
    }
    return "{}";
}

Envelope Envelope::from_json(const std::string& json) {
    auto result = EnvelopeProcessor::deserialize(json);
    if (result.is_success() && result.value.has_value()) {
        return result.value.value();
    }
    return Envelope{};
}

} // namespace umicp
