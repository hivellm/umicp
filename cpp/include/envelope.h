/**
 * UMICP Envelope Header
 * JSON control plane message handling
 */

#ifndef UMICP_ENVELOPE_H
#define UMICP_ENVELOPE_H

#include "umicp_types.h"
#include <memory>
#include <string>

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
    EnvelopeBuilder& capabilities(const StringMap& caps);
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
