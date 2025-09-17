/**
 * UMICP Node.js Native Addon - Main Entry Point
 * TypeScript/JavaScript bindings for UMICP C++ core
 */

#include <napi.h>
#include "envelope_wrap.h"
#include "frame_wrap.h"
#include "matrix_wrap.h"

// Module initialization
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    // Initialize envelope bindings
    EnvelopeWrap::Init(env, exports);

    // Initialize frame bindings
    FrameWrap::Init(env, exports);

    // Initialize matrix operations bindings
    MatrixWrap::Init(env, exports);

    // Note: WebSocket and HTTP/2 transports are available in TypeScript layer only
    // They are not included in this C++ binding to keep the core lightweight

    // Export version information
    exports.Set("version", Napi::String::New(env, "1.0.0"));
    exports.Set("UMICP_VERSION", Napi::String::New(env, "1.0"));

    // Export constants
    exports.Set("OPERATION_CONTROL", Napi::Number::New(env, 0));
    exports.Set("OPERATION_DATA", Napi::Number::New(env, 1));
    exports.Set("OPERATION_ACK", Napi::Number::New(env, 2));
    exports.Set("OPERATION_ERROR", Napi::Number::New(env, 3));

    exports.Set("PAYLOAD_VECTOR", Napi::Number::New(env, 0));
    exports.Set("PAYLOAD_TEXT", Napi::Number::New(env, 1));
    exports.Set("PAYLOAD_METADATA", Napi::Number::New(env, 2));
    exports.Set("PAYLOAD_BINARY", Napi::Number::New(env, 3));

    exports.Set("ENCODING_FLOAT32", Napi::Number::New(env, 0));
    exports.Set("ENCODING_FLOAT64", Napi::Number::New(env, 1));
    exports.Set("ENCODING_INT32", Napi::Number::New(env, 2));
    exports.Set("ENCODING_INT64", Napi::Number::New(env, 3));
    exports.Set("ENCODING_UINT8", Napi::Number::New(env, 4));
    exports.Set("ENCODING_UINT16", Napi::Number::New(env, 5));
    exports.Set("ENCODING_UINT32", Napi::Number::New(env, 6));
    exports.Set("ENCODING_UINT64", Napi::Number::New(env, 7));

    return exports;
}

// Node.js addon registration
NODE_API_MODULE(umicp_core, InitAll)
