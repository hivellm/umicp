/**
 * UMICP Matrix Operations Node.js Wrapper Implementation
 */

#include "matrix_wrap.h"
#include "../../../cpp/include/matrix_ops.h"
#include <vector>
#include <stdexcept>

using namespace umicp;

Napi::Object MatrixWrap::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Matrix", {
        InstanceMethod("add", &MatrixWrap::Add),
        InstanceMethod("multiply", &MatrixWrap::Multiply),
        InstanceMethod("transpose", &MatrixWrap::Transpose),
        InstanceMethod("dotProduct", &MatrixWrap::DotProduct),
        InstanceMethod("normalize", &MatrixWrap::Normalize),
        InstanceMethod("cosineSimilarity", &MatrixWrap::CosineSimilarity)
    });

    exports.Set("Matrix", func);
    return exports;
}

MatrixWrap::MatrixWrap(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<MatrixWrap>(info) {
    // Constructor - no initialization needed
}

Napi::Value MatrixWrap::Add(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 4) {
        Napi::TypeError::New(env, "Expected 4 arguments: arrayA, arrayB, resultArray, rows, cols").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsTypedArray() || !info[1].IsTypedArray() ||
        !info[2].IsTypedArray() || !info[3].IsNumber() || !info[4].IsNumber()) {
        Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto arrayA = info[0].As<Napi::TypedArray>();
        auto arrayB = info[1].As<Napi::TypedArray>();
        auto resultArray = info[2].As<Napi::TypedArray>();
        size_t rows = info[3].As<Napi::Number>().Uint32Value();
        size_t cols = info[4].As<Napi::Number>().Uint32Value();

        std::vector<float> vecA = ConvertToFloatVector(arrayA);
        std::vector<float> vecB = ConvertToFloatVector(arrayB);
        std::vector<float> result(rows * cols);

        auto op_result = MatrixOps::add(vecA.data(), vecB.data(), result.data(), rows, cols);

        if (!op_result.is_success()) {
            return CreateResultObject(env, false, op_result.error_message.value_or("Matrix addition failed"));
        }

        // Copy result back to the typed array
        float* resultData = reinterpret_cast<float*>(resultArray.ArrayBuffer().Data());
        std::memcpy(resultData, result.data(), result.size() * sizeof(float));

        return CreateResultObject(env, true);

    } catch (const std::exception& e) {
        return CreateResultObject(env, false, std::string("Exception: ") + e.what());
    }
}

Napi::Value MatrixWrap::Multiply(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 6) {
        Napi::TypeError::New(env, "Expected 6 arguments: arrayA, arrayB, resultArray, m, n, p").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsTypedArray() || !info[1].IsTypedArray() ||
        !info[2].IsTypedArray() || !info[3].IsNumber() || !info[4].IsNumber() || !info[5].IsNumber()) {
        Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto arrayA = info[0].As<Napi::TypedArray>();
        auto arrayB = info[1].As<Napi::TypedArray>();
        auto resultArray = info[2].As<Napi::TypedArray>();
        size_t m = info[3].As<Napi::Number>().Uint32Value();
        size_t n = info[4].As<Napi::Number>().Uint32Value();
        size_t p = info[5].As<Napi::Number>().Uint32Value();

        std::vector<float> vecA = ConvertToFloatVector(arrayA);
        std::vector<float> vecB = ConvertToFloatVector(arrayB);
        std::vector<float> result(m * p);

        auto op_result = MatrixOps::multiply(vecA.data(), vecB.data(), result.data(), m, n, p);

        if (!op_result.is_success()) {
            return CreateResultObject(env, false, op_result.error_message.value_or("Matrix multiplication failed"));
        }

        // Copy result back to the typed array
        float* resultData = reinterpret_cast<float*>(resultArray.ArrayBuffer().Data());
        std::memcpy(resultData, result.data(), result.size() * sizeof(float));

        return CreateResultObject(env, true);

    } catch (const std::exception& e) {
        return CreateResultObject(env, false, std::string("Exception: ") + e.what());
    }
}

Napi::Value MatrixWrap::Transpose(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 4) {
        Napi::TypeError::New(env, "Expected 4 arguments: inputArray, outputArray, rows, cols").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsTypedArray() || !info[1].IsTypedArray() ||
        !info[2].IsNumber() || !info[3].IsNumber()) {
        Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto inputArray = info[0].As<Napi::TypedArray>();
        auto outputArray = info[1].As<Napi::TypedArray>();
        size_t rows = info[2].As<Napi::Number>().Uint32Value();
        size_t cols = info[3].As<Napi::Number>().Uint32Value();

        std::vector<float> input = ConvertToFloatVector(inputArray);
        std::vector<float> output(rows * cols);

        auto op_result = MatrixOps::transpose(input.data(), output.data(), rows, cols);

        if (!op_result.is_success()) {
            return CreateResultObject(env, false, op_result.error_message.value_or("Matrix transpose failed"));
        }

        // Copy result back to the typed array
        float* outputData = reinterpret_cast<float*>(outputArray.ArrayBuffer().Data());
        std::memcpy(outputData, output.data(), output.size() * sizeof(float));

        return CreateResultObject(env, true);

    } catch (const std::exception& e) {
        return CreateResultObject(env, false, std::string("Exception: ") + e.what());
    }
}

Napi::Value MatrixWrap::DotProduct(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Expected 2 arguments: arrayA, arrayB").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsTypedArray() || !info[1].IsTypedArray()) {
        Napi::TypeError::New(env, "TypedArray expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto arrayA = info[0].As<Napi::TypedArray>();
        auto arrayB = info[1].As<Napi::TypedArray>();

        std::vector<float> vecA = ConvertToFloatVector(arrayA);
        std::vector<float> vecB = ConvertToFloatVector(arrayB);
        float result = 0.0f;

        auto op_result = MatrixOps::dot_product(vecA.data(), vecB.data(), &result, vecA.size());

        if (!op_result.is_success()) {
            return CreateResultObject(env, false, op_result.error_message.value_or("Dot product failed"));
        }

        Napi::Object result_obj = Napi::Object::New(env);
        result_obj.Set("success", Napi::Boolean::New(env, true));
        result_obj.Set("result", Napi::Number::New(env, result));
        return result_obj;

    } catch (const std::exception& e) {
        return CreateResultObject(env, false, std::string("Exception: ") + e.what());
    }
}

Napi::Value MatrixWrap::Normalize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Expected 2 arguments: array, rows, cols").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsTypedArray() || !info[1].IsNumber() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto array = info[0].As<Napi::TypedArray>();
        size_t rows = info[1].As<Napi::Number>().Uint32Value();
        size_t cols = info[2].As<Napi::Number>().Uint32Value();

        std::vector<float> vec = ConvertToFloatVector(array);

        auto op_result = MatrixOps::normalize(vec.data(), rows, cols);

        if (!op_result.is_success()) {
            return CreateResultObject(env, false, op_result.error_message.value_or("Normalization failed"));
        }

        // Copy normalized result back to the typed array
        float* arrayData = reinterpret_cast<float*>(array.ArrayBuffer().Data());
        std::memcpy(arrayData, vec.data(), vec.size() * sizeof(float));

        return CreateResultObject(env, true);

    } catch (const std::exception& e) {
        return CreateResultObject(env, false, std::string("Exception: ") + e.what());
    }
}

Napi::Value MatrixWrap::CosineSimilarity(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Expected 2 arguments: arrayA, arrayB").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsTypedArray() || !info[1].IsTypedArray()) {
        Napi::TypeError::New(env, "TypedArray expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto arrayA = info[0].As<Napi::TypedArray>();
        auto arrayB = info[1].As<Napi::TypedArray>();

        std::vector<float> vecA = ConvertToFloatVector(arrayA);
        std::vector<float> vecB = ConvertToFloatVector(arrayB);
        float result = 0.0f;

        auto op_result = MatrixOps::cosine_similarity(vecA.data(), vecB.data(), &result, vecA.size());

        if (!op_result.is_success()) {
            return CreateResultObject(env, false, op_result.error_message.value_or("Cosine similarity failed"));
        }

        Napi::Object result_obj = Napi::Object::New(env);
        result_obj.Set("success", Napi::Boolean::New(env, true));
        result_obj.Set("similarity", Napi::Number::New(env, result));
        return result_obj;

    } catch (const std::exception& e) {
        return CreateResultObject(env, false, std::string("Exception: ") + e.what());
    }
}

std::vector<float> MatrixWrap::ConvertToFloatVector(const Napi::TypedArray& array) {
    if (array.TypedArrayType() != napi_float32_array) {
        throw std::runtime_error("Float32Array expected");
    }

    size_t length = array.ElementLength();
    std::vector<float> result(length);

    float* data = reinterpret_cast<float*>(array.ArrayBuffer().Data()) + array.ByteOffset() / sizeof(float);
    std::memcpy(result.data(), data, length * sizeof(float));

    return result;
}

Napi::Object MatrixWrap::CreateResultObject(Napi::Env env, bool success,
                                          const std::string& error_msg,
                                          const std::vector<float>& data) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, success));

    if (!success && !error_msg.empty()) {
        result.Set("error", Napi::String::New(env, error_msg));
    }

    if (!data.empty()) {
        result.Set("data", ConvertFromFloatVector(env, data));
    }

    return result;
}

Napi::TypedArray MatrixWrap::ConvertFromFloatVector(Napi::Env env, const std::vector<float>& vec) {
    auto buffer = Napi::ArrayBuffer::New(env, vec.size() * sizeof(float));
    float* data = reinterpret_cast<float*>(buffer.Data());
    std::memcpy(data, vec.data(), vec.size() * sizeof(float));

    return Napi::Float32Array::New(env, vec.size(), buffer, 0);
}
