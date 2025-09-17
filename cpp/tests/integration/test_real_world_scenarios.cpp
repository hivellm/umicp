/**
 * UMICP Real-World Scenarios Tests
 * Testing realistic usage patterns and production-like scenarios
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "serialization.h"
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <future>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace umicp;
using namespace umicp::testing;

class RealWorldScenariosTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Production-like configuration
        production_config_.max_message_size = 10 * 1024 * 1024; // 10MB
        production_config_.connection_timeout = 30000;          // 30 seconds
        production_config_.enable_binary = true;
        production_config_.preferred_format = ContentType::BINARY;
        production_config_.require_auth = true;
        production_config_.require_encryption = true;

        // Initialize random number generator for realistic data
        rng_.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    UMICPConfig production_config_;
    std::mt19937 rng_;

    // Helper to generate realistic ML model data
    struct MLModelData {
        std::vector<float> weights;
        std::vector<float> biases;
        std::vector<float> gradients;
        int layer_count;
        int input_size;
        int output_size;
    };

    MLModelData generate_ml_model_data(int complexity_level = 1) {
        MLModelData model;
        model.layer_count = 3 + complexity_level;
        model.input_size = 128 * complexity_level;
        model.output_size = 64 * complexity_level;

        size_t total_weights = model.input_size * model.output_size * model.layer_count;
        model.weights = TestHelpers::generate_random_vector(total_weights, rng_());
        model.biases = TestHelpers::generate_random_vector(model.output_size * model.layer_count, rng_());
        model.gradients = TestHelpers::generate_random_vector(total_weights, rng_());

        return model;
    }

    // Helper to simulate realistic network conditions
    void simulate_network_delay(int min_ms = 10, int max_ms = 100) {
        std::uniform_int_distribution<int> delay_dist(min_ms, max_ms);
        int delay = delay_dist(rng_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Helper to create realistic message content
    ByteBuffer create_realistic_message(const std::string& message_type, size_t size_hint = 0) {
        if (message_type == "ml_weights") {
            auto model = generate_ml_model_data(2);
            ByteBuffer data;
            data.reserve(model.weights.size() * sizeof(float));
            const uint8_t* weights_ptr = reinterpret_cast<const uint8_t*>(model.weights.data());
            data.assign(weights_ptr, weights_ptr + model.weights.size() * sizeof(float));
            return data;
        } else if (message_type == "sensor_data") {
            // Simulate IoT sensor data
            size_t data_size = size_hint > 0 ? size_hint : 512;
            return TestHelpers::generate_random_data(data_size, rng_());
        } else if (message_type == "file_chunk") {
            // Simulate file transfer chunk
            size_t chunk_size = size_hint > 0 ? size_hint : 64 * 1024; // 64KB default
            return TestHelpers::generate_random_data(chunk_size, rng_());
        } else {
            // Generic message
            size_t msg_size = size_hint > 0 ? size_hint : 1024;
            return TestHelpers::generate_random_data(msg_size, rng_());
        }
    }
};

// ===============================================
// Machine Learning Scenarios
// ===============================================

TEST_F(RealWorldScenariosTest, MLScenario_DistributedTraining) {
    std::cout << "\n  🧠 Testing Distributed ML Training Scenario:" << std::endl;

    // Simulate 4 worker nodes in distributed training
    const int num_workers = 4;
    const int training_rounds = 10;
    const int gradients_per_round = 5;

    std::vector<std::unique_ptr<Protocol>> workers;
    std::vector<std::unique_ptr<SecurityManager>> worker_security;

    // Initialize workers
    for (int i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<Protocol>("ml-worker-" + std::to_string(i));
        worker->configure(production_config_);
        worker->set_transport(std::make_shared<MockTransport>(
            TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9000 + i)));

        auto security = std::make_unique<SecurityManager>("worker-security-" + std::to_string(i));
        security->generate_keypair();

        auto connect_result = worker->connect();
        ASSERT_TRUE(connect_result.is_success()) << "Worker " << i << " failed to connect";

        workers.push_back(std::move(worker));
        worker_security.push_back(std::move(security));
    }

    std::atomic<int> total_gradients_sent{0};
    std::atomic<int> total_model_updates{0};

    // Simulate distributed training
    auto training_start = std::chrono::high_resolution_clock::now();

    for (int round = 0; round < training_rounds; ++round) {
        std::vector<std::future<void>> round_futures;

        // Each worker computes and sends gradients
        for (int worker_id = 0; worker_id < num_workers; ++worker_id) {
            round_futures.push_back(std::async(std::launch::async, [&, worker_id]() {
                for (int grad = 0; grad < gradients_per_round; ++grad) {
                    // Generate gradient data
                    ByteBuffer gradient_data = create_realistic_message("ml_weights");

                    // Sign the gradient (security requirement)
                    auto signature = worker_security[worker_id]->sign_data(gradient_data);
                    ASSERT_TRUE(signature.is_success());

                    // Send gradient to parameter server
                    auto send_result = workers[worker_id]->send_data("parameter-server", gradient_data);
                    if (send_result.is_success()) {
                        total_gradients_sent.fetch_add(1);
                    }

                    // Simulate computation time
                    simulate_network_delay(5, 20);
                }

                // Receive model update
                ByteBuffer model_update = create_realistic_message("ml_weights");
                // In real scenario, this would be received via callback
                total_model_updates.fetch_add(1);
            }));
        }

        // Wait for round to complete
        for (auto& future : round_futures) {
            future.wait();
        }

        std::cout << "    Round " << (round + 1) << "/" << training_rounds << " completed" << std::endl;
    }

    auto training_end = std::chrono::high_resolution_clock::now();
    auto training_duration = std::chrono::duration_cast<std::chrono::milliseconds>(training_end - training_start);

    int expected_gradients = num_workers * training_rounds * gradients_per_round;
    int expected_updates = num_workers * training_rounds;

    std::cout << "    Training Results:" << std::endl;
    std::cout << "      Duration: " << training_duration.count() << " ms" << std::endl;
    std::cout << "      Gradients sent: " << total_gradients_sent.load() << "/" << expected_gradients << std::endl;
    std::cout << "      Model updates: " << total_model_updates.load() << "/" << expected_updates << std::endl;

    EXPECT_EQ(total_gradients_sent.load(), expected_gradients);
    EXPECT_EQ(total_model_updates.load(), expected_updates);

    // Verify worker statistics
    for (int i = 0; i < num_workers; ++i) {
        auto stats = workers[i]->get_stats();
        EXPECT_GT(stats.messages_sent, 0);
        EXPECT_GT(stats.bytes_sent, 0);
    }

    std::cout << "    ✅ Distributed ML training scenario completed successfully" << std::endl;
}

TEST_F(RealWorldScenariosTest, MLScenario_InferenceService) {
    std::cout << "\n  🧠 Testing ML Inference Service Scenario:" << std::endl;

    // Simulate inference service with multiple clients
    Protocol inference_server("ml-inference-server");
    inference_server.configure(production_config_);
    inference_server.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9100)));

    auto connect_result = inference_server.connect();
    ASSERT_TRUE(connect_result.is_success());

    const int num_clients = 8;
    const int requests_per_client = 50;
    std::atomic<int> successful_inferences{0};
    std::atomic<int> failed_inferences{0};

    // Load "model" (simulate loading ML model weights)
    auto model_data = generate_ml_model_data(3); // Complex model
    std::cout << "    Model loaded: " << model_data.weights.size() << " weights, "
              << model_data.layer_count << " layers" << std::endl;

    auto inference_start = std::chrono::high_resolution_clock::now();

    std::vector<std::future<void>> client_futures;

    // Launch inference clients
    for (int client_id = 0; client_id < num_clients; ++client_id) {
        client_futures.push_back(std::async(std::launch::async, [&, client_id]() {
            Protocol client("inference-client-" + std::to_string(client_id));
            client.configure(production_config_);
            client.set_transport(std::make_shared<MockTransport>(
                TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9200 + client_id)));

            auto client_connect = client.connect();
            if (!client_connect.is_success()) {
                failed_inferences.fetch_add(requests_per_client);
                return;
            }

            for (int request = 0; request < requests_per_client; ++request) {
                // Create inference request (input data)
                std::vector<float> input_data = TestHelpers::generate_random_vector(model_data.input_size, rng_());
                ByteBuffer request_data(reinterpret_cast<const uint8_t*>(input_data.data()),
                                       reinterpret_cast<const uint8_t*>(input_data.data()) + input_data.size() * sizeof(float));

                // Send inference request
                auto send_result = client.send_data("ml-inference-server", request_data);
                if (!send_result.is_success()) {
                    failed_inferences.fetch_add(1);
                    continue;
                }

                // Simulate inference computation (matrix operations)
                std::vector<float> output(model_data.output_size);
                auto inference_result = MatrixOps::multiply(
                    input_data.data(), model_data.weights.data(), output.data(),
                    1, model_data.input_size, model_data.output_size);

                if (inference_result.is_success()) {
                    successful_inferences.fetch_add(1);

                    // Send response back (in real scenario, this would be automatic)
                    ByteBuffer response_data(reinterpret_cast<const uint8_t*>(output.data()),
                                           reinterpret_cast<const uint8_t*>(output.data()) + output.size() * sizeof(float));
                    inference_server.send_data("inference-client-" + std::to_string(client_id), response_data);
                } else {
                    failed_inferences.fetch_add(1);
                }

                // Realistic request interval
                simulate_network_delay(1, 10);
            }
        }));
    }

    // Wait for all clients to complete
    for (auto& future : client_futures) {
        future.wait();
    }

    auto inference_end = std::chrono::high_resolution_clock::now();
    auto inference_duration = std::chrono::duration_cast<std::chrono::milliseconds>(inference_end - inference_start);

    int total_requests = num_clients * requests_per_client;
    double success_rate = (double)successful_inferences.load() / total_requests * 100.0;
    double requests_per_second = (total_requests * 1000.0) / inference_duration.count();

    std::cout << "    Inference Results:" << std::endl;
    std::cout << "      Duration: " << inference_duration.count() << " ms" << std::endl;
    std::cout << "      Successful inferences: " << successful_inferences.load() << "/" << total_requests << std::endl;
    std::cout << "      Success rate: " << std::fixed << std::setprecision(1) << success_rate << "%" << std::endl;
    std::cout << "      Throughput: " << std::setprecision(1) << requests_per_second << " requests/s" << std::endl;

    EXPECT_GT(success_rate, 95.0); // At least 95% success rate
    EXPECT_GT(requests_per_second, 100.0); // At least 100 requests/s

    std::cout << "    ✅ ML inference service scenario completed successfully" << std::endl;
}

// ===============================================
// IoT and Sensor Data Scenarios
// ===============================================

TEST_F(RealWorldScenariosTest, IoTScenario_SensorDataAggregation) {
    std::cout << "\n  📡 Testing IoT Sensor Data Aggregation Scenario:" << std::endl;

    // Simulate IoT gateway collecting data from multiple sensors
    Protocol iot_gateway("iot-gateway");
    iot_gateway.configure(production_config_);
    iot_gateway.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9300)));

    auto gateway_connect = iot_gateway.connect();
    ASSERT_TRUE(gateway_connect.is_success());

    const int num_sensors = 20;
    const int readings_per_sensor = 100;
    const int collection_duration_seconds = 30;

    std::atomic<int> total_readings_collected{0};
    std::atomic<int> total_readings_processed{0};
    std::atomic<size_t> total_data_bytes{0};

    // Sensor data collection structure
    struct SensorReading {
        uint32_t sensor_id;
        uint64_t timestamp;
        float temperature;
        float humidity;
        float pressure;
        uint16_t battery_level;
    };

    std::vector<std::future<void>> sensor_futures;
    auto collection_start = std::chrono::high_resolution_clock::now();

    // Launch sensor simulators
    for (int sensor_id = 0; sensor_id < num_sensors; ++sensor_id) {
        sensor_futures.push_back(std::async(std::launch::async, [&, sensor_id]() {
            Protocol sensor("iot-sensor-" + std::to_string(sensor_id));
            sensor.configure(production_config_);
            sensor.set_transport(std::make_shared<MockTransport>(
                TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9400 + sensor_id)));

            auto sensor_connect = sensor.connect();
            if (!sensor_connect.is_success()) return;

            std::uniform_real_distribution<float> temp_dist(18.0f, 35.0f);
            std::uniform_real_distribution<float> humidity_dist(30.0f, 90.0f);
            std::uniform_real_distribution<float> pressure_dist(980.0f, 1020.0f);
            std::uniform_int_distribution<uint16_t> battery_dist(20, 100);

            for (int reading = 0; reading < readings_per_sensor; ++reading) {
                SensorReading sensor_data;
                sensor_data.sensor_id = sensor_id;
                sensor_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                sensor_data.temperature = temp_dist(rng_);
                sensor_data.humidity = humidity_dist(rng_);
                sensor_data.pressure = pressure_dist(rng_);
                sensor_data.battery_level = battery_dist(rng_);

                // Convert to ByteBuffer
                ByteBuffer reading_data(reinterpret_cast<const uint8_t*>(&sensor_data),
                                       reinterpret_cast<const uint8_t*>(&sensor_data) + sizeof(SensorReading));

                // Send to gateway
                auto send_result = sensor.send_data("iot-gateway", reading_data);
                if (send_result.is_success()) {
                    total_readings_collected.fetch_add(1);
                    total_data_bytes.fetch_add(reading_data.size());
                }

                // Realistic sensor reading interval
                simulate_network_delay(100, 500); // 100-500ms between readings
            }
        }));
    }

    // Gateway data processing simulation
    std::future<void> processing_future = std::async(std::launch::async, [&]() {
        std::vector<SensorReading> readings_buffer;
        readings_buffer.reserve(num_sensors * readings_per_sensor);

        // Simulate processing collected data
        while (total_readings_collected.load() < num_sensors * readings_per_sensor) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // Simulate batch processing
            int current_readings = total_readings_collected.load();
            int to_process = std::min(100, current_readings - total_readings_processed.load());

            if (to_process > 0) {
                // Simulate data processing (aggregation, filtering, etc.)
                for (int i = 0; i < to_process; ++i) {
                    // Simulate processing time
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    total_readings_processed.fetch_add(1);
                }

                // Send aggregated data to cloud (simulate)
                if (total_readings_processed.load() % 50 == 0) {
                    ByteBuffer aggregated_data = create_realistic_message("sensor_data", 2048);
                    iot_gateway.send_data("cloud-service", aggregated_data);
                }
            }
        }
    });

    // Wait for collection to complete
    for (auto& future : sensor_futures) {
        future.wait();
    }
    processing_future.wait();

    auto collection_end = std::chrono::high_resolution_clock::now();
    auto collection_duration = std::chrono::duration_cast<std::chrono::milliseconds>(collection_end - collection_start);

    int expected_readings = num_sensors * readings_per_sensor;
    double readings_per_second = (total_readings_collected.load() * 1000.0) / collection_duration.count();
    double data_throughput_kbps = (total_data_bytes.load() * 8.0 * 1000.0) / (collection_duration.count() * 1024.0);

    std::cout << "    IoT Collection Results:" << std::endl;
    std::cout << "      Duration: " << collection_duration.count() << " ms" << std::endl;
    std::cout << "      Readings collected: " << total_readings_collected.load() << "/" << expected_readings << std::endl;
    std::cout << "      Readings processed: " << total_readings_processed.load() << std::endl;
    std::cout << "      Data throughput: " << std::fixed << std::setprecision(2) << data_throughput_kbps << " kbps" << std::endl;
    std::cout << "      Collection rate: " << std::setprecision(1) << readings_per_second << " readings/s" << std::endl;

    EXPECT_EQ(total_readings_collected.load(), expected_readings);
    EXPECT_EQ(total_readings_processed.load(), expected_readings);
    EXPECT_GT(readings_per_second, 10.0); // At least 10 readings/s

    std::cout << "    ✅ IoT sensor data aggregation scenario completed successfully" << std::endl;
}

// ===============================================
// File Transfer and Streaming Scenarios
// ===============================================

TEST_F(RealWorldScenariosTest, FileTransfer_LargeFileStreaming) {
    std::cout << "\n  📁 Testing Large File Streaming Scenario:" << std::endl;

    // Simulate large file transfer with chunking
    Protocol file_server("file-transfer-server");
    file_server.configure(production_config_);
    file_server.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9500)));

    Protocol file_client("file-transfer-client");
    file_client.configure(production_config_);
    file_client.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9501)));

    auto server_connect = file_server.connect();
    auto client_connect = file_client.connect();
    ASSERT_TRUE(server_connect.is_success());
    ASSERT_TRUE(client_connect.is_success());

    // Simulate 100MB file transfer
    const size_t total_file_size = 100 * 1024 * 1024; // 100MB
    const size_t chunk_size = 1024 * 1024;            // 1MB chunks
    const size_t num_chunks = total_file_size / chunk_size;

    std::atomic<size_t> chunks_sent{0};
    std::atomic<size_t> chunks_received{0};
    std::atomic<size_t> bytes_transferred{0};

    auto transfer_start = std::chrono::high_resolution_clock::now();

    // File transfer simulation
    std::future<void> sender_future = std::async(std::launch::async, [&]() {
        for (size_t chunk_id = 0; chunk_id < num_chunks; ++chunk_id) {
            // Generate chunk data
            ByteBuffer chunk_data = create_realistic_message("file_chunk", chunk_size);

            // Create chunk header with metadata
            struct ChunkHeader {
                uint64_t chunk_id;
                uint64_t total_chunks;
                uint64_t chunk_size;
                uint64_t file_size;
            };

            ChunkHeader header;
            header.chunk_id = chunk_id;
            header.total_chunks = num_chunks;
            header.chunk_size = chunk_data.size();
            header.file_size = total_file_size;

            // Combine header and data
            ByteBuffer full_chunk(sizeof(ChunkHeader) + chunk_data.size());
            std::memcpy(full_chunk.data(), &header, sizeof(ChunkHeader));
            std::memcpy(full_chunk.data() + sizeof(ChunkHeader), chunk_data.data(), chunk_data.size());

            // Send chunk
            auto send_result = file_server.send_data("file-transfer-client", full_chunk);
            if (send_result.is_success()) {
                chunks_sent.fetch_add(1);
                bytes_transferred.fetch_add(full_chunk.size());
            }

            // Simulate network conditions
            simulate_network_delay(1, 5);
        }
    });

    // File receiver simulation
    std::future<void> receiver_future = std::async(std::launch::async, [&]() {
        std::vector<bool> received_chunks(num_chunks, false);
        size_t received_count = 0;

        while (received_count < num_chunks) {
            // Simulate receiving chunks (in real scenario, this would be via callback)
            if (chunks_sent.load() > received_count) {
                received_chunks[received_count] = true;
                received_count++;
                chunks_received.fetch_add(1);

                // Send acknowledgment
                ByteBuffer ack_data(8);
                uint64_t ack_chunk_id = received_count - 1;
                std::memcpy(ack_data.data(), &ack_chunk_id, sizeof(uint64_t));
                file_client.send_data("file-transfer-server", ack_data);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Wait for transfer completion
    sender_future.wait();
    receiver_future.wait();

    auto transfer_end = std::chrono::high_resolution_clock::now();
    auto transfer_duration = std::chrono::duration_cast<std::chrono::milliseconds>(transfer_end - transfer_start);

    double throughput_mbps = (bytes_transferred.load() * 8.0 * 1000.0) / (transfer_duration.count() * 1024.0 * 1024.0);
    double chunks_per_second = (chunks_sent.load() * 1000.0) / transfer_duration.count();

    std::cout << "    File Transfer Results:" << std::endl;
    std::cout << "      Duration: " << transfer_duration.count() << " ms" << std::endl;
    std::cout << "      Chunks sent: " << chunks_sent.load() << "/" << num_chunks << std::endl;
    std::cout << "      Chunks received: " << chunks_received.load() << "/" << num_chunks << std::endl;
    std::cout << "      Bytes transferred: " << TestHelpers::format_bytes(bytes_transferred.load()) << std::endl;
    std::cout << "      Throughput: " << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps" << std::endl;
    std::cout << "      Chunk rate: " << std::setprecision(1) << chunks_per_second << " chunks/s" << std::endl;

    EXPECT_EQ(chunks_sent.load(), num_chunks);
    EXPECT_EQ(chunks_received.load(), num_chunks);
    EXPECT_GT(throughput_mbps, 10.0); // At least 10 Mbps

    std::cout << "    ✅ Large file streaming scenario completed successfully" << std::endl;
}

// ===============================================
// Multi-Protocol Communication Scenario
// ===============================================

TEST_F(RealWorldScenariosTest, MultiProtocol_HybridCommunication) {
    std::cout << "\n  🔄 Testing Multi-Protocol Hybrid Communication Scenario:" << std::endl;

    // Simulate system using both JSON and binary protocols
    const int num_nodes = 6;
    std::vector<std::unique_ptr<Protocol>> json_nodes;
    std::vector<std::unique_ptr<Protocol>> binary_nodes;

    // Create JSON-based nodes (legacy systems)
    UMICPConfig json_config = production_config_;
    json_config.preferred_format = ContentType::JSON;
    json_config.enable_binary = false;

    for (int i = 0; i < num_nodes / 2; ++i) {
        auto node = std::make_unique<Protocol>("json-node-" + std::to_string(i));
        node->configure(json_config);
        node->set_transport(std::make_shared<MockTransport>(
            TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9600 + i)));

        auto connect_result = node->connect();
        ASSERT_TRUE(connect_result.is_success());
        json_nodes.push_back(std::move(node));
    }

    // Create binary-based nodes (new systems)
    UMICPConfig binary_config = production_config_;
    binary_config.preferred_format = ContentType::BINARY;
    binary_config.enable_binary = true;

    for (int i = 0; i < num_nodes / 2; ++i) {
        auto node = std::make_unique<Protocol>("binary-node-" + std::to_string(i));
        node->configure(binary_config);
        node->set_transport(std::make_shared<MockTransport>(
            TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9700 + i)));

        auto connect_result = node->connect();
        ASSERT_TRUE(connect_result.is_success());
        binary_nodes.push_back(std::move(node));
    }

    const int messages_per_node = 50;
    std::atomic<int> json_messages_sent{0};
    std::atomic<int> binary_messages_sent{0};
    std::atomic<int> cross_protocol_messages{0};

    auto communication_start = std::chrono::high_resolution_clock::now();

    std::vector<std::future<void>> communication_futures;

    // JSON nodes communication
    for (size_t i = 0; i < json_nodes.size(); ++i) {
        communication_futures.push_back(std::async(std::launch::async, [&, i]() {
            for (int msg = 0; msg < messages_per_node; ++msg) {
                // Send to other JSON nodes
                for (size_t j = 0; j < json_nodes.size(); ++j) {
                    if (i != j) {
                        ByteBuffer data = create_realistic_message("generic", 512);
                        auto result = json_nodes[i]->send_data("json-node-" + std::to_string(j), data);
                        if (result.is_success()) {
                            json_messages_sent.fetch_add(1);
                        }
                    }
                }

                // Send to binary nodes (cross-protocol)
                for (size_t j = 0; j < binary_nodes.size(); ++j) {
                    ByteBuffer data = create_realistic_message("generic", 256);
                    auto result = json_nodes[i]->send_data("binary-node-" + std::to_string(j), data);
                    if (result.is_success()) {
                        cross_protocol_messages.fetch_add(1);
                    }
                }

                simulate_network_delay(10, 50);
            }
        }));
    }

    // Binary nodes communication
    for (size_t i = 0; i < binary_nodes.size(); ++i) {
        communication_futures.push_back(std::async(std::launch::async, [&, i]() {
            for (int msg = 0; msg < messages_per_node; ++msg) {
                // Send to other binary nodes
                for (size_t j = 0; j < binary_nodes.size(); ++j) {
                    if (i != j) {
                        ByteBuffer data = create_realistic_message("ml_weights", 2048);
                        auto result = binary_nodes[i]->send_data("binary-node-" + std::to_string(j), data);
                        if (result.is_success()) {
                            binary_messages_sent.fetch_add(1);
                        }
                    }
                }

                // Send to JSON nodes (cross-protocol)
                for (size_t j = 0; j < json_nodes.size(); ++j) {
                    ByteBuffer data = create_realistic_message("sensor_data", 128);
                    auto result = binary_nodes[i]->send_data("json-node-" + std::to_string(j), data);
                    if (result.is_success()) {
                        cross_protocol_messages.fetch_add(1);
                    }
                }

                simulate_network_delay(5, 25);
            }
        }));
    }

    // Wait for all communication to complete
    for (auto& future : communication_futures) {
        future.wait();
    }

    auto communication_end = std::chrono::high_resolution_clock::now();
    auto communication_duration = std::chrono::duration_cast<std::chrono::milliseconds>(communication_end - communication_start);

    int total_messages = json_messages_sent.load() + binary_messages_sent.load() + cross_protocol_messages.load();
    double messages_per_second = (total_messages * 1000.0) / communication_duration.count();

    std::cout << "    Multi-Protocol Communication Results:" << std::endl;
    std::cout << "      Duration: " << communication_duration.count() << " ms" << std::endl;
    std::cout << "      JSON messages: " << json_messages_sent.load() << std::endl;
    std::cout << "      Binary messages: " << binary_messages_sent.load() << std::endl;
    std::cout << "      Cross-protocol messages: " << cross_protocol_messages.load() << std::endl;
    std::cout << "      Total messages: " << total_messages << std::endl;
    std::cout << "      Message rate: " << std::fixed << std::setprecision(1) << messages_per_second << " msg/s" << std::endl;

    // Verify statistics from both types of nodes
    for (const auto& node : json_nodes) {
        auto stats = node->get_stats();
        EXPECT_GT(stats.messages_sent, 0);
    }

    for (const auto& node : binary_nodes) {
        auto stats = node->get_stats();
        EXPECT_GT(stats.messages_sent, 0);
    }

    EXPECT_GT(total_messages, (num_nodes * messages_per_node * (num_nodes - 1)) * 0.9); // At least 90% success
    EXPECT_GT(cross_protocol_messages.load(), 0); // Cross-protocol communication worked

    std::cout << "    ✅ Multi-protocol hybrid communication scenario completed successfully" << std::endl;
}

// ===============================================
// Production Load Testing Scenario
// ===============================================

TEST_F(RealWorldScenariosTest, Production_HighLoadSimulation) {
    std::cout << "\n  🏭 Testing Production High Load Simulation:" << std::endl;

    // Simulate production load with realistic patterns
    const int num_servers = 4;
    const int clients_per_server = 25;
    const int total_clients = num_servers * clients_per_server;
    const int duration_seconds = 60;

    std::vector<std::unique_ptr<Protocol>> servers;
    std::atomic<int> total_connections{0};
    std::atomic<int> total_messages{0};
    std::atomic<size_t> total_bytes{0};
    std::atomic<int> connection_errors{0};
    std::atomic<int> message_errors{0};

    // Create servers
    for (int i = 0; i < num_servers; ++i) {
        auto server = std::make_unique<Protocol>("prod-server-" + std::to_string(i));
        server->configure(production_config_);
        server->set_transport(std::make_shared<MockTransport>(
            TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 9800 + i)));

        auto connect_result = server->connect();
        ASSERT_TRUE(connect_result.is_success());
        servers.push_back(std::move(server));
    }

    auto load_test_start = std::chrono::high_resolution_clock::now();

    std::vector<std::future<void>> client_futures;

    // Launch client load generators
    for (int client_id = 0; client_id < total_clients; ++client_id) {
        client_futures.push_back(std::async(std::launch::async, [&, client_id]() {
            Protocol client("prod-client-" + std::to_string(client_id));
            client.configure(production_config_);
            client.set_transport(std::make_shared<MockTransport>(
                TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 10000 + client_id)));

            auto client_connect = client.connect();
            if (!client_connect.is_success()) {
                connection_errors.fetch_add(1);
                return;
            }
            total_connections.fetch_add(1);

            int server_id = client_id % num_servers;
            std::string target_server = "prod-server-" + std::to_string(server_id);

            // Run for specified duration
            auto client_start = std::chrono::high_resolution_clock::now();
            auto client_end_time = client_start + std::chrono::seconds(duration_seconds);

            std::uniform_int_distribution<int> message_type_dist(0, 2);
            std::uniform_int_distribution<int> message_size_dist(100, 10000);

            while (std::chrono::high_resolution_clock::now() < client_end_time) {
                // Generate realistic message
                int message_type = message_type_dist(rng_);
                int message_size = message_size_dist(rng_);

                ByteBuffer message_data;
                switch (message_type) {
                    case 0: message_data = create_realistic_message("sensor_data", message_size); break;
                    case 1: message_data = create_realistic_message("ml_weights", message_size); break;
                    case 2: message_data = create_realistic_message("generic", message_size); break;
                }

                auto send_result = client.send_data(target_server, message_data);
                if (send_result.is_success()) {
                    total_messages.fetch_add(1);
                    total_bytes.fetch_add(message_data.size());
                } else {
                    message_errors.fetch_add(1);
                }

                // Realistic message intervals (10-100ms)
                simulate_network_delay(10, 100);
            }
        }));
    }

    // Wait for all clients to complete
    for (auto& future : client_futures) {
        future.wait();
    }

    auto load_test_end = std::chrono::high_resolution_clock::now();
    auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(load_test_end - load_test_start);

    double connection_success_rate = (double)total_connections.load() / total_clients * 100.0;
    double message_success_rate = (double)total_messages.load() / (total_messages.load() + message_errors.load()) * 100.0;
    double messages_per_second = (total_messages.load() * 1000.0) / test_duration.count();
    double throughput_mbps = (total_bytes.load() * 8.0 * 1000.0) / (test_duration.count() * 1024.0 * 1024.0);

    std::cout << "    Production Load Test Results:" << std::endl;
    std::cout << "      Test duration: " << test_duration.count() << " ms" << std::endl;
    std::cout << "      Total clients: " << total_clients << std::endl;
    std::cout << "      Successful connections: " << total_connections.load() << "/" << total_clients
              << " (" << std::fixed << std::setprecision(1) << connection_success_rate << "%)" << std::endl;
    std::cout << "      Total messages: " << total_messages.load() << std::endl;
    std::cout << "      Message errors: " << message_errors.load() << std::endl;
    std::cout << "      Message success rate: " << std::setprecision(1) << message_success_rate << "%" << std::endl;
    std::cout << "      Messages per second: " << std::setprecision(1) << messages_per_second << std::endl;
    std::cout << "      Throughput: " << std::setprecision(2) << throughput_mbps << " Mbps" << std::endl;
    std::cout << "      Data transferred: " << TestHelpers::format_bytes(total_bytes.load()) << std::endl;

    // Production quality requirements
    EXPECT_GT(connection_success_rate, 98.0);    // 98% connection success
    EXPECT_GT(message_success_rate, 99.0);       // 99% message success
    EXPECT_GT(messages_per_second, 500.0);       // At least 500 msg/s
    EXPECT_GT(throughput_mbps, 10.0);            // At least 10 Mbps

    // Verify server statistics
    for (const auto& server : servers) {
        auto stats = server->get_stats();
        EXPECT_GT(stats.messages_sent, 0);
        std::cout << "      Server " << server->get_node_id() << ": "
                  << stats.messages_sent << " messages, "
                  << TestHelpers::format_bytes(stats.bytes_sent) << " sent" << std::endl;
    }

    std::cout << "    ✅ Production high load simulation completed successfully" << std::endl;
}
