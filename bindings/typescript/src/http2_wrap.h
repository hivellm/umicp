/**
 * HTTP/2 Transport Wrapper Header
 * Node.js bindings for HTTP/2 transport functionality
 */

#pragma once

#include <napi.h>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

class HTTP2TransportWrap : public Napi::ObjectWrap<HTTP2TransportWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    HTTP2TransportWrap(const Napi::CallbackInfo& info);

private:
    // JavaScript methods
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value Disconnect(const Napi::CallbackInfo& info);
    Napi::Value Send(const Napi::CallbackInfo& info);
    Napi::Value IsConnected(const Napi::CallbackInfo& info);
    Napi::Value GetStats(const Napi::CallbackInfo& info);
    void SetMessageCallback(const Napi::CallbackInfo& info);
    void SetConnectionCallback(const Napi::CallbackInfo& info);

private:
    // Internal implementation class
    class Impl;
    std::unique_ptr<Impl> impl_;
};
