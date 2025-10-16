/**
 * UMICP Envelope Header
 * JSON control plane message handling
 *
 * Version 1.1.0 - Native JSON type support in capabilities
 */

#ifndef UMICP_ENVELOPE_H
#define UMICP_ENVELOPE_H

#include "umicp_types.h"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace umicp {

// Forward declaration
class EnvelopeImpl;

// Envelope builder pattern
class EnvelopeBuilder {
public:
    EnvelopeBuilder();
    ~EnvelopeBuilder();

    EnvelopeBuilder& from(const std::string& from);
    EnvelopeBuilder& to(const std::string& to);
    EnvelopeBuilder& operation(OperationType op);
    EnvelopeBuilder& message_id(const std::string& msg_id);

    // BREAKING CHANGE (v1.1.0): capabilities now accepts CapabilitiesMap with native JSON types
    EnvelopeBuilder& capabilities(const CapabilitiesMap& caps);

    // Add a single capability with native JSON value
    EnvelopeBuilder& capability(const std::string& key, const json& value);

    // Convenience methods for common types (backward compatible)
    EnvelopeBuilder& capability_str(const std::string& key, const std::string& value);
    EnvelopeBuilder& capability_int(const std::string& key, int64_t value);
    EnvelopeBuilder& capability_bool(const std::string& key, bool value);
    EnvelopeBuilder& capability_double(const std::string& key, double value);

    EnvelopeBuilder& payload_hint(const PayloadHint& hint);

    Result<Envelope> build();

private:
    std::unique_ptr<EnvelopeImpl> impl_;
};

// Envelope processor for serialization/validation
class EnvelopeProcessor {
public:
    static Result<std::string> serialize(const Envelope& envelope);
    static Result<Envelope> deserialize(const std::string& json_str);
    static Result<void> validate(const Envelope& envelope);
    static std::string hash(const Envelope& envelope);
};

} // namespace umicp

#endif // UMICP_ENVELOPE_H
