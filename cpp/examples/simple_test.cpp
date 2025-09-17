/**
 * UMICP Simple Test
 * Basic functionality test without external dependencies
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

// Simple test structures (subset of UMICP types)
enum class OperationType { CONTROL = 0, DATA = 1, ACK = 2, ERROR = 3 };

struct Envelope {
    std::string version = "1.0";
    std::string msg_id;
    std::string ts;
    std::string from;
    std::string to;
    OperationType op = OperationType::CONTROL;
};

// Simple envelope builder
class EnvelopeBuilder {
public:
    Envelope envelope;

    EnvelopeBuilder() {
        envelope.version = "1.0";
        envelope.msg_id = generate_simple_id();
        envelope.ts = get_current_timestamp();
        envelope.op = OperationType::CONTROL;
    }

    EnvelopeBuilder& from(const std::string& from) {
        envelope.from = from;
        return *this;
    }

    EnvelopeBuilder& to(const std::string& to) {
        envelope.to = to;
        return *this;
    }

    EnvelopeBuilder& operation(OperationType op) {
        envelope.op = op;
        return *this;
    }

    Envelope build() {
        return envelope;
    }

private:
    std::string generate_simple_id() {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        return "msg-" + std::to_string(timestamp);
    }

    std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3)
           << (std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch()).count() % 1000);
        ss << "Z";
        return ss.str();
    }
};

// Simple matrix operations (without SIMD for basic testing)
class SimpleMatrixOps {
public:
    static void add(const float* a, const float* b, float* result, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            result[i] = a[i] + b[i];
        }
    }

    static float dot_product(const float* a, const float* b, size_t size) {
        float result = 0.0f;
        for (size_t i = 0; i < size; ++i) {
            result += a[i] * b[i];
        }
        return result;
    }
};

int main() {
    std::cout << "🧪 UMICP Simple Test" << std::endl;
    std::cout << "====================" << std::endl;

    // Test 1: Envelope Builder
    std::cout << "\n📝 Test 1: Envelope Builder" << std::endl;
    std::cout << "---------------------------" << std::endl;

    EnvelopeBuilder builder;
    Envelope envelope = builder
        .from("test-model-a")
        .to("test-model-b")
        .operation(OperationType::DATA)
        .build();

    std::cout << "✅ Envelope created:" << std::endl;
    std::cout << "   Version: " << envelope.version << std::endl;
    std::cout << "   From: " << envelope.from << std::endl;
    std::cout << "   To: " << envelope.to << std::endl;
    std::cout << "   Operation: " << static_cast<int>(envelope.op) << std::endl;
    std::cout << "   Message ID: " << envelope.msg_id << std::endl;
    std::cout << "   Timestamp: " << envelope.ts << std::endl;

    // Test 2: Matrix Operations
    std::cout << "\n🧮 Test 2: Matrix Operations" << std::endl;
    std::cout << "----------------------------" << std::endl;

    const size_t vector_size = 1000;
    std::vector<float> a(vector_size);
    std::vector<float> b(vector_size);
    std::vector<float> result(vector_size);

    // Initialize test data
    for (size_t i = 0; i < vector_size; ++i) {
        a[i] = static_cast<float>(i) * 0.01f;
        b[i] = static_cast<float>(i % 10) * 0.1f;
    }

    // Time the operations
    auto start_time = std::chrono::high_resolution_clock::now();

    // Vector addition
    SimpleMatrixOps::add(a.data(), b.data(), result.data(), vector_size);

    // Dot product
    float dot_result = SimpleMatrixOps::dot_product(a.data(), b.data(), vector_size);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "✅ Operations completed in " << duration.count() << " μs" << std::endl;
    std::cout << "📊 Vector size: " << vector_size << " elements" << std::endl;
    std::cout << "📈 Sample result[0]: " << result[0] << " (expected: " << (a[0] + b[0]) << ")" << std::endl;
    std::cout << "🔢 Dot product: " << dot_result << std::endl;

    // Test 3: Performance estimation
    std::cout << "\n⚡ Test 3: Performance Estimation" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    const size_t perf_size = 10000;
    std::vector<float> perf_a(perf_size);
    std::vector<float> perf_b(perf_size);
    std::vector<float> perf_result(perf_size);

    for (size_t i = 0; i < perf_size; ++i) {
        perf_a[i] = static_cast<float>(i % 100) * 0.01f;
        perf_b[i] = static_cast<float>(i % 50) * 0.02f;
    }

    start_time = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < 1000; ++iter) {
        SimpleMatrixOps::add(perf_a.data(), perf_b.data(), perf_result.data(), perf_size);
    }

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "⏱️  1000 vector additions (" << perf_size << " elements each)" << std::endl;
    std::cout << "📊 Total time: " << duration.count() << " ms" << std::endl;
    std::cout << "🚀 Time per operation: " << (duration.count() / 1000.0) << " ms" << std::endl;

    std::cout << "\n🎉 UMICP Simple Test Completed Successfully!" << std::endl;
    std::cout << "============================================" << std::endl;

    return 0;
}
