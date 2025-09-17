/**
 * UMICP Envelope Node.js Wrapper Header
 */

#ifndef ENVELOPE_WRAP_H
#define ENVELOPE_WRAP_H

#include <napi.h>
#include "../../../cpp/include/envelope.h"

class EnvelopeWrap : public Napi::ObjectWrap<EnvelopeWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    EnvelopeWrap(const Napi::CallbackInfo& info);

private:
    // Envelope operations
    Napi::Value SetFrom(const Napi::CallbackInfo& info);
    Napi::Value SetTo(const Napi::CallbackInfo& info);
    Napi::Value SetOperation(const Napi::CallbackInfo& info);
    Napi::Value SetMessageId(const Napi::CallbackInfo& info);
    Napi::Value SetCapabilities(const Napi::CallbackInfo& info);
    Napi::Value SetPayloadHint(const Napi::CallbackInfo& info);
    Napi::Value Serialize(const Napi::CallbackInfo& info);
    Napi::Value Deserialize(const Napi::CallbackInfo& info);
    Napi::Value Validate(const Napi::CallbackInfo& info);
    Napi::Value GetHash(const Napi::CallbackInfo& info);
    Napi::Value GetFrom(const Napi::CallbackInfo& info);
    Napi::Value GetTo(const Napi::CallbackInfo& info);
    Napi::Value GetOperation(const Napi::CallbackInfo& info);
    Napi::Value GetMessageId(const Napi::CallbackInfo& info);
    Napi::Value GetCapabilities(const Napi::CallbackInfo& info);

    // Static methods
    static Napi::Value CreateEnvelope(const Napi::CallbackInfo& info);
    static Napi::Value SerializeEnvelope(const Napi::CallbackInfo& info);
    static Napi::Value DeserializeEnvelope(const Napi::CallbackInfo& info);
    static Napi::Value ValidateEnvelope(const Napi::CallbackInfo& info);
    static Napi::Value HashEnvelope(const Napi::CallbackInfo& info);

private:
    std::string generate_uuid();
    std::string get_current_timestamp();

    umicp::Envelope envelope_;
};

#endif // ENVELOPE_WRAP_H
