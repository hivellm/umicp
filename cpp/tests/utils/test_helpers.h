/**
 * UMICP Test Helpers Header
 * Utility functions and fixtures for testing
 */

#ifndef UMICP_TEST_HELPERS_H
#define UMICP_TEST_HELPERS_H

#include <gtest/gtest.h>
#include "umicp_types.h"
#include "serialization.h"
#include "transport.h"
#include <functional>
#include <vector>
#include <chrono>

namespace umicp::testing {

class TestHelpers {
public:
    // Data generation
    static ByteBuffer generate_random_data(size_t size, uint32_t seed = 12345);
    static std::vector<float> generate_random_vector(size_t size, uint32_t seed = 12345);
    static Envelope create_test_envelope(const std::string& from, const std::string& to,
                                       OperationType op = OperationType::CONTROL);
    static Frame create_test_frame(uint64_t stream_id, uint32_t sequence,
                                 const ByteBuffer& payload);
    static TransportConfig create_test_transport_config(TransportType type,
                                                       uint16_t port = 8080);

    // Verification helpers
    static bool vectors_equal(const std::vector<float>& a, const std::vector<float>& b,
                            float tolerance = 0.001f);
    static bool buffers_equal(const ByteBuffer& a, const ByteBuffer& b);
    static bool envelopes_equal(const Envelope& a, const Envelope& b);
    static bool frames_equal(const Frame& a, const Frame& b);

    // Performance helpers
    static void sleep_ms(int milliseconds);
    static double benchmark_function(std::function<void()> func, int iterations = 1000);
    static std::string format_bytes(size_t bytes);
    static std::string format_duration(double microseconds);
};

// Mock Transport for testing
class MockTransport : public Transport {
public:
    explicit MockTransport(const TransportConfig& config);
    virtual ~MockTransport() = default;

    // Transport interface
    Result<void> connect() override;
    Result<void> disconnect() override;
    bool is_connected() const override { return connected_; }

    Result<void> send(const ByteBuffer& data) override;
    Result<void> send_envelope(const Envelope& envelope) override;
    Result<void> send_frame(const Frame& frame) override;

    Result<void> configure(const TransportConfig& config) override {
        config_ = config;
        return Result<void>();
    }

    TransportConfig get_config() const override { return config_; }

    void set_message_callback(MessageCallback callback) override {
        message_callback_ = callback;
    }

    void set_connection_callback(ConnectionCallback callback) override {
        connection_callback_ = callback;
    }

    void set_error_callback(ErrorCallback callback) override {
        error_callback_ = callback;
    }

    TransportStats get_stats() const override { return stats_; }
    void reset_stats() override {
        stats_ = TransportStats{};
        stats_.last_activity = std::chrono::steady_clock::now();
    }

    TransportType get_type() const override { return config_.type; }
    std::string get_endpoint() const override {
        return "mock://" + config_.host + ":" + std::to_string(config_.port);
    }

    // Test-specific methods
    void simulate_receive_message(const ByteBuffer& data);
    const std::vector<ByteBuffer>& get_sent_data() const { return sent_data_; }
    void clear_sent_data() { sent_data_.clear(); }

private:
    TransportConfig config_;
    bool connected_;
    TransportStats stats_;
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    ErrorCallback error_callback_;
    std::vector<ByteBuffer> sent_data_;
};

// Test fixtures
class UMICPTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
    }

    void TearDown() override {
        // Common cleanup
    }
};

// Performance test fixture
class UMICPPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "🏃 Performance Test: " << ::testing::UnitTest::GetInstance()
                     ->current_test_info()->name() << std::endl;
    }

    void TearDown() override {
        // Performance cleanup
    }

    void PrintResults(const std::string& operation, double time_us, size_t operations = 1) {
        std::cout << "  📊 " << operation << ": "
                  << TestHelpers::format_duration(time_us);
        if (operations > 1) {
            std::cout << " (" << operations << " ops, "
                      << TestHelpers::format_duration(time_us / operations) << " avg)";
        }
        std::cout << std::endl;
    }
};

// Macros for common assertions
#define ASSERT_VECTORS_EQUAL(a, b, tolerance) \
    ASSERT_TRUE(TestHelpers::vectors_equal(a, b, tolerance)) \
    << "Vectors are not equal within tolerance " << tolerance

#define ASSERT_BUFFERS_EQUAL(a, b) \
    ASSERT_TRUE(TestHelpers::buffers_equal(a, b)) \
    << "ByteBuffers are not equal"

#define ASSERT_ENVELOPES_EQUAL(a, b) \
    ASSERT_TRUE(TestHelpers::envelopes_equal(a, b)) \
    << "Envelopes are not equal"

#define ASSERT_FRAMES_EQUAL(a, b) \
    ASSERT_TRUE(TestHelpers::frames_equal(a, b)) \
    << "Frames are not equal"

#define EXPECT_PERFORMANCE_BETTER_THAN(operation, max_time_us) \
    do { \
        double actual_time = TestHelpers::benchmark_function(operation); \
        EXPECT_LT(actual_time, max_time_us) \
            << "Performance test failed: " << TestHelpers::format_duration(actual_time) \
            << " > " << TestHelpers::format_duration(max_time_us); \
        PrintResults(#operation, actual_time); \
    } while(0)

} // namespace umicp::testing

#endif // UMICP_TEST_HELPERS_H
