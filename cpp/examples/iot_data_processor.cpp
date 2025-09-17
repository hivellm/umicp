/**
 * UMICP C++ Bindings - IoT Data Processor
 *
 * This example demonstrates real-time IoT data processing using UMICP envelopes
 * for sensor data collection, processing, and analytics.
 */

#include <umicp.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <unordered_map>

struct SensorReading {
    std::string sensor_id;
    std::string sensor_type;
    float value;
    std::string unit;
    std::string location;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, float> metadata;

    SensorReading(const std::string& id, const std::string& type, float val,
                  const std::string& u, const std::string& loc)
        : sensor_id(id), sensor_type(type), value(val), unit(u), location(loc),
          timestamp(std::chrono::system_clock::now()) {}
};

class IoTDataProcessor {
private:
    std::queue<SensorReading> data_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> running_{true};
    std::atomic<size_t> processed_count_{0};
    std::thread processing_thread_;

    // Statistics
    std::unordered_map<std::string, std::vector<float>> sensor_stats_;
    std::mutex stats_mutex_;

    // Batch processing
    const size_t batch_size_ = 50;
    std::vector<SensorReading> current_batch_;

public:
    IoTDataProcessor() {
        processing_thread_ = std::thread(&IoTDataProcessor::processing_loop, this);
    }

    ~IoTDataProcessor() {
        running_ = false;
        queue_cv_.notify_all();
        if (processing_thread_.joinable()) {
            processing_thread_.join();
        }
    }

    // Add sensor reading to processing queue
    void add_reading(const SensorReading& reading) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            data_queue_.push(reading);
        }
        queue_cv_.notify_one();
    }

    // Get processing statistics
    std::unordered_map<std::string, double> get_statistics() {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        std::unordered_map<std::string, double> stats;

        stats["total_processed"] = processed_count_.load();
        stats["queue_size"] = data_queue_.size();

        for (const auto& [sensor_type, values] : sensor_stats_) {
            if (!values.empty()) {
                double sum = 0.0;
                double min_val = *std::min_element(values.begin(), values.end());
                double max_val = *std::max_element(values.begin(), values.end());

                for (double val : values) {
                    sum += val;
                }

                stats[sensor_type + "_count"] = values.size();
                stats[sensor_type + "_avg"] = sum / values.size();
                stats[sensor_type + "_min"] = min_val;
                stats[sensor_type + "_max"] = max_val;
            }
        }

        return stats;
    }

private:
    void processing_loop() {
        while (running_) {
            std::vector<SensorReading> batch;

            // Wait for data or timeout
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                if (queue_cv_.wait_for(lock, std::chrono::milliseconds(100),
                    [this]() { return !data_queue_.empty() || !running_; })) {

                    // Collect batch
                    while (!data_queue_.empty() && batch.size() < batch_size_) {
                        batch.push_back(data_queue_.front());
                        data_queue_.pop();
                    }
                }
            }

            if (!batch.empty()) {
                process_batch(batch);
            }
        }

        // Process remaining items
        std::vector<SensorReading> remaining;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            while (!data_queue_.empty()) {
                remaining.push_back(data_queue_.front());
                data_queue_.pop();
            }
        }

        if (!remaining.empty()) {
            process_batch(remaining);
        }
    }

    void process_batch(const std::vector<SensorReading>& batch) {
        std::cout << "🔄 Processing batch of " << batch.size() << " sensor readings" << std::endl;

        // Create UMICP envelope for batch
        umicp_envelope_t* batch_envelope = create_batch_envelope(batch);

        // Process individual readings
        for (const auto& reading : batch) {
            process_reading(reading);
            processed_count_++;
        }

        // Simulate sending envelope (in real system, this would be sent to server)
        std::cout << "📤 Batch envelope created - Size: " << calculate_envelope_size(batch_envelope) << " bytes" << std::endl;

        // Clean up
        umicp_free_envelope(batch_envelope);
    }

    void process_reading(const SensorReading& reading) {
        // Update statistics
        {
            std::unique_lock<std::mutex> lock(stats_mutex_);
            sensor_stats_[reading.sensor_type].push_back(reading.value);

            // Keep only last 1000 readings for memory efficiency
            if (sensor_stats_[reading.sensor_type].size() > 1000) {
                sensor_stats_[reading.sensor_type].erase(
                    sensor_stats_[reading.sensor_type].begin()
                );
            }
        }

        // Apply sensor-specific processing
        if (reading.sensor_type == "temperature") {
            process_temperature_reading(reading);
        } else if (reading.sensor_type == "humidity") {
            process_humidity_reading(reading);
        } else if (reading.sensor_type == "motion") {
            process_motion_reading(reading);
        } else if (reading.sensor_type == "light") {
            process_light_reading(reading);
        }

        // Check for anomalies
        if (detect_anomaly(reading)) {
            std::cout << "⚠️  Anomaly detected in " << reading.sensor_id
                      << ": " << reading.value << reading.unit << std::endl;
        }
    }

    void process_temperature_reading(const SensorReading& reading) {
        // Temperature-specific processing
        if (reading.value > 30.0f) {
            std::cout << "🔥 High temperature alert: " << reading.sensor_id
                      << " = " << reading.value << "°C" << std::endl;
        } else if (reading.value < 5.0f) {
            std::cout << "❄️  Low temperature alert: " << reading.sensor_id
                      << " = " << reading.value << "°C" << std::endl;
        }
    }

    void process_humidity_reading(const SensorReading& reading) {
        // Humidity-specific processing
        if (reading.value > 80.0f) {
            std::cout << "💧 High humidity alert: " << reading.sensor_id
                      << " = " << reading.value << "%" << std::endl;
        }
    }

    void process_motion_reading(const SensorReading& reading) {
        // Motion detection
        if (reading.value > 0.5f) {
            std::cout << "🚶 Motion detected: " << reading.sensor_id << std::endl;
        }
    }

    void process_light_reading(const SensorReading& reading) {
        // Light level processing
        if (reading.value < 10.0f) {
            std::cout << "🌙 Low light conditions: " << reading.sensor_id
                      << " = " << reading.value << " lux" << std::endl;
        }
    }

    bool detect_anomaly(const SensorReading& reading) {
        // Simple anomaly detection based on statistical analysis
        std::unique_lock<std::mutex> lock(stats_mutex_);
        const auto& values = sensor_stats_[reading.sensor_type];

        if (values.size() < 10) {
            return false; // Need minimum data for anomaly detection
        }

        // Calculate mean and standard deviation
        double sum = 0.0;
        for (double val : values) {
            sum += val;
        }
        double mean = sum / values.size();

        double variance = 0.0;
        for (double val : values) {
            double diff = val - mean;
            variance += diff * diff;
        }
        variance /= values.size();
        double std_dev = std::sqrt(variance);

        // Check if reading is 3 standard deviations from mean
        double z_score = std::abs(reading.value - mean) / std_dev;
        return z_score > 3.0;
    }

    umicp_envelope_t* create_batch_envelope(const std::vector<SensorReading>& batch) {
        umicp_envelope_t* envelope = umicp_create_envelope();

        umicp_set_from(envelope, "iot-data-processor");
        umicp_set_to(envelope, "data-analytics-server");
        umicp_set_operation(envelope, UMICP_OPERATION_DATA);

        // Generate unique message ID based on timestamp
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        std::string message_id = "batch-" + std::to_string(timestamp);
        umicp_set_message_id(envelope, message_id.c_str());

        // Add batch metadata
        umicp_set_capability(envelope, "message_type", "sensor_batch");
        umicp_set_capability(envelope, "batch_size", std::to_string(batch.size()).c_str());
        umicp_set_capability(envelope, "timestamp", std::to_string(timestamp).c_str());

        // Count sensor types
        std::unordered_map<std::string, size_t> sensor_counts;
        for (const auto& reading : batch) {
            sensor_counts[reading.sensor_type]++;
        }

        std::string sensor_types;
        for (const auto& [type, count] : sensor_counts) {
            if (!sensor_types.empty()) sensor_types += ",";
            sensor_types += type + ":" + std::to_string(count);
        }
        umicp_set_capability(envelope, "sensor_types", sensor_types.c_str());

        // Add batch data as JSON-like string
        std::string batch_data = "[";
        for (size_t i = 0; i < batch.size(); ++i) {
            const auto& reading = batch[i];
            if (i > 0) batch_data += ",";

            batch_data += "{";
            batch_data += "\"id\":\"" + reading.sensor_id + "\",";
            batch_data += "\"type\":\"" + reading.sensor_type + "\",";
            batch_data += "\"value\":" + std::to_string(reading.value) + ",";
            batch_data += "\"unit\":\"" + reading.unit + "\",";
            batch_data += "\"location\":\"" + reading.location + "\"";
            batch_data += "}";
        }
        batch_data += "]";

        umicp_set_capability(envelope, "batch_data", batch_data.c_str());

        return envelope;
    }

    size_t calculate_envelope_size(umicp_envelope_t* envelope) {
        // Get serialized envelope and calculate size
        const char* serialized = umicp_serialize_envelope(envelope);
        return strlen(serialized);
    }
};

class IoTSensorSimulator {
private:
    std::vector<SensorConfig> sensors_;
    IoTDataProcessor& processor_;
    std::thread simulation_thread_;
    std::atomic<bool> running_{true};

public:
    IoTSensorSimulator(IoTDataProcessor& processor)
        : processor_(processor) {
        initialize_sensors();
    }

    ~IoTSensorSimulator() {
        running_ = false;
        if (simulation_thread_.joinable()) {
            simulation_thread_.join();
        }
    }

    void start_simulation(size_t update_interval_ms = 500) {
        simulation_thread_ = std::thread([this, update_interval_ms]() {
            while (running_) {
                generate_readings();
                std::this_thread::sleep_for(std::chrono::milliseconds(update_interval_ms));
            }
        });
    }

    void stop_simulation() {
        running_ = false;
    }

private:
    void initialize_sensors() {
        // Temperature sensors
        sensors_.push_back({"temp-001", "temperature", 22.0f, 3.0f, "°C", "warehouse-a"});
        sensors_.push_back({"temp-002", "temperature", 18.0f, 2.0f, "°C", "warehouse-b"});
        sensors_.push_back({"temp-003", "temperature", 24.0f, 1.0f, "°C", "office"});

        // Humidity sensors
        sensors_.push_back({"hum-001", "humidity", 65.0f, 10.0f, "%", "warehouse-a"});
        sensors_.push_back({"hum-002", "humidity", 70.0f, 8.0f, "%", "warehouse-b"});

        // Motion sensors
        sensors_.push_back({"motion-001", "motion", 0.0f, 1.0f, "boolean", "entrance"});
        sensors_.push_back({"motion-002", "motion", 0.0f, 1.0f, "boolean", "storage-room"});

        // Light sensors
        sensors_.push_back({"light-001", "light", 500.0f, 100.0f, "lux", "warehouse-a"});
    }

    void generate_readings() {
        for (const auto& sensor : sensors_) {
            float value = generate_sensor_value(sensor);
            SensorReading reading(sensor.id, sensor.type, value,
                                sensor.unit, sensor.location);

            // Add metadata
            reading.metadata["battery_level"] = 85.0f + (rand() % 16); // 85-100%
            reading.metadata["signal_strength"] = -30.0f + (rand() % 21); // -30 to -10 dBm
            reading.metadata["calibration_offset"] = ((rand() % 201) - 100) * 0.001f; // -0.1 to 0.1

            processor_.add_reading(reading);
        }
    }

    float generate_sensor_value(const SensorConfig& sensor) {
        // Base value with random variation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dist(sensor.base_value, sensor.variance);

        float value = dist(gen);

        // Apply time-based variations for realism
        auto now = std::chrono::system_clock::now();
        auto hours = std::chrono::duration_cast<std::chrono::hours>(
            now.time_since_epoch()).count() % 24;

        if (sensor.type == "temperature") {
            // Temperature varies with time of day
            value += std::sin((hours - 6) * 3.14159 / 12.0) * 2.0f;
        } else if (sensor.type == "light") {
            // Light varies with time of day
            value = std::max(10.0f, value * std::sin((hours - 6) * 3.14159 / 12.0));
        } else if (sensor.type == "motion") {
            // Random motion detection
            value = (rand() % 100) < 5 ? 1.0f : 0.0f; // 5% chance
        } else if (sensor.type == "humidity") {
            // Humidity stays within bounds
            value = std::max(0.0f, std::min(100.0f, value));
        }

        return value;
    }

    struct SensorConfig {
        std::string id;
        std::string type;
        float base_value;
        float variance;
        std::string unit;
        std::string location;
    };
};

int main() {
    std::cout << "🌡️ UMICP C++ IoT Data Processor" << std::endl;
    std::cout << "==============================" << std::endl;

    // Create data processor
    IoTDataProcessor processor;

    // Create sensor simulator
    IoTSensorSimulator simulator(processor);

    std::cout << "📡 Starting IoT simulation..." << std::endl;
    std::cout << "💡 Press Ctrl+C to stop" << std::endl;
    std::cout << std::endl;

    // Start simulation
    simulator.start_simulation(300); // Update every 300ms

    // Run for 10 seconds
    auto start_time = std::chrono::steady_clock::now();
    size_t report_interval = 0;

    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(10)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Periodic reporting
        report_interval++;
        if (report_interval % 2 == 0) {
            auto stats = processor.get_statistics();
            std::cout << "📊 Statistics Update:" << std::endl;
            std::cout << "   Total processed: " << stats["total_processed"] << std::endl;
            std::cout << "   Queue size: " << stats["queue_size"] << std::endl;

            if (stats.count("temperature_count")) {
                std::cout << "   Temperature sensors: " << stats["temperature_count"]
                          << " (avg: " << stats["temperature_avg"] << "°C)" << std::endl;
            }
            if (stats.count("humidity_count")) {
                std::cout << "   Humidity sensors: " << stats["humidity_count"]
                          << " (avg: " << stats["humidity_avg"] << "%)" << std::endl;
            }
            std::cout << std::endl;
        }
    }

    // Stop simulation
    simulator.stop_simulation();

    // Final statistics
    auto final_stats = processor.get_statistics();

    std::cout << "🎉 IoT Processing Complete!" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << "📊 Final Statistics:" << std::endl;
    std::cout << "   Total readings processed: " << final_stats["total_processed"] << std::endl;
    std::cout << "   Final queue size: " << final_stats["queue_size"] << std::endl;
    std::cout << std::endl;

    // Show detailed statistics for each sensor type
    for (const auto& [key, value] : final_stats) {
        if (key != "total_processed" && key != "queue_size") {
            std::cout << "   " << key << ": " << value << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "💡 Key Features Demonstrated:" << std::endl;
    std::cout << "   • Real-time sensor data processing" << std::endl;
    std::cout << "   • Batch processing with UMICP envelopes" << std::endl;
    std::cout << "   • Anomaly detection algorithms" << std::endl;
    std::cout << "   • Statistical analysis and monitoring" << std::endl;
    std::cout << "   • Multi-threaded data processing" << std::endl;
    std::cout << "   • Memory-efficient data structures" << std::endl;

    return 0;
}
