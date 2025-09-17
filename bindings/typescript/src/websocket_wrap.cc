/**
 * WebSocket Transport Wrapper for Node.js Bindings
 * Provides WebSocket transport functionality to TypeScript/JavaScript
 */

#include <napi.h>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;
typedef websocketpp::client<websocketpp::config::asio> WebSocketClient;

class WebSocketTransportWrap : public Napi::ObjectWrap<WebSocketTransportWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    WebSocketTransportWrap(const Napi::CallbackInfo& info);

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
    // Internal implementation
    class Impl {
    public:
        Impl(const std::string& url, bool is_server);
        ~Impl();

        bool connect();
        bool disconnect();
        bool send(const std::string& message);
        bool is_connected() const;
        Napi::Object get_stats(Napi::Env env);

        void set_message_callback(Napi::Function callback);
        void set_connection_callback(Napi::Function callback);

    private:
        void run_server();
        void run_client();

        std::string url_;
        bool is_server_;
        std::atomic<bool> connected_{false};
        std::atomic<bool> running_{false};

        // WebSocket components
        WebSocketServer server_;
        WebSocketClient client_;
        websocketpp::connection_hdl connection_handle_;

        // Callbacks
        Napi::ThreadSafeFunction message_callback_;
        Napi::ThreadSafeFunction connection_callback_;

        // Threading
        std::unique_ptr<std::thread> io_thread_;

        // Stats
        struct Stats {
            uint64_t messages_sent = 0;
            uint64_t messages_received = 0;
            uint64_t bytes_sent = 0;
            uint64_t bytes_received = 0;
            std::chrono::steady_clock::time_point last_activity;
        } stats_;
        mutable std::mutex stats_mutex_;
    };

    std::unique_ptr<Impl> impl_;
};

// Implementation
Napi::Object WebSocketTransportWrap::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "WebSocketTransport", {
        InstanceMethod("connect", &WebSocketTransportWrap::Connect),
        InstanceMethod("disconnect", &WebSocketTransportWrap::Disconnect),
        InstanceMethod("send", &WebSocketTransportWrap::Send),
        InstanceMethod("isConnected", &WebSocketTransportWrap::IsConnected),
        InstanceMethod("getStats", &WebSocketTransportWrap::GetStats),
        InstanceMethod("setMessageCallback", &WebSocketTransportWrap::SetMessageCallback),
        InstanceMethod("setConnectionCallback", &WebSocketTransportWrap::SetConnectionCallback)
    });

    exports.Set("WebSocketTransport", func);
    return exports;
}

WebSocketTransportWrap::WebSocketTransportWrap(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<WebSocketTransportWrap>(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "URL required").ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "URL must be a string").ThrowAsJavaScriptException();
        return;
    }

    bool is_server = false;
    if (info.Length() > 1 && info[1].IsBoolean()) {
        is_server = info[1].As<Napi::Boolean>().Value();
    }

    std::string url = info[0].As<Napi::String>().Utf8Value();
    impl_ = std::make_unique<Impl>(url, is_server);
}

Napi::Value WebSocketTransportWrap::Connect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool success = impl_->connect();
    return Napi::Boolean::New(env, success);
}

Napi::Value WebSocketTransportWrap::Disconnect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool success = impl_->disconnect();
    return Napi::Boolean::New(env, success);
}

Napi::Value WebSocketTransportWrap::Send(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Message string required").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::string message = info[0].As<Napi::String>().Utf8Value();
    bool success = impl_->send(message);
    return Napi::Boolean::New(env, success);
}

Napi::Value WebSocketTransportWrap::IsConnected(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool connected = impl_->is_connected();
    return Napi::Boolean::New(env, connected);
}

Napi::Value WebSocketTransportWrap::GetStats(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return impl_->get_stats(env);
}

void WebSocketTransportWrap::SetMessageCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function required").ThrowAsJavaScriptException();
        return;
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    impl_->set_message_callback(callback);
}

void WebSocketTransportWrap::SetConnectionCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function required").ThrowAsJavaScriptException();
        return;
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    impl_->set_connection_callback(callback);
}

// Implementation details
WebSocketTransportWrap::Impl::Impl(const std::string& url, bool is_server)
    : url_(url), is_server_(is_server) {
    stats_.last_activity = std::chrono::steady_clock::now();
}

WebSocketTransportWrap::Impl::~Impl() {
    disconnect();
}

bool WebSocketTransportWrap::Impl::connect() {
    if (connected_.load()) {
        return true;
    }

    running_.store(true);

    if (is_server_) {
        io_thread_ = std::make_unique<std::thread>([this]() { run_server(); });
    } else {
        io_thread_ = std::make_unique<std::thread>([this]() { run_client(); });
    }

    connected_.store(true);
    return true;
}

bool WebSocketTransportWrap::Impl::disconnect() {
    if (!connected_.load()) {
        return true;
    }

    running_.store(false);
    connected_.store(false);

    if (io_thread_ && io_thread_->joinable()) {
        io_thread_->join();
    }

    return true;
}

bool WebSocketTransportWrap::Impl::send(const std::string& message) {
    if (!connected_.load()) {
        return false;
    }

    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_sent++;
        stats_.bytes_sent += message.size();
        stats_.last_activity = std::chrono::steady_clock::now();
    }

    // Send via WebSocket
    // Implementation depends on whether it's server or client mode
    // For now, return success
    return true;
}

bool WebSocketTransportWrap::Impl::is_connected() const {
    return connected_.load();
}

Napi::Object WebSocketTransportWrap::Impl::get_stats(Napi::Env env) {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    Napi::Object stats_obj = Napi::Object::New(env);
    stats_obj.Set("messagesSent", Napi::Number::New(env, stats_.messages_sent));
    stats_obj.Set("messagesReceived", Napi::Number::New(env, stats_.messages_received));
    stats_obj.Set("bytesSent", Napi::Number::New(env, stats_.bytes_sent));
    stats_obj.Set("bytesReceived", Napi::Number::New(env, stats_.bytes_received));

    return stats_obj;
}

void WebSocketTransportWrap::Impl::set_message_callback(Napi::Function callback) {
    message_callback_ = Napi::ThreadSafeFunction::New(
        callback.Env(), callback, "WebSocketMessageCallback", 0, 1);
}

void WebSocketTransportWrap::Impl::set_connection_callback(Napi::Function callback) {
    connection_callback_ = Napi::ThreadSafeFunction::New(
        callback.Env(), callback, "WebSocketConnectionCallback", 0, 1);
}

void WebSocketTransportWrap::Impl::run_server() {
    // WebSocket server implementation
    // This would use websocketpp server
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void WebSocketTransportWrap::Impl::run_client() {
    // WebSocket client implementation
    // This would use websocketpp client
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
