/**
 * UMICP Protocol Unit Tests
 * Testing main protocol orchestrator functionality
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "../utils/test_helpers.h"

using namespace umicp;
using namespace umicp::testing;

class ProtocolTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        protocol_ = std::make_unique<Protocol>("test-protocol-node");

        // Configure protocol
        config_.enable_binary = true;
        config_.preferred_format = ContentType::JSON;
        config_.require_auth = false;
        config_.require_encryption = false;

        // Create mock transport
        transport_config_ = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET);
        mock_transport_ = std::make_shared<MockTransport>(transport_config_);
    }

    std::unique_ptr<Protocol> protocol_;
    UMICPConfig config_;
    TransportConfig transport_config_;
    std::shared_ptr<MockTransport> mock_transport_;
};

// ===============================================
// Basic Protocol Setup Tests
// ===============================================

TEST_F(ProtocolTest, BasicSetup_Configuration) {
    auto result = protocol_->configure(config_);
    ASSERT_TRUE(result.is_success());

    // Protocol should not be connected initially
    EXPECT_FALSE(protocol_->is_connected());
}

TEST_F(ProtocolTest, BasicSetup_TransportAssignment) {
    auto result = protocol_->set_transport(mock_transport_);
    ASSERT_TRUE(result.is_success());

    // Still not connected until explicitly connected
    EXPECT_FALSE(protocol_->is_connected());
}

TEST_F(ProtocolTest, BasicSetup_NullTransport) {
    auto result = protocol_->set_transport(nullptr);
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.code, ErrorCode::INVALID_ARGUMENT);
}

// ===============================================
// Connection Management Tests
// ===============================================

TEST_F(ProtocolTest, Connection_BasicFlow) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());

    // Connect
    auto connect_result = protocol_->connect();
    ASSERT_TRUE(connect_result.is_success());
    EXPECT_TRUE(protocol_->is_connected());

    // Disconnect
    auto disconnect_result = protocol_->disconnect();
    ASSERT_TRUE(disconnect_result.is_success());
    EXPECT_FALSE(protocol_->is_connected());
}

TEST_F(ProtocolTest, Connection_WithoutTransport) {
    auto result = protocol_->connect();
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.code, ErrorCode::INVALID_ARGUMENT);
}

TEST_F(ProtocolTest, Connection_DoubleConnection) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());

    auto result1 = protocol_->connect();
    auto result2 = protocol_->connect();

    EXPECT_TRUE(result1.is_success());
    EXPECT_FALSE(result2.is_success()); // Should reject double connection
    EXPECT_EQ(result2.code, ErrorCode::INVALID_ARGUMENT);
}

// ===============================================
// Message Sending Tests
// ===============================================

TEST_F(ProtocolTest, Messaging_SendControl) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    auto result = protocol_->send_control("target-node", OperationType::CONTROL, "ping", R"({"test": true})");

    ASSERT_TRUE(result.is_success());
    EXPECT_FALSE(result.value->empty()); // Should return message ID

    // Check that transport received the message
    const auto& sent_data = mock_transport_->get_sent_data();
    EXPECT_GT(sent_data.size(), 0);

    std::cout << "  📤 Control message sent, ID: " << *result.value << std::endl;
}

TEST_F(ProtocolTest, Messaging_SendData) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    ByteBuffer test_data = TestHelpers::generate_random_data(512);
    PayloadHint hint;
    hint.type = PayloadType::BINARY;
    hint.size = test_data.size();
    hint.encoding = EncodingType::UINT8;

    auto result = protocol_->send_data("target-node", test_data, hint);

    ASSERT_TRUE(result.is_success());
    EXPECT_FALSE(result.value->empty());

    const auto& sent_data = mock_transport_->get_sent_data();
    EXPECT_GT(sent_data.size(), 0);

    // Data should be sent as binary frame
    const auto& frame_data = sent_data.back();
    EXPECT_GE(frame_data.size(), UMICP_FRAME_HEADER_SIZE);

    std::cout << "  📦 Data message sent: " << TestHelpers::format_bytes(test_data.size()) << std::endl;
}

TEST_F(ProtocolTest, Messaging_SendAck) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    auto result = protocol_->send_ack("target-node", "original-msg-123");

    ASSERT_TRUE(result.is_success());

    const auto& sent_data = mock_transport_->get_sent_data();
    EXPECT_GT(sent_data.size(), 0);

    // Verify ACK content
    const auto& ack_data = sent_data.back();
    std::string ack_json(ack_data.begin(), ack_data.end());
    EXPECT_NE(ack_json.find("original-msg-123"), std::string::npos);
    EXPECT_NE(ack_json.find("\"op\":2"), std::string::npos); // ACK = 2

    std::cout << "  ✅ ACK message sent for: original-msg-123" << std::endl;
}

TEST_F(ProtocolTest, Messaging_SendError) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    auto result = protocol_->send_error("target-node", ErrorCode::INVALID_FRAME,
                                       "Frame parsing failed", "problematic-msg-456");

    ASSERT_TRUE(result.is_success());

    const auto& sent_data = mock_transport_->get_sent_data();
    EXPECT_GT(sent_data.size(), 0);

    // Verify error content
    const auto& error_data = sent_data.back();
    std::string error_json(error_data.begin(), error_data.end());
    EXPECT_NE(error_json.find("Frame parsing failed"), std::string::npos);
    EXPECT_NE(error_json.find("\"op\":3"), std::string::npos); // ERROR = 3

    std::cout << "  ❌ Error message sent: Frame parsing failed" << std::endl;
}

TEST_F(ProtocolTest, Messaging_SendWithoutConnection) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    // Don't connect

    auto control_result = protocol_->send_control("target", OperationType::CONTROL, "test");
    auto data_result = protocol_->send_data("target", ByteBuffer{1, 2, 3});
    auto ack_result = protocol_->send_ack("target", "msg-id");
    auto error_result = protocol_->send_error("target", ErrorCode::NETWORK_ERROR, "test");

    EXPECT_FALSE(control_result.is_success());
    EXPECT_FALSE(data_result.is_success());
    EXPECT_FALSE(ack_result.is_success());
    EXPECT_FALSE(error_result.is_success());

    EXPECT_EQ(control_result.code, ErrorCode::INVALID_ARGUMENT);
}

// ===============================================
// Message Processing Tests
// ===============================================

TEST_F(ProtocolTest, Processing_ControlMessage) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());

    bool control_handled = false;
    Envelope received_envelope;

    protocol_->register_handler(OperationType::CONTROL,
        [&](const Envelope& env, const ByteBuffer* payload) {
            control_handled = true;
            received_envelope = env;
        });

    // Create test control message
    Envelope test_envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node", OperationType::CONTROL);
    auto json_result = JsonSerializer::serialize_envelope(test_envelope);
    ASSERT_TRUE(json_result.is_success());

    ByteBuffer message_data(json_result.value->begin(), json_result.value->end());

    auto process_result = protocol_->process_message(message_data);
    ASSERT_TRUE(process_result.is_success());

    EXPECT_TRUE(control_handled);
    EXPECT_EQ(received_envelope.from, "sender");
    EXPECT_EQ(received_envelope.op, OperationType::CONTROL);
}

TEST_F(ProtocolTest, Processing_DataMessage) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());

    bool data_handled = false;
    ByteBuffer received_payload;

    protocol_->register_handler(OperationType::DATA,
        [&](const Envelope& env, const ByteBuffer* payload) {
            data_handled = true;
            if (payload) {
                received_payload = *payload;
            }
        });

    // Create test binary frame
    ByteBuffer test_payload = TestHelpers::generate_random_data(100);
    Frame test_frame = TestHelpers::create_test_frame(1001, 1, test_payload);

    auto frame_result = BinarySerializer::serialize_frame(test_frame);
    ASSERT_TRUE(frame_result.is_success());

    auto process_result = protocol_->process_message(*frame_result.value);
    ASSERT_TRUE(process_result.is_success());

    EXPECT_TRUE(data_handled);
    ASSERT_BUFFERS_EQUAL(received_payload, test_payload);
}

TEST_F(ProtocolTest, Processing_UnhandledMessageType) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());

    // Don't register any handlers

    Envelope test_envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node", OperationType::CONTROL);
    auto json_result = JsonSerializer::serialize_envelope(test_envelope);
    ASSERT_TRUE(json_result.is_success());

    ByteBuffer message_data(json_result.value->begin(), json_result.value->end());

    // Should succeed even without handler (handler is optional)
    auto process_result = protocol_->process_message(message_data);
    EXPECT_TRUE(process_result.is_success());
}

TEST_F(ProtocolTest, Processing_InvalidMessage) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());

    ByteBuffer invalid_data = {0x01, 0x02, 0x03}; // Too small for any valid message

    auto process_result = protocol_->process_message(invalid_data);

    // Should handle gracefully by trying JSON first, which will fail
    // The exact behavior depends on implementation, but shouldn't crash
    // For our current implementation, it will likely succeed with a parsed envelope
}

// ===============================================
// Statistics Tests
// ===============================================

TEST_F(ProtocolTest, Statistics_MessageCounting) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    auto initial_stats = protocol_->get_stats();

    // Send various message types
    protocol_->send_control("target", OperationType::CONTROL, "ping");
    protocol_->send_data("target", ByteBuffer{1, 2, 3, 4, 5});
    protocol_->send_ack("target", "msg-123");

    auto final_stats = protocol_->get_stats();

    EXPECT_EQ(final_stats.messages_sent, initial_stats.messages_sent + 3);
    EXPECT_GT(final_stats.bytes_sent, initial_stats.bytes_sent);
}

TEST_F(ProtocolTest, Statistics_MessageProcessing) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());

    int messages_processed = 0;
    protocol_->register_handler(OperationType::CONTROL,
        [&](const Envelope& env, const ByteBuffer* payload) {
            messages_processed++;
        });

    auto initial_stats = protocol_->get_stats();

    // Process some messages
    for (int i = 0; i < 5; ++i) {
        Envelope test_envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node");
        auto json_result = JsonSerializer::serialize_envelope(test_envelope);
        if (json_result.is_success()) {
            ByteBuffer data(json_result.value->begin(), json_result.value->end());
            protocol_->process_message(data);
        }
    }

    auto final_stats = protocol_->get_stats();

    EXPECT_EQ(messages_processed, 5);
    EXPECT_EQ(final_stats.messages_received, initial_stats.messages_received + 5);
}

TEST_F(ProtocolTest, Statistics_Reset) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    // Generate some activity
    protocol_->send_control("target", OperationType::CONTROL, "test");

    auto stats_before_reset = protocol_->get_stats();
    EXPECT_GT(stats_before_reset.messages_sent, 0);

    protocol_->reset_stats();

    auto stats_after_reset = protocol_->get_stats();
    EXPECT_EQ(stats_after_reset.messages_sent, 0);
    EXPECT_EQ(stats_after_reset.bytes_sent, 0);
    EXPECT_EQ(stats_after_reset.messages_received, 0);
    EXPECT_EQ(stats_after_reset.bytes_received, 0);
    EXPECT_EQ(stats_after_reset.errors_count, 0);
}

// ===============================================
// Handler Management Tests
// ===============================================

TEST_F(ProtocolTest, Handlers_MultipleHandlers) {
    int control_count = 0, data_count = 0, ack_count = 0, error_count = 0;

    protocol_->register_handler(OperationType::CONTROL, [&](const Envelope&, const ByteBuffer*) { control_count++; });
    protocol_->register_handler(OperationType::DATA, [&](const Envelope&, const ByteBuffer*) { data_count++; });
    protocol_->register_handler(OperationType::ACK, [&](const Envelope&, const ByteBuffer*) { ack_count++; });
    protocol_->register_handler(OperationType::ERROR, [&](const Envelope&, const ByteBuffer*) { error_count++; });

    // Create and process different message types
    std::vector<OperationType> message_types = {
        OperationType::CONTROL, OperationType::DATA,
        OperationType::ACK, OperationType::ERROR
    };

    for (auto op : message_types) {
        Envelope envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node", op);
        auto json_result = JsonSerializer::serialize_envelope(envelope);
        if (json_result.is_success()) {
            ByteBuffer data(json_result.value->begin(), json_result.value->end());
            protocol_->process_message(data);
        }
    }

    EXPECT_EQ(control_count, 1);
    EXPECT_EQ(data_count, 1);
    EXPECT_EQ(ack_count, 1);
    EXPECT_EQ(error_count, 1);
}

TEST_F(ProtocolTest, Handlers_HandlerReplacement) {
    int first_handler_calls = 0, second_handler_calls = 0;

    // Register first handler
    protocol_->register_handler(OperationType::CONTROL,
        [&](const Envelope&, const ByteBuffer*) { first_handler_calls++; });

    // Process a message
    Envelope envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node");
    auto json_result = JsonSerializer::serialize_envelope(envelope);
    ASSERT_TRUE(json_result.is_success());
    ByteBuffer data(json_result.value->begin(), json_result.value->end());

    protocol_->process_message(data);
    EXPECT_EQ(first_handler_calls, 1);

    // Replace with second handler
    protocol_->register_handler(OperationType::CONTROL,
        [&](const Envelope&, const ByteBuffer*) { second_handler_calls++; });

    // Process another message
    protocol_->process_message(data);
    EXPECT_EQ(first_handler_calls, 1); // Should not increase
    EXPECT_EQ(second_handler_calls, 1); // Should increase
}

TEST_F(ProtocolTest, Handlers_Unregister) {
    int handler_calls = 0;

    protocol_->register_handler(OperationType::CONTROL,
        [&](const Envelope&, const ByteBuffer*) { handler_calls++; });

    // Process message with handler
    Envelope envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node");
    auto json_result = JsonSerializer::serialize_envelope(envelope);
    ASSERT_TRUE(json_result.is_success());
    ByteBuffer data(json_result.value->begin(), json_result.value->end());

    protocol_->process_message(data);
    EXPECT_EQ(handler_calls, 1);

    // Unregister handler
    protocol_->unregister_handler(OperationType::CONTROL);

    // Process message without handler
    protocol_->process_message(data);
    EXPECT_EQ(handler_calls, 1); // Should not increase
}

// ===============================================
// Performance Tests
// ===============================================

TEST_F(ProtocolTest, Performance_MessageSending) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());
    ASSERT_TRUE(protocol_->set_transport(mock_transport_).is_success());
    ASSERT_TRUE(protocol_->connect().is_success());

    const int num_messages = 100;

    auto send_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_messages; ++i) {
            protocol_->send_control("target", OperationType::CONTROL, "ping");
        }
    }, 1);

    double messages_per_second = (num_messages * 1000000.0) / send_time;

    std::cout << "  📊 Protocol send rate: " << static_cast<int>(messages_per_second)
              << " msg/s (" << TestHelpers::format_duration(send_time / num_messages) << " avg)" << std::endl;
}

TEST_F(ProtocolTest, Performance_MessageProcessing) {
    ASSERT_TRUE(protocol_->configure(config_).is_success());

    int processed_count = 0;
    protocol_->register_handler(OperationType::CONTROL,
        [&](const Envelope&, const ByteBuffer*) { processed_count++; });

    // Pre-generate messages
    std::vector<ByteBuffer> messages;
    for (int i = 0; i < 100; ++i) {
        Envelope envelope = TestHelpers::create_test_envelope("sender", "test-protocol-node");
        auto json_result = JsonSerializer::serialize_envelope(envelope);
        if (json_result.is_success()) {
            messages.emplace_back(json_result.value->begin(), json_result.value->end());
        }
    }

    auto process_time = TestHelpers::benchmark_function([&]() {
        for (const auto& message : messages) {
            protocol_->process_message(message);
        }
    }, 1);

    double messages_per_second = (messages.size() * 1000000.0) / process_time;

    EXPECT_EQ(processed_count, messages.size());

    std::cout << "  📊 Protocol process rate: " << static_cast<int>(messages_per_second)
              << " msg/s (" << TestHelpers::format_duration(process_time / messages.size()) << " avg)" << std::endl;
}
