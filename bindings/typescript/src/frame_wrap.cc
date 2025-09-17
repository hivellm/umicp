/**
 * UMICP Frame Node.js Wrapper Implementation
 */

#include "frame_wrap.h"
#include "../../../cpp/include/frame.h"
#include <napi.h>

using namespace umicp;

Napi::Object FrameWrap::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Frame", {
        InstanceMethod("setType", &FrameWrap::SetType),
        InstanceMethod("setStreamId", &FrameWrap::SetStreamId),
        InstanceMethod("setSequence", &FrameWrap::SetSequence),
        InstanceMethod("setFlags", &FrameWrap::SetFlags),
        InstanceMethod("setPayload", &FrameWrap::SetPayload),
        InstanceMethod("serialize", &FrameWrap::Serialize),
        InstanceMethod("deserialize", &FrameWrap::Deserialize),
        InstanceMethod("getType", &FrameWrap::GetType),
        InstanceMethod("getStreamId", &FrameWrap::GetStreamId),
        InstanceMethod("getSequence", &FrameWrap::GetSequence),
        InstanceMethod("getFlags", &FrameWrap::GetFlags),
        InstanceMethod("getPayloadSize", &FrameWrap::GetPayloadSize)
    });

    exports.Set("Frame", func);
    return exports;
}

FrameWrap::FrameWrap(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<FrameWrap>(info) {
    // Initialize with default frame
    frame_.header.version = 1;
    frame_.header.type = 0;
    frame_.header.flags = 0;
    frame_.header.stream_id = 0;
    frame_.header.sequence = 0;
    frame_.header.length = 0;
}

Napi::Value FrameWrap::SetType(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    frame_.header.type = info[0].As<Napi::Number>().Uint32Value();
    return info.This();
}

Napi::Value FrameWrap::SetStreamId(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    frame_.header.stream_id = info[0].As<Napi::Number>().Int64Value();
    return info.This();
}

Napi::Value FrameWrap::SetSequence(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    frame_.header.sequence = info[0].As<Napi::Number>().Uint32Value();
    return info.This();
}

Napi::Value FrameWrap::SetFlags(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    frame_.header.flags = info[0].As<Napi::Number>().Uint32Value();
    return info.This();
}

Napi::Value FrameWrap::SetPayload(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsTypedArray()) {
        Napi::TypeError::New(env, "TypedArray expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::TypedArray typedArray = info[0].As<Napi::TypedArray>();

    if (typedArray.TypedArrayType() != napi_uint8_array) {
        Napi::TypeError::New(env, "Uint8Array expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    size_t length = typedArray.ElementLength();
    frame_.payload.resize(length);

    uint8_t* data = reinterpret_cast<uint8_t*>(typedArray.ArrayBuffer().Data()) + typedArray.ByteOffset();
    std::memcpy(frame_.payload.data(), data, length);

    frame_.header.length = static_cast<uint32_t>(length);

    return info.This();
}

Napi::Value FrameWrap::Serialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    try {
        auto result = FrameProcessor::serialize(frame_);
        if (!result.is_success()) {
            Napi::Error::New(env, result.error_message.value_or("Serialization failed")).ThrowAsJavaScriptException();
            return env.Null();
        }

        // Create Node.js Buffer from the result
        auto& buffer = *result.value;
        return Napi::Buffer<uint8_t>::Copy(env, buffer.data(), buffer.size());

    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Exception: ") + e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value FrameWrap::Deserialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsTypedArray()) {
        Napi::TypeError::New(env, "TypedArray expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        Napi::TypedArray typedArray = info[0].As<Napi::TypedArray>();

        if (typedArray.TypedArrayType() != napi_uint8_array) {
            Napi::TypeError::New(env, "Uint8Array expected").ThrowAsJavaScriptException();
            return env.Null();
        }

        size_t length = typedArray.ElementLength();
        ByteBuffer input(length);

        uint8_t* data = reinterpret_cast<uint8_t*>(typedArray.ArrayBuffer().Data()) + typedArray.ByteOffset();
        std::memcpy(input.data(), data, length);

        auto result = FrameProcessor::deserialize(input);
        if (!result.is_success()) {
            Napi::Error::New(env, result.error_message.value_or("Deserialization failed")).ThrowAsJavaScriptException();
            return env.Null();
        }

        frame_ = *result.value;
        return info.This();

    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Exception: ") + e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value FrameWrap::GetType(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), frame_.header.type);
}

Napi::Value FrameWrap::GetStreamId(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(frame_.header.stream_id));
}

Napi::Value FrameWrap::GetSequence(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), frame_.header.sequence);
}

Napi::Value FrameWrap::GetFlags(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), frame_.header.flags);
}

Napi::Value FrameWrap::GetPayloadSize(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), frame_.payload.size());
}
