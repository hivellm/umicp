/**
 * UMICP Test Helpers
 * Utility functions and fixtures for testing
 */

#include "test_helpers.h"
#include <random>
#include <thread>
#include <sstream>
#include <iomanip>

namespace umicp::testing {

// ===============================================
// Test Data Generation
// ===============================================

ByteBuffer TestHelpers::generate_random_data(size_t size, uint32_t seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    ByteBuffer data(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }

    return data;
}

std::vector<float> TestHelpers::generate_random_vector(size_t size, uint32_t seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);

    std::vector<float> vec(size);
    for (size_t i = 0; i < size; ++i) {
        vec[i] = dis(gen);
    }

    return vec;
}

Envelope TestHelpers::create_test_envelope(const std::string& from, const std::string& to,
                                          OperationType op) {
    Envelope envelope;
    envelope.version = "1.0";
    envelope.msg_id = "test-msg-" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    envelope.from = from;
    envelope.to = to;
    envelope.op = op;

    // Set current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3)
       << (std::chrono::duration_cast<std::chrono::milliseconds>(
           now.time_since_epoch()).count() % 1000);
    ss << "Z";
    envelope.ts = ss.str();

    return envelope;
}

Frame TestHelpers::create_test_frame(uint64_t stream_id, uint32_t sequence,
                                    const ByteBuffer& payload) {
    Frame frame;
    frame.header.version = 1;
    frame.header.type = static_cast<uint8_t>(OperationType::DATA);
    frame.header.flags = 0;
    frame.header.stream_id = stream_id;
    frame.header.sequence = sequence;
    frame.payload = payload;

    return frame;
}

TransportConfig TestHelpers::create_test_transport_config(TransportType type,
                                                         uint16_t port) {
    TransportConfig config;
    config.type = type;
    config.host = "localhost";
    config.port = port;
    config.path = "/umicp-test";
    config.headers = StringMap{{"User-Agent", "UMICP-Test/1.0"}};

    return config;
}

// ===============================================
// Test Verification Helpers
// ===============================================

bool TestHelpers::vectors_equal(const std::vector<float>& a, const std::vector<float>& b,
                               float tolerance) {
    if (a.size() != b.size()) return false;

    for (size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a[i] - b[i]) > tolerance) {
            return false;
        }
    }

    return true;
}

bool TestHelpers::buffers_equal(const ByteBuffer& a, const ByteBuffer& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

bool TestHelpers::envelopes_equal(const Envelope& a, const Envelope& b) {
    return a.version == b.version &&
           a.msg_id == b.msg_id &&
           a.from == b.from &&
           a.to == b.to &&
           a.op == b.op;
}

bool TestHelpers::frames_equal(const Frame& a, const Frame& b) {
    return a.header.version == b.header.version &&
           a.header.type == b.header.type &&
           a.header.flags == b.header.flags &&
           a.header.stream_id == b.header.stream_id &&
           a.header.sequence == b.header.sequence &&
           buffers_equal(a.payload, b.payload);
}

// ===============================================
// Performance Helpers
// ===============================================

void TestHelpers::sleep_ms(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double TestHelpers::benchmark_function(std::function<void()> func, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        func();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return static_cast<double>(duration.count()) / iterations; // μs per iteration
}

std::string TestHelpers::format_bytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit < 3) {
        size /= 1024.0;
        unit++;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return ss.str();
}

std::string TestHelpers::format_duration(double microseconds) {
    if (microseconds < 1000.0) {
        return std::to_string(static_cast<int>(microseconds)) + " μs";
    } else if (microseconds < 1000000.0) {
        return std::to_string(static_cast<int>(microseconds / 1000.0)) + " ms";
    } else {
        return std::to_string(static_cast<int>(microseconds / 1000000.0)) + " s";
    }
}

// ===============================================
// Mock Transport Implementation
// ===============================================

MockTransport::MockTransport(const TransportConfig& config)
    : config_(config), connected_(false) {
    stats_.last_activity = std::chrono::steady_clock::now();
}

Result<void> MockTransport::connect() {
    if (connected_) {
        return Result<void>(ErrorCode::NETWORK_ERROR, "Already connected");
    }

    TestHelpers::sleep_ms(10); // Simulate connection delay

    connected_ = true;
    stats_.connection_count++;

    if (connection_callback_) {
        connection_callback_(true, "");
    }

    return Result<void>();
}

Result<void> MockTransport::disconnect() {
    if (!connected_) {
        return Result<void>();
    }

    connected_ = false;

    if (connection_callback_) {
        connection_callback_(false, "Disconnected");
    }

    return Result<void>();
}

Result<void> MockTransport::send(const ByteBuffer& data) {
    if (!connected_) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Not connected");
    }

    if (data.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Data cannot be empty");
    }

    stats_.bytes_sent += data.size();
    stats_.messages_sent++;
    stats_.last_activity = std::chrono::steady_clock::now();

    // Store sent data for verification
    sent_data_.push_back(data);

    // Simulate network delay
    TestHelpers::sleep_ms(1);

    return Result<void>();
}

Result<void> MockTransport::send_envelope(const Envelope& envelope) {
    auto json_result = JsonSerializer::serialize_envelope(envelope);
    if (!json_result.is_success()) {
        return Result<void>(json_result.code, json_result.error_message.value());
    }

    ByteBuffer data(json_result.value->begin(), json_result.value->end());
    return send(data);
}

Result<void> MockTransport::send_frame(const Frame& frame) {
    auto frame_result = BinarySerializer::serialize_frame(frame);
    if (!frame_result.is_success()) {
        return Result<void>(frame_result.code, frame_result.error_message.value());
    }

    return send(*frame_result.value);
}

void MockTransport::simulate_receive_message(const ByteBuffer& data) {
    if (connected_ && message_callback_) {
        stats_.bytes_received += data.size();
        stats_.messages_received++;
        stats_.last_activity = std::chrono::steady_clock::now();

        message_callback_(data);
    }
}

} // namespace umicp::testing
