/**
 * UMICP Enhanced Comprehensive Test
 * Tests the updated C++ implementation with transport and serialization
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <thread>

// UMICP headers
#include "../include/umicp_types.h"
#include "../include/protocol.h"
#include "../include/transport.h"
#include "../include/serialization.h"
#include "../include/matrix_ops.h"

using namespace umicp;

// Test utilities
void print_test_header(const std::string& test_name) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "🧪 " << test_name << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void print_result(const std::string& test, bool success, const std::string& details = "") {
    std::cout << "[" << (success ? "✅ PASS" : "❌ FAIL") << "] " << test;
    if (!details.empty()) {
        std::cout << " - " << details;
    }
    std::cout << std::endl;
}

// Test 1: Basic Matrix Operations
bool test_matrix_operations() {
    print_test_header("Matrix Operations Test");

    bool all_passed = true;

    // Test vector addition
    std::vector<float> a = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> b = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> result(4);

    auto add_result = MatrixOps::add(a.data(), b.data(), result.data(), 1, 4);
    bool add_success = add_result.is_success() &&
                      result[0] == 6.0f && result[1] == 8.0f &&
                      result[2] == 10.0f && result[3] == 12.0f;

    print_result("Vector Addition", add_success,
                 "Expected [6,8,10,12], got [" + std::to_string(result[0]) + "," +
                 std::to_string(result[1]) + "," + std::to_string(result[2]) + "," +
                 std::to_string(result[3]) + "]");
    all_passed &= add_success;

    // Test dot product
    float dot_result = 0.0f;
    auto dot_op_result = MatrixOps::dot_product(a.data(), b.data(), &dot_result, 4);
    bool dot_success = dot_op_result.is_success() && std::abs(dot_result - 70.0f) < 0.001f;

    print_result("Dot Product", dot_success,
                 "Expected 70.0, got " + std::to_string(dot_result));
    all_passed &= dot_success;

    // Test normalization
    std::vector<float> norm_vec = {3.0f, 4.0f}; // Length = 5
    auto norm_result = MatrixOps::normalize(norm_vec.data(), 1, 2);
    bool norm_success = norm_result.is_success() &&
                       std::abs(norm_vec[0] - 0.6f) < 0.001f &&
                       std::abs(norm_vec[1] - 0.8f) < 0.001f;

    print_result("Vector Normalization", norm_success,
                 "Expected [0.6,0.8], got [" + std::to_string(norm_vec[0]) + "," +
                 std::to_string(norm_vec[1]) + "]");
    all_passed &= norm_success;

    return all_passed;
}

// Test 2: Serialization
bool test_serialization() {
    print_test_header("Serialization Test");

    bool all_passed = true;

    // Test envelope serialization
    Envelope envelope;
    envelope.version = "1.0";
    envelope.msg_id = "test-msg-123";
    envelope.ts = "2025-09-10T03:00:00.000Z";
    envelope.from = "test-sender";
    envelope.to = "test-receiver";
    envelope.op = OperationType::CONTROL;
    envelope.capabilities = StringMap{{"command", "ping"}, {"version", "1.0"}};

    auto json_result = JsonSerializer::serialize_envelope(envelope);
    bool json_success = json_result.is_success() && !json_result.value->empty();

    print_result("JSON Envelope Serialization", json_success,
                 json_success ? "Size: " + std::to_string(json_result.value->size()) + " bytes" :
                               json_result.error_message.value_or("Unknown error"));
    all_passed &= json_success;

    if (json_success) {
        // Test envelope deserialization
        auto deserialize_result = JsonSerializer::deserialize_envelope(*json_result.value);
        bool deserialize_success = deserialize_result.is_success() &&
                                  deserialize_result.value->msg_id == envelope.msg_id &&
                                  deserialize_result.value->from == envelope.from &&
                                  deserialize_result.value->to == envelope.to;

        print_result("JSON Envelope Deserialization", deserialize_success);
        all_passed &= deserialize_success;
    }

    // Test frame serialization
    Frame frame;
    frame.header.version = 1;
    frame.header.type = static_cast<uint8_t>(OperationType::DATA);
    frame.header.flags = 0;
    frame.header.stream_id = 12345;
    frame.header.sequence = 1;
    frame.payload = {0x01, 0x02, 0x03, 0x04, 0x05};

    auto frame_result = BinarySerializer::serialize_frame(frame);
    bool frame_success = frame_result.is_success() &&
                        frame_result.value->size() == (UMICP_FRAME_HEADER_SIZE + 5);

    print_result("Binary Frame Serialization", frame_success,
                 frame_success ? "Size: " + std::to_string(frame_result.value->size()) + " bytes" :
                               frame_result.error_message.value_or("Unknown error"));
    all_passed &= frame_success;

    if (frame_success) {
        // Test frame deserialization
        auto frame_deserialize_result = BinarySerializer::deserialize_frame(*frame_result.value);
        bool frame_deserialize_success = frame_deserialize_result.is_success() &&
                                        frame_deserialize_result.value->header.stream_id == 12345 &&
                                        frame_deserialize_result.value->payload.size() == 5;

        print_result("Binary Frame Deserialization", frame_deserialize_success);
        all_passed &= frame_deserialize_success;
    }

    return all_passed;
}

// Test 3: Transport Layer
bool test_transport() {
    print_test_header("Transport Layer Test");

    bool all_passed = true;

    // Create WebSocket transport configuration
    TransportConfig config;
    config.type = TransportType::WEBSOCKET;
    config.host = "localhost";
    config.port = 8080;
    config.path = "/umicp";

    auto transport = TransportFactory::create_websocket(config);
    bool transport_created = (transport != nullptr);

    print_result("WebSocket Transport Creation", transport_created);
    all_passed &= transport_created;

    if (transport_created) {
        // Test connection
        bool message_received = false;
        std::string received_message;

        transport->set_message_callback([&](const ByteBuffer& data) {
            message_received = true;
            received_message = std::string(data.begin(), data.end());
        });

        auto connect_result = transport->connect();
        bool connect_success = connect_result.is_success();

        print_result("Transport Connection", connect_success,
                     connect_success ? "Connected to " + transport->get_endpoint() :
                                     connect_result.error_message.value_or("Unknown error"));
        all_passed &= connect_success;

        if (connect_success) {
            // Test sending data
            std::string test_message = R"({"type":"test","data":"hello world"})";
            ByteBuffer test_data(test_message.begin(), test_message.end());

            auto send_result = transport->send(test_data);
            bool send_success = send_result.is_success();

            print_result("Transport Send", send_success);
            all_passed &= send_success;

            // Wait a bit for any responses (in real transport)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Check stats
            auto stats = transport->get_stats();
            bool stats_valid = (stats.messages_sent.load() > 0) && (stats.bytes_sent.load() > 0);

            print_result("Transport Statistics", stats_valid,
                        "Messages sent: " + std::to_string(stats.messages_sent.load()) +
                        ", Bytes sent: " + std::to_string(stats.bytes_sent.load()));

            // Disconnect
            auto disconnect_result = transport->disconnect();
            bool disconnect_success = disconnect_result.is_success();

            print_result("Transport Disconnection", disconnect_success);
            all_passed &= disconnect_success;
        }
    }

    return all_passed;
}

// Test 4: Protocol Integration
bool test_protocol_integration() {
    print_test_header("Protocol Integration Test");

    bool all_passed = true;

    // Create protocol instance
    Protocol protocol("test-model-enhanced");

    // Configure protocol
    UMICPConfig config;
    config.enable_binary = true;
    config.preferred_format = ContentType::JSON;
    config.require_auth = false;

    auto configure_result = protocol.configure(config);
    bool configure_success = configure_result.is_success();

    print_result("Protocol Configuration", configure_success);
    all_passed &= configure_success;

    // Create and set transport
    TransportConfig transport_config;
    transport_config.type = TransportType::WEBSOCKET;
    auto transport = TransportFactory::create_websocket(transport_config);

    auto set_transport_result = protocol.set_transport(transport);
    bool set_transport_success = set_transport_result.is_success();

    print_result("Set Transport", set_transport_success);
    all_passed &= set_transport_success;

    // Test message handling
    bool control_handled = false;
    bool data_handled = false;

    protocol.register_handler(OperationType::CONTROL,
        [&](const Envelope& env, const ByteBuffer* payload) {
            control_handled = true;
            std::cout << "    📨 Control message received from: " << env.from << std::endl;
        });

    protocol.register_handler(OperationType::DATA,
        [&](const Envelope& env, const ByteBuffer* payload) {
            data_handled = true;
            std::cout << "    📦 Data message received: " <<
                        (payload ? std::to_string(payload->size()) + " bytes" : "no payload") << std::endl;
        });

    print_result("Message Handlers Registration", true);

    // Test connection (simulated)
    auto connect_result = protocol.connect();
    bool connect_success = connect_result.is_success();

    print_result("Protocol Connection", connect_success,
                 connect_success ? "Connected" : connect_result.error_message.value_or("Unknown error"));
    all_passed &= connect_success;

    if (connect_success) {
        // Test sending control message
        auto control_result = protocol.send_control("test-receiver", OperationType::CONTROL, "ping", R"({"test":true})");
        bool control_success = control_result.is_success();

        print_result("Send Control Message", control_success,
                     control_success ? "Message ID: " + *control_result.value :
                                     control_result.error_message.value_or("Unknown error"));
        all_passed &= control_success;

        // Test sending data message
        ByteBuffer test_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
        PayloadHint hint;
        hint.type = PayloadType::BINARY;
        hint.size = test_data.size();

        auto data_result = protocol.send_data("test-receiver", test_data, hint);
        bool data_success = data_result.is_success();

        print_result("Send Data Message", data_success,
                     data_success ? "Message ID: " + *data_result.value :
                                   data_result.error_message.value_or("Unknown error"));
        all_passed &= data_success;

        // Test ACK
        auto ack_result = protocol.send_ack("test-receiver", "test-msg-id");
        bool ack_success = ack_result.is_success();

        print_result("Send ACK Message", ack_success);
        all_passed &= ack_success;

        // Check statistics
        auto stats = protocol.get_stats();
        bool stats_valid = (stats.messages_sent > 0);

        print_result("Protocol Statistics", stats_valid,
                    "Messages sent: " + std::to_string(stats.messages_sent) +
                    ", Bytes sent: " + std::to_string(stats.bytes_sent));

        // Disconnect
        auto disconnect_result = protocol.disconnect();
        print_result("Protocol Disconnection", disconnect_result.is_success());
    }

    return all_passed;
}

// Test 5: Hash and Utilities
bool test_utilities() {
    print_test_header("Utilities Test");

    bool all_passed = true;

    // Test SHA256
    std::string test_string = "Hello, UMICP!";
    auto hash_result = HashUtils::sha256_hex(test_string);
    bool hash_success = hash_result.is_success() && hash_result.value->length() == 64;

    print_result("SHA256 Hash", hash_success,
                 hash_success ? "Hash: " + hash_result.value->substr(0, 16) + "..." :
                               hash_result.error_message.value_or("Unknown error"));
    all_passed &= hash_success;

    // Test Base64
    ByteBuffer test_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    auto base64_result = Base64Utils::encode(test_data);
    bool base64_success = base64_result.is_success() && *base64_result.value == "SGVsbG8=";

    print_result("Base64 Encoding", base64_success,
                 base64_success ? "Encoded: " + *base64_result.value :
                                 base64_result.error_message.value_or("Unknown error"));
    all_passed &= base64_success;

    if (base64_success) {
        // Test Base64 decode
        auto decode_result = Base64Utils::decode(*base64_result.value);
        bool decode_success = decode_result.is_success() &&
                             decode_result.value->size() == test_data.size() &&
                             std::equal(decode_result.value->begin(), decode_result.value->end(), test_data.begin());

        print_result("Base64 Decoding", decode_success);
        all_passed &= decode_success;
    }

    return all_passed;
}

int main() {
    std::cout << "🚀 UMICP Enhanced Comprehensive Test Suite" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Testing updated C++ implementation with transport and serialization" << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    bool all_tests_passed = true;

    // Run all tests
    all_tests_passed &= test_matrix_operations();
    all_tests_passed &= test_serialization();
    all_tests_passed &= test_transport();
    all_tests_passed &= test_protocol_integration();
    all_tests_passed &= test_utilities();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Print final results
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "🏁 Test Suite Complete" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    std::cout << "Overall Result: " << (all_tests_passed ? "✅ ALL TESTS PASSED" : "❌ SOME TESTS FAILED") << std::endl;
    std::cout << "Execution Time: " << duration.count() << " ms" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    return all_tests_passed ? 0 : 1;
}
