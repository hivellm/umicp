/**
 * UMICP Frame Node.js Wrapper Header
 */

#ifndef FRAME_WRAP_H
#define FRAME_WRAP_H

#include <napi.h>
#include "../../../cpp/include/frame.h"

class FrameWrap : public Napi::ObjectWrap<FrameWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    FrameWrap(const Napi::CallbackInfo& info);

private:
    // Frame operations
    Napi::Value SetType(const Napi::CallbackInfo& info);
    Napi::Value SetStreamId(const Napi::CallbackInfo& info);
    Napi::Value SetSequence(const Napi::CallbackInfo& info);
    Napi::Value SetFlags(const Napi::CallbackInfo& info);
    Napi::Value SetPayload(const Napi::CallbackInfo& info);
    Napi::Value Serialize(const Napi::CallbackInfo& info);
    Napi::Value Deserialize(const Napi::CallbackInfo& info);
    Napi::Value GetType(const Napi::CallbackInfo& info);
    Napi::Value GetStreamId(const Napi::CallbackInfo& info);
    Napi::Value GetSequence(const Napi::CallbackInfo& info);
    Napi::Value GetFlags(const Napi::CallbackInfo& info);
    Napi::Value GetPayloadSize(const Napi::CallbackInfo& info);

private:
    umicp::Frame frame_;
};

#endif // FRAME_WRAP_H
