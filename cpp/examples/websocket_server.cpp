/**
 * WebSocket Server Example using libwebsockets
 * Simple echo server for testing WebSocket client
 */

#include <libwebsockets.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mutex>

// Global server state
static std::map<struct lws*, std::string> client_messages;
static std::mutex messages_mutex;

// WebSocket callback
static int websocket_callback(struct lws* wsi, enum lws_callback_reasons reason,
                             void* user, void* in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << "🔗 Client connected" << std::endl;
            break;

        case LWS_CALLBACK_RECEIVE:
            {
                std::string message(static_cast<const char*>(in), len);
                std::cout << "📨 Received: " << message << std::endl;

                // Echo the message back
                {
                    std::lock_guard<std::mutex> lock(messages_mutex);
                    client_messages[wsi] = "Echo: " + message;
                }

                // Request write callback
                lws_callback_on_writable(wsi);
            }
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                auto it = client_messages.find(wsi);
                if (it != client_messages.end()) {
                    std::string response = it->second;
                    client_messages.erase(it);

                    // Send response
                    std::vector<unsigned char> buf(LWS_PRE + response.length());
                    memcpy(&buf[LWS_PRE], response.c_str(), response.length());

                    int ret = lws_write(wsi, &buf[LWS_PRE], response.length(), LWS_WRITE_TEXT);
                    if (ret < 0) {
                        std::cerr << "❌ Write failed" << std::endl;
                        return -1;
                    }

                    std::cout << "📤 Sent: " << response << std::endl;
                }
            }
            break;

        case LWS_CALLBACK_CLOSED:
            std::cout << "🔌 Client disconnected" << std::endl;
            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                client_messages.erase(wsi);
            }
            break;

        default:
            break;
    }

    return 0;
}

// Protocol definition
static struct lws_protocols protocols[] = {
    {
        "umicp-protocol",
        websocket_callback,
        0,
        4096,  // rx_buffer_size
        0,     // id
        nullptr, // user
        0      // per_session_data_size
    },
    { nullptr, nullptr, 0, 0, 0, nullptr, 0 } // terminator
};

int main() {
    std::cout << "🚀 UMICP WebSocket Server" << std::endl;
    std::cout << "=========================" << std::endl;

    // Create libwebsockets context
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = 8080;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;

    struct lws_context* context = lws_create_context(&info);
    if (!context) {
        std::cerr << "❌ Failed to create WebSocket context" << std::endl;
        return 1;
    }

    std::cout << "✅ WebSocket server started on port 8080" << std::endl;
    std::cout << "🌐 Endpoint: ws://localhost:8080/umicp" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    // Main event loop
    while (true) {
        int ret = lws_service(context, 50); // 50ms timeout
        if (ret < 0) {
            std::cerr << "❌ Service error: " << ret << std::endl;
            break;
        }
    }

    std::cout << "🔌 Shutting down server..." << std::endl;
    lws_context_destroy(context);

    std::cout << "✅ Server stopped" << std::endl;
    return 0;
}
