/**
 * UMICP Matrix Operations Node.js Wrapper Header
 */

#ifndef MATRIX_WRAP_H
#define MATRIX_WRAP_H

#include <napi.h>

class MatrixWrap : public Napi::ObjectWrap<MatrixWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    MatrixWrap(const Napi::CallbackInfo& info);

private:
    // Matrix operations
    Napi::Value Add(const Napi::CallbackInfo& info);
    Napi::Value Multiply(const Napi::CallbackInfo& info);
    Napi::Value Transpose(const Napi::CallbackInfo& info);
    Napi::Value DotProduct(const Napi::CallbackInfo& info);
    Napi::Value Normalize(const Napi::CallbackInfo& info);
    Napi::Value CosineSimilarity(const Napi::CallbackInfo& info);

    // Helper methods
    std::vector<float> ConvertToFloatVector(const Napi::TypedArray& array);
    Napi::TypedArray ConvertFromFloatVector(Napi::Env env, const std::vector<float>& vec);
    Napi::Object CreateResultObject(Napi::Env env, bool success,
                                   const std::string& error_msg = "",
                                   const std::vector<float>& data = {});
};

#endif // MATRIX_WRAP_H
