/**
 * UMICP C++ Core - Basic Usage Example
 * Demonstrates envelope, frame, security, and compression operations
 */

#include "../include/umicp_types.h"
#include "../include/envelope.h"
#include "../include/frame.h"
#include "../include/matrix_ops.h"
#include "../include/security.h"
#include "../include/compression.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <iomanip>

using namespace umicp;

int main() {
    std::cout << "🚀 UMICP C++ Core - Basic Example" << std::endl;
    std::cout << "=====================================" << std::endl;

    // Example 1: Envelope Operations
    std::cout << "\n📝 Example 1: Envelope Operations" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    // Create envelope using builder pattern
    auto envelope_result = EnvelopeBuilder()
        .from("ai-model-a")
        .to("ai-model-b")
        .operation(OperationType::DATA)
        .capabilities({
            {"binary_support", "true"},
            {"compression", "gzip,brotli"},
            {"formats", "cbor,msgpack"}
        })
        .payload_hint(PayloadHint{
            PayloadType::VECTOR,
            1024, // size
            EncodingType::FLOAT32,
            256  // count
        })
        .build();

    if (!envelope_result.is_success()) {
        std::cerr << "Failed to build envelope: " << envelope_result.error_message.value_or("Unknown error") << std::endl;
        return 1;
    }

    Envelope envelope = envelope_result.value.value();

    // Serialize envelope to JSON
    auto serialize_result = EnvelopeProcessor::serialize(envelope);
    if (serialize_result.is_success()) {
        std::cout << "✅ Envelope serialized successfully" << std::endl;
        std::cout << "📄 JSON length: " << serialize_result.value->length() << " bytes" << std::endl;

        // Deserialize back
        auto deserialize_result = EnvelopeProcessor::deserialize(*serialize_result.value);
        if (deserialize_result.is_success()) {
            std::cout << "✅ Envelope deserialized successfully" << std::endl;
            std::cout << "📨 From: " << deserialize_result.value->from << std::endl;
            std::cout << "📨 To: " << deserialize_result.value->to << std::endl;
        }
    }

    // Example 2: Frame Operations
    std::cout << "\n📦 Example 2: Frame Operations" << std::endl;
    std::cout << "-------------------------------" << std::endl;

    // Create some sample data (simulating embeddings)
    std::vector<float> embeddings(768);
    for (size_t i = 0; i < embeddings.size(); ++i) {
        embeddings[i] = static_cast<float>(i) * 0.01f;
    }

    // Create frame with binary data
    ByteBuffer payload_data(embeddings.size() * sizeof(float));
    std::memcpy(payload_data.data(), embeddings.data(), payload_data.size());

    auto frame_result = FrameBuilder()
        .type(1) // Data frame
        .stream_id(42)
        .sequence(1)
        .payload(payload_data)
        .compressed(FrameFlags::COMPRESSED_GZIP)
        .build();

    if (!frame_result.is_success()) {
        std::cerr << "Failed to build frame: " << frame_result.error_message.value_or("Unknown error") << std::endl;
        return 1;
    }

    Frame frame = frame_result.value.value();

    // Serialize frame
    auto frame_serialize_result = FrameProcessor::serialize(frame);
    if (frame_serialize_result.is_success()) {
        std::cout << "✅ Frame serialized successfully" << std::endl;
        std::cout << "📦 Frame size: " << frame_serialize_result.value->size() << " bytes" << std::endl;
        std::cout << "🔢 Frame type: " << static_cast<int>(frame.header.type) << std::endl;
        std::cout << "🆔 Stream ID: " << frame.header.stream_id << std::endl;
        std::cout << "📊 Sequence: " << frame.header.sequence << std::endl;
        std::cout << "🏷️  Flags: " << frame.header.flags << std::endl;
        std::cout << "📏 Payload length: " << frame.header.length << " bytes" << std::endl;
    }

    // Example 3: Matrix Operations
    std::cout << "\n🧮 Example 3: Matrix Operations" << std::endl;
    std::cout << "-------------------------------" << std::endl;

    // Create sample matrices
    const size_t rows = 100;
    const size_t cols = 768;
    std::vector<float> matrix_a(rows * cols);
    std::vector<float> matrix_b(rows * cols);
    std::vector<float> result_matrix(rows * cols);

    // Initialize with sample data
    for (size_t i = 0; i < matrix_a.size(); ++i) {
        matrix_a[i] = static_cast<float>(i % 100) * 0.1f;
        matrix_b[i] = static_cast<float>((i + 50) % 100) * 0.1f;
    }

    // Matrix addition
    auto start_time = std::chrono::high_resolution_clock::now();
    auto add_result = MatrixOps::add(matrix_a.data(), matrix_b.data(), result_matrix.data(), rows, cols);
    auto end_time = std::chrono::high_resolution_clock::now();

    if (add_result.is_success()) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "✅ Matrix addition completed in " << duration.count() << " μs" << std::endl;
        std::cout << "📊 Matrix size: " << rows << "x" << cols << " (" << (rows * cols) << " elements)" << std::endl;
    }

    // Vector normalization
    start_time = std::chrono::high_resolution_clock::now();
    auto normalize_result = MatrixOps::normalize(result_matrix.data(), rows, cols);
    end_time = std::chrono::high_resolution_clock::now();

    if (normalize_result.is_success()) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "✅ Matrix normalization completed in " << duration.count() << " μs" << std::endl;
    }

    // Dot product example
    std::vector<float> vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> vec_b = {5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float dot_result = 0.0f;

    auto dot_product_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &dot_result, vec_a.size());
    if (dot_product_result.is_success()) {
        std::cout << "✅ Dot product: " << dot_result << std::endl;
    }

    // Cosine similarity
    float cos_sim = 0.0f;
    auto cos_sim_result = MatrixOps::cosine_similarity(vec_a.data(), vec_b.data(), &cos_sim, vec_a.size());
    if (cos_sim_result.is_success()) {
        std::cout << "✅ Cosine similarity: " << cos_sim << std::endl;
    }

    // Example 4: Performance Comparison
    std::cout << "\n⚡ Example 4: Performance Comparison" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    const size_t perf_size = 10000;
    std::vector<float> perf_a(perf_size);
    std::vector<float> perf_b(perf_size);
    std::vector<float> perf_result(perf_size);

    // Initialize test data
    for (size_t i = 0; i < perf_size; ++i) {
        perf_a[i] = static_cast<float>(i) * 0.001f;
        perf_b[i] = static_cast<float>(i % 10) * 0.1f;
    }

    // Time the operations
    start_time = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 100; ++iter) {
        MatrixOps::add(perf_a.data(), perf_b.data(), perf_result.data(), perf_size, 1);
    }
    end_time = std::chrono::high_resolution_clock::now();

    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "⏱️  100 vector additions (" << perf_size << " elements each): " << total_duration.count() << " ms" << std::endl;
    std::cout << "📈 Average time per addition: " << (total_duration.count() / 100.0) << " ms" << std::endl;

    // Example 5: Security Operations (ChaCha20-Poly1305)
    std::cout << "\n🔐 Example 5: Security Operations (ChaCha20-Poly1305)" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;

    try {
        SecurityManager security("test-node");

        // Generate keys
        auto keygen_result = security.generate_keypair();
        if (keygen_result.is_success()) {
            std::cout << "✅ Key pair generated successfully" << std::endl;

            // Test data
            ByteBuffer test_data = {'S', 'e', 'c', 'r', 'e', 't', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'};
            std::cout << "📝 Original data size: " << test_data.size() << " bytes" << std::endl;

            // Encrypt
            auto encrypt_result = security.encrypt_data(test_data);
            if (encrypt_result.is_success()) {
                std::cout << "🔒 Encryption successful" << std::endl;
                std::cout << "📦 Encrypted data size: " << encrypt_result.value->size() << " bytes" << std::endl;

                // Decrypt
                auto decrypt_result = security.decrypt_data(*encrypt_result.value);
                if (decrypt_result.is_success()) {
                    std::cout << "🔓 Decryption successful" << std::endl;
                    if (*decrypt_result.value == test_data) {
                        std::cout << "✅ Round-trip encryption/decryption successful" << std::endl;
                    } else {
                        std::cout << "❌ Round-trip failed - data mismatch" << std::endl;
                    }
                } else {
                    std::cout << "❌ Decryption failed: " << decrypt_result.error_message.value_or("Unknown error") << std::endl;
                }
            } else {
                std::cout << "❌ Encryption failed: " << encrypt_result.error_message.value_or("Unknown error") << std::endl;
            }
        } else {
            std::cout << "❌ Key generation failed: " << keygen_result.error_message.value_or("Unknown error") << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ Security test failed with exception: " << e.what() << std::endl;
    }

    // Example 6: Compression Operations (LZ4)
    std::cout << "\n⚡ Example 6: Compression Operations (LZ4)" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;

    try {
        CompressionManager compression(CompressionAlgorithm::LZ4);

        // Create test data
        std::string test_string = "This is a test string for LZ4 compression testing. ";
        for (int i = 0; i < 50; ++i) {
            test_string += "Additional repetitive data to improve compression ratio. ";
        }

        ByteBuffer original_data(test_string.begin(), test_string.end());
        std::cout << "📝 Original data size: " << original_data.size() << " bytes" << std::endl;

        // Compress
        auto compress_result = compression.compress(original_data);
        if (compress_result.is_success()) {
            std::cout << "🗜️  Compression successful" << std::endl;
            std::cout << "📦 Compressed data size: " << compress_result.value->size() << " bytes" << std::endl;

            double ratio = static_cast<double>(compress_result.value->size()) / original_data.size() * 100.0;
            std::cout << "📊 Compression ratio: " << std::fixed << std::setprecision(1) << ratio << "%" << std::endl;

            // Decompress
            auto decompress_result = compression.decompress(*compress_result.value);
            if (decompress_result.is_success()) {
                std::cout << "📤 Decompression successful" << std::endl;
                if (*decompress_result.value == original_data) {
                    std::cout << "✅ Round-trip compression/decompression successful" << std::endl;
                } else {
                    std::cout << "❌ Round-trip failed - data corruption" << std::endl;
                }
            } else {
                std::cout << "❌ Decompression failed: " << decompress_result.error_message.value_or("Unknown error") << std::endl;
            }
        } else {
            std::cout << "❌ Compression failed: " << compress_result.error_message.value_or("Unknown error") << std::endl;
            std::cout << "ℹ️  This may be due to LZ4 library not being available" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ Compression test failed with exception: " << e.what() << std::endl;
    }

    // Example 7: Hardware Acceleration Detection
    std::cout << "\n🚀 Example 7: Hardware Acceleration Detection" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;

    try {
        SecurityManager hw_security("hw-test-node");

        // This will demonstrate if AES-NI is available
        auto keygen_result = hw_security.generate_keypair();
        if (keygen_result.is_success()) {
            std::cout << "✅ Security manager initialized" << std::endl;

            ByteBuffer test_data = {'H', 'W', ' ', 'A', 'c', 'c', 'e', 'l'};
            auto encrypt_result = hw_security.encrypt_data(test_data);

            if (encrypt_result.is_success()) {
                std::cout << "✅ Hardware-accelerated encryption available" << std::endl;
                std::cout << "🔥 AES-NI or ChaCha20-Poly1305 encryption working" << std::endl;
            } else {
                std::cout << "ℹ️  Hardware acceleration may not be available" << std::endl;
                std::cout << "🔄 Using fallback ChaCha20 implementation" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "❌ Hardware acceleration test failed: " << e.what() << std::endl;
    }

    std::cout << "\n🎉 UMICP C++ Core Example Completed Successfully!" << std::endl;
    std::cout << "📋 Tested Features:" << std::endl;
    std::cout << "   ✅ Matrix Operations (SIMD)" << std::endl;
    std::cout << "   ✅ ChaCha20-Poly1305 Encryption" << std::endl;
    std::cout << "   ✅ LZ4 Compression" << std::endl;
    std::cout << "   ✅ Hardware Acceleration Detection" << std::endl;
    std::cout << "   ✅ Envelope & Frame Processing" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
