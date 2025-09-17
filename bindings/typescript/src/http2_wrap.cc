/**
 * HTTP/2 Transport Wrapper for Node.js Bindings
 * Provides HTTP/2 transport functionality to TypeScript/JavaScript
 */

#include <napi.h>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <nghttp2/nghttp2.h>

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
    // Internal implementation
    class Impl {
    public:
        Impl(const std::string& url);
        ~Impl();

        bool connect();
        bool disconnect();
        bool send(const std::string& message);
        bool is_connected() const;
        Napi::Object get_stats(Napi::Env env);

        void set_message_callback(Napi::Function callback);
        void set_connection_callback(Napi::Function callback);

    private:
        void run_session();
        static ssize_t send_callback(nghttp2_session* session, const uint8_t* data,
                                   size_t length, int flags, void* user_data);
        static ssize_t recv_callback(nghttp2_session* session, uint8_t* buf,
                                   size_t length, int flags, void* user_data);
        static int on_frame_recv_callback(nghttp2_session* session,
                                        const nghttp2_frame* frame, void* user_data);
        static int on_data_chunk_recv_callback(nghttp2_session* session, uint8_t flags,
                                             int32_t stream_id, const uint8_t* data,
                                             size_t len, void* user_data);

        std::string url_;
        std::atomic<bool> connected_{false};
        std::atomic<bool> running_{false};

        // HTTP/2 session
        nghttp2_session* session_;
        int32_t stream_id_;

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
Napi::Object HTTP2TransportWrap::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "HTTP2Transport", {
        InstanceMethod("connect", &HTTP2TransportWrap::Connect),
        InstanceMethod("disconnect", &HTTP2TransportWrap::Disconnect),
        InstanceMethod("send", &HTTP2TransportWrap::Send),
        InstanceMethod("isConnected", &HTTP2TransportWrap::IsConnected),
        InstanceMethod("getStats", &HTTP2TransportWrap::GetStats),
        InstanceMethod("setMessageCallback", &HTTP2TransportWrap::SetMessageCallback),
        InstanceMethod("setConnectionCallback", &HTTP2TransportWrap::SetConnectionCallback)
    });

    exports.Set("HTTP2Transport", func);
    return exports;
}

HTTP2TransportWrap::HTTP2TransportWrap(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<HTTP2TransportWrap>(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "URL required").ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "URL must be a string").ThrowAsJavaScriptException();
        return;
    }

    std::string url = info[0].As<Napi::String>().Utf8Value();
    impl_ = std::make_unique<Impl>(url);
}

Napi::Value HTTP2TransportWrap::Connect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool success = impl_->connect();
    return Napi::Boolean::New(env, success);
}

Napi::Value HTTP2TransportWrap::Disconnect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool success = impl_->disconnect();
    return Napi::Boolean::New(env, success);
}

Napi::Value HTTP2TransportWrap::Send(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Message string required").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::string message = info[0].As<Napi::String>().Utf8Value();
    bool success = impl_->send(message);
    return Napi::Boolean::New(env, success);
}

Napi::Value HTTP2TransportWrap::IsConnected(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool connected = impl_->is_connected();
    return Napi::Boolean::New(env, connected);
}

Napi::Value HTTP2TransportWrap::GetStats(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return impl_->get_stats(env);
}

void HTTP2TransportWrap::SetMessageCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function required").ThrowAsJavaScriptException();
        return;
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    impl_->set_message_callback(callback);
}

void HTTP2TransportWrap::SetConnectionCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Callback function required").ThrowAsJavaScriptException();
        return;
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    impl_->set_connection_callback(callback);
}

// Implementation details
HTTP2TransportWrap::Impl::Impl(const std::string& url)
    : url_(url), session_(nullptr), stream_id_(-1) {
    stats_.last_activity = std::chrono::steady_clock::now();
}

HTTP2TransportWrap::Impl::~Impl() {
    disconnect();
}

bool HTTP2TransportWrap::Impl::connect() {
    if (connected_.load()) {
        return true;
    }

    running_.store(true);

    // Initialize nghttp2 session
    nghttp2_session_callbacks* callbacks;
    nghttp2_session_callbacks_new(&callbacks);

    nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);
    nghttp2_session_callbacks_set_recv_callback(callbacks, recv_callback);
    nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, on_frame_recv_callback);
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, on_data_chunk_recv_callback);

    nghttp2_session_client_new(&session_, callbacks, this);
    nghttp2_session_callbacks_del(callbacks);

    io_thread_ = std::make_unique<std::thread>([this]() { run_session(); });

    connected_.store(true);
    return true;
}

bool HTTP2TransportWrap::Impl::disconnect() {
    if (!connected_.load()) {
        return true;
    }

    running_.store(false);
    connected_.store(false);

    if (session_) {
        nghttp2_session_del(session_);
        session_ = nullptr;
    }

    if (io_thread_ && io_thread_->joinable()) {
        io_thread_->join();
    }

    return true;
}

bool HTTP2TransportWrap::Impl::send(const std::string& message) {
    if (!connected_.load() || !session_) {
        return false;
    }

    // Submit data to HTTP/2 stream
    nghttp2_data_provider data_provider;
    data_provider.source.ptr = (void*)message.c_str();
    data_provider.read_callback = [](nghttp2_session* session, int32_t stream_id,
                                   uint8_t* buf, size_t length,
                                   uint32_t* data_flags,
                                   nghttp2_data_source* source,
                                   void* user_data) -> ssize_t {
        const char* data = (const char*)source->ptr;
        size_t data_len = strlen(data);

        if (length > data_len) {
            length = data_len;
        }

        memcpy(buf, data, length);
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;

        return length;
    };

    int rv = nghttp2_submit_data(session_, NGHTTP2_FLAG_END_STREAM,
                               stream_id_, &data_provider);

    if (rv != 0) {
        return false;
    }

    // Send pending frames
    nghttp2_session_send(session_);

    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_sent++;
        stats_.bytes_sent += message.size();
        stats_.last_activity = std::chrono::steady_clock::now();
    }

    return true;
}

bool HTTP2TransportWrap::Impl::is_connected() const {
    return connected_.load();
}

Napi::Object HTTP2TransportWrap::Impl::get_stats(Napi::Env env) {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    Napi::Object stats_obj = Napi::Object::New(env);
    stats_obj.Set("messagesSent", Napi::Number::New(env, stats_.messages_sent));
    stats_obj.Set("messagesReceived", Napi::Number::New(env, stats_.messages_received));
    stats_obj.Set("bytesSent", Napi::Number::New(env, stats_.bytes_sent));
    stats_obj.Set("bytesReceived", Napi::Number::New(env, stats_.bytes_received));

    return stats_obj;
}

void HTTP2TransportWrap::Impl::set_message_callback(Napi::Function callback) {
    message_callback_ = Napi::ThreadSafeFunction::New(
        callback.Env(), callback, "HTTP2MessageCallback", 0, 1);
}

void HTTP2TransportWrap::Impl::set_connection_callback(Napi::Function callback) {
    connection_callback_ = Napi::ThreadSafeFunction::New(
        callback.Env(), callback, "HTTP2ConnectionCallback", 0, 1);
}

void HTTP2TransportWrap::Impl::run_session() {
    // HTTP/2 session management loop
    // This would handle the HTTP/2 protocol state machine
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Static callback implementations
ssize_t HTTP2TransportWrap::Impl::send_callback(nghttp2_session* session,
                                              const uint8_t* data,
                                              size_t length, int flags,
                                              void* user_data) {
    // Send data to network (would need actual socket implementation)
    return length;
}

ssize_t HTTP2TransportWrap::Impl::recv_callback(nghttp2_session* session,
                                              uint8_t* buf, size_t length,
                                              int flags, void* user_data) {
    // Receive data from network (would need actual socket implementation)
    return 0;
}

int HTTP2TransportWrap::Impl::on_frame_recv_callback(nghttp2_session* session,
                                                   const nghttp2_frame* frame,
                                                   void* user_data) {
    Impl* impl = static_cast<Impl*>(user_data);

    if (frame->hd.type == NGHTTP2_DATA && frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        // Data frame received
        {
            std::lock_guard<std::mutex> lock(impl->stats_mutex_);
            impl->stats_.messages_received++;
            impl->stats_.last_activity = std::chrono::steady_clock::now();
        }
    }

    return 0;
}

int HTTP2TransportWrap::Impl::on_data_chunk_recv_callback(nghttp2_session* session,
                                                        uint8_t flags,
                                                        int32_t stream_id,
                                                        const uint8_t* data,
                                                        size_t len,
                                                        void* user_data) {
    Impl* impl = static_cast<Impl*>(user_data);

    {
        std::lock_guard<std::mutex> lock(impl->stats_mutex_);
        impl->stats_.bytes_received += len;
    }

    return 0;
}
