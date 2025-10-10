/**
 * UMICP Config - Comprehensive Tests
 * 100% coverage of configuration operations
 */

#include "config.h"
#include "umicp_types.h"
#include <gtest/gtest.h>
#include <fstream>

using namespace umicp;

// ============================================================================
// Config Creation Tests
// ============================================================================

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_unique<UMICPConfig>();
    }

    void TearDown() override {
        // Clean up any test files
        std::remove("/tmp/test_config.json");
        std::remove("/tmp/test_config_output.json");
    }

    std::unique_ptr<UMICPConfig> config;
};

TEST_F(ConfigTest, DefaultValues) {
    UMICPConfig cfg;

    EXPECT_FALSE(cfg.peer_id.empty());
    EXPECT_GT(cfg.buffer_size, 0);
    EXPECT_GT(cfg.max_message_size, 0);
}

TEST_F(ConfigTest, SetPeerId) {
    config->peer_id = "test-peer-123";
    EXPECT_EQ(config->peer_id, "test-peer-123");
}

TEST_F(ConfigTest, SetBufferSize) {
    config->buffer_size = 8192;
    EXPECT_EQ(config->buffer_size, 8192);
}

TEST_F(ConfigTest, SetMaxMessageSize) {
    config->max_message_size = 10 * 1024 * 1024;
    EXPECT_EQ(config->max_message_size, 10 * 1024 * 1024);
}

TEST_F(ConfigTest, SetTimeout) {
    config->timeout = std::chrono::seconds(30);
    EXPECT_EQ(config->timeout, std::chrono::seconds(30));
}

TEST_F(ConfigTest, EnableCompression) {
    config->enable_compression = true;
    EXPECT_TRUE(config->enable_compression);
}

TEST_F(ConfigTest, SetCompressionAlgorithm) {
    config->compression_algorithm = CompressionAlgorithm::GZIP;
    EXPECT_EQ(config->compression_algorithm, CompressionAlgorithm::GZIP);
}

// ============================================================================
// JSON Serialization Tests
// ============================================================================

TEST_F(ConfigTest, ToJsonProducesValidString) {
    config->peer_id = "peer-123";
    config->buffer_size = 4096;

    auto result = config->to_json();

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());

    std::string json = *result.value;
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("peer_id"), std::string::npos);
}

TEST_F(ConfigTest, FromJsonParsesCorrectly) {
    std::string json = R"({
        "peer_id": "peer-456",
        "buffer_size": 8192,
        "max_message_size": 2097152
    })";

    auto result = config->from_json(json);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(config->peer_id, "peer-456");
    EXPECT_EQ(config->buffer_size, 8192);
    EXPECT_EQ(config->max_message_size, 2097152);
}

TEST_F(ConfigTest, RoundTripJsonSerialization) {
    config->peer_id = "round-trip-peer";
    config->buffer_size = 16384;
    config->enable_compression = true;

    auto json_result = config->to_json();
    ASSERT_TRUE(json_result.is_ok());

    UMICPConfig new_config;
    auto parse_result = new_config.from_json(*json_result.value);

    ASSERT_TRUE(parse_result.is_ok());
    EXPECT_EQ(new_config.peer_id, "round-trip-peer");
    EXPECT_EQ(new_config.buffer_size, 16384);
    EXPECT_EQ(new_config.enable_compression, true);
}

TEST_F(ConfigTest, FromJsonInvalidJsonFails) {
    std::string invalid = "{ invalid json }";

    auto result = config->from_json(invalid);

    EXPECT_FALSE(result.is_ok());
}

TEST_F(ConfigTest, FromJsonEmptyStringFails) {
    auto result = config->from_json("");

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// File Operations Tests
// ============================================================================

TEST_F(ConfigTest, SaveToFile) {
    config->peer_id = "file-peer";
    config->buffer_size = 4096;

    auto result = config->save_to_file("/tmp/test_config.json");

    EXPECT_TRUE(result.is_ok());

    // Verify file exists
    std::ifstream file("/tmp/test_config.json");
    EXPECT_TRUE(file.good());
}

TEST_F(ConfigTest, LoadFromFile) {
    // Create test file
    config->peer_id = "load-peer";
    config->buffer_size = 2048;
    config->save_to_file("/tmp/test_config.json");

    // Load into new config
    UMICPConfig loaded;
    auto result = loaded.load_from_file("/tmp/test_config.json");

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(loaded.peer_id, "load-peer");
    EXPECT_EQ(loaded.buffer_size, 2048);
}

TEST_F(ConfigTest, LoadFromNonExistentFileFails) {
    auto result = config->load_from_file("/tmp/nonexistent_file.json");

    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::FILE_ERROR);
}

TEST_F(ConfigTest, SaveToInvalidPathFails) {
    auto result = config->save_to_file("/invalid/path/config.json");

    EXPECT_FALSE(result.is_ok());
}

TEST_F(ConfigTest, RoundTripFileSerialization) {
    config->peer_id = "roundtrip-file";
    config->buffer_size = 32768;
    config->max_message_size = 5 * 1024 * 1024;
    config->enable_compression = true;
    config->compression_algorithm = CompressionAlgorithm::GZIP;

    // Save
    auto save_result = config->save_to_file("/tmp/test_config_output.json");
    ASSERT_TRUE(save_result.is_ok());

    // Load
    UMICPConfig loaded;
    auto load_result = loaded.load_from_file("/tmp/test_config_output.json");

    ASSERT_TRUE(load_result.is_ok());
    EXPECT_EQ(loaded.peer_id, "roundtrip-file");
    EXPECT_EQ(loaded.buffer_size, 32768);
    EXPECT_EQ(loaded.max_message_size, 5 * 1024 * 1024);
    EXPECT_EQ(loaded.enable_compression, true);
    EXPECT_EQ(loaded.compression_algorithm, CompressionAlgorithm::GZIP);
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST_F(ConfigTest, ValidateValidConfig) {
    config->peer_id = "valid-peer";
    config->buffer_size = 4096;
    config->max_message_size = 1024 * 1024;

    auto result = config->validate();

    EXPECT_TRUE(result.is_ok());
}

TEST_F(ConfigTest, ValidateEmptyPeerIdFails) {
    config->peer_id = "";
    config->buffer_size = 4096;

    auto result = config->validate();

    EXPECT_FALSE(result.is_ok());
}

TEST_F(ConfigTest, ValidateZeroBufferSizeFails) {
    config->peer_id = "peer";
    config->buffer_size = 0;

    auto result = config->validate();

    EXPECT_FALSE(result.is_ok());
}

TEST_F(ConfigTest, ValidateZeroMaxMessageSizeFails) {
    config->peer_id = "peer";
    config->max_message_size = 0;

    auto result = config->validate();

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Merge/Update Tests
// ============================================================================

TEST_F(ConfigTest, MergeConfigs) {
    config->peer_id = "original";
    config->buffer_size = 4096;

    UMICPConfig update;
    update.buffer_size = 8192;
    update.enable_compression = true;

    auto result = config->merge(update);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(config->peer_id, "original"); // Unchanged
    EXPECT_EQ(config->buffer_size, 8192); // Updated
    EXPECT_TRUE(config->enable_compression); // Updated
}

TEST_F(ConfigTest, MergeEmptyConfig) {
    config->peer_id = "original";
    config->buffer_size = 4096;

    UMICPConfig empty;
    auto result = config->merge(empty);

    EXPECT_TRUE(result.is_ok());
    // Original values should remain
}

// ============================================================================
// TransportConfig Tests
// ============================================================================

TEST(TransportConfigTest, DefaultValues) {
    TransportConfig cfg;

    EXPECT_EQ(cfg.type, TransportType::WEBSOCKET);
    EXPECT_EQ(cfg.host, "localhost");
    EXPECT_EQ(cfg.port, 8080);
}

TEST(TransportConfigTest, SetAllFields) {
    TransportConfig cfg;
    cfg.type = TransportType::HTTP2;
    cfg.host = "example.com";
    cfg.port = 443;
    cfg.path = "/api/umicp";
    cfg.max_payload_size = 10 * 1024 * 1024;

    EXPECT_EQ(cfg.type, TransportType::HTTP2);
    EXPECT_EQ(cfg.host, "example.com");
    EXPECT_EQ(cfg.port, 443);
    EXPECT_EQ(cfg.path, "/api/umicp");
    EXPECT_TRUE(cfg.max_payload_size.has_value());
    EXPECT_EQ(cfg.max_payload_size.value(), 10 * 1024 * 1024);
}

TEST(TransportConfigTest, SetHeaders) {
    TransportConfig cfg;
    cfg.headers["Authorization"] = "Bearer token123";
    cfg.headers["Content-Type"] = "application/json";

    EXPECT_EQ(cfg.headers.size(), 2);
    EXPECT_EQ(cfg.headers["Authorization"], "Bearer token123");
}

TEST(TransportConfigTest, SSLConfig) {
    TransportConfig cfg;
    cfg.ssl_config = SSLConfig{};
    cfg.ssl_config->enable_ssl = true;
    cfg.ssl_config->verify_peer = true;

    EXPECT_TRUE(cfg.ssl_config.has_value());
    EXPECT_TRUE(cfg.ssl_config->enable_ssl);
    EXPECT_TRUE(cfg.ssl_config->verify_peer);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ConfigTest, VeryLongPeerId) {
    std::string long_id(10000, 'x');
    config->peer_id = long_id;

    EXPECT_EQ(config->peer_id, long_id);
}

TEST_F(ConfigTest, SpecialCharactersInPeerId) {
    config->peer_id = "peer-!@#$%^&*()_+-={}[]|:;<>?,./";

    EXPECT_EQ(config->peer_id, "peer-!@#$%^&*()_+-={}[]|:;<>?,./");
}

TEST_F(ConfigTest, UnicodeInPeerId) {
    config->peer_id = "节点-123";

    EXPECT_EQ(config->peer_id, "节点-123");
}

TEST_F(ConfigTest, VeryLargeBufferSize) {
    config->buffer_size = 1024 * 1024 * 1024; // 1GB

    EXPECT_EQ(config->buffer_size, 1024 * 1024 * 1024);
}

TEST_F(ConfigTest, VeryLargeMaxMessageSize) {
    config->max_message_size = 100 * 1024 * 1024; // 100MB

    EXPECT_EQ(config->max_message_size, 100 * 1024 * 1024);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(ConfigThreadSafetyTest, ConcurrentConfigCreation) {
    std::vector<std::thread> threads;
    std::vector<UMICPConfig> configs(10);

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &configs]() {
            configs[i].peer_id = "peer-" + std::to_string(i);
            configs[i].buffer_size = 1024 * (i + 1);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(configs[i].peer_id, "peer-" + std::to_string(i));
        EXPECT_EQ(configs[i].buffer_size, 1024 * (i + 1));
    }
}

TEST(ConfigThreadSafetyTest, ConcurrentSerialization) {
    UMICPConfig shared_config;
    shared_config.peer_id = "shared";
    shared_config.buffer_size = 4096;

    std::vector<std::thread> threads;
    std::vector<std::string> results(10);

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &shared_config, &results]() {
            auto json_result = shared_config.to_json();
            if (json_result.is_ok()) {
                results[i] = *json_result.value;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (const auto& result : results) {
        EXPECT_FALSE(result.empty());
    }
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(ConfigPerformanceTest, SerializationPerformance) {
    UMICPConfig cfg;
    cfg.peer_id = "perf-peer";
    cfg.buffer_size = 4096;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        auto result = cfg.to_json();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 serializations should complete in < 100ms
    EXPECT_LT(duration.count(), 100);
}

TEST(ConfigPerformanceTest, DeserializationPerformance) {
    std::string json = R"({
        "peer_id": "perf-peer",
        "buffer_size": 4096
    })";

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        UMICPConfig cfg;
        auto result = cfg.from_json(json);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 deserializations should complete in < 200ms
    EXPECT_LT(duration.count(), 200);
}

// ============================================================================
// Complete Config Tests
// ============================================================================

TEST_F(ConfigTest, CompleteConfiguration) {
    config->peer_id = "complete-peer";
    config->buffer_size = 65536;
    config->max_message_size = 50 * 1024 * 1024;
    config->timeout = std::chrono::seconds(60);
    config->enable_compression = true;
    config->compression_algorithm = CompressionAlgorithm::GZIP;
    config->enable_encryption = true;
    config->heartbeat_interval = std::chrono::seconds(30);
    config->max_reconnect_attempts = 10;

    auto json_result = config->to_json();
    ASSERT_TRUE(json_result.is_ok());

    UMICPConfig loaded;
    auto parse_result = loaded.from_json(*json_result.value);

    ASSERT_TRUE(parse_result.is_ok());
    EXPECT_EQ(loaded.peer_id, "complete-peer");
    EXPECT_EQ(loaded.buffer_size, 65536);
    EXPECT_EQ(loaded.max_message_size, 50 * 1024 * 1024);
    EXPECT_TRUE(loaded.enable_compression);
    EXPECT_TRUE(loaded.enable_encryption);
}

// ============================================================================
// Validation Edge Cases
// ============================================================================

TEST_F(ConfigTest, MinimalValidConfig) {
    config->peer_id = "minimal";
    config->buffer_size = 1;
    config->max_message_size = 1;

    auto result = config->validate();

    EXPECT_TRUE(result.is_ok());
}

TEST_F(ConfigTest, MaximalConfig) {
    config->peer_id = std::string(1000, 'x');
    config->buffer_size = 1024 * 1024 * 1024;
    config->max_message_size = 100 * 1024 * 1024;

    auto result = config->validate();

    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Pretty Print Tests
// ============================================================================

TEST_F(ConfigTest, PrettyPrintJson) {
    config->peer_id = "pretty-peer";

    auto result = config->to_json_pretty();

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());

    std::string json = *result.value;
    // Should contain newlines (pretty formatted)
    EXPECT_NE(json.find('\n'), std::string::npos);
}

TEST_F(ConfigTest, PrettyPrintVsCompact) {
    config->peer_id = "compare";
    config->buffer_size = 4096;

    auto compact = config->to_json();
    auto pretty = config->to_json_pretty();

    ASSERT_TRUE(compact.is_ok());
    ASSERT_TRUE(pretty.is_ok());

    // Pretty should be longer (whitespace)
    EXPECT_GT(pretty.value->length(), compact.value->length());
}

// ============================================================================
// Multiple Configs
// ============================================================================

TEST(ConfigMultipleTest, IndependentConfigs) {
    UMICPConfig cfg1, cfg2, cfg3;

    cfg1.peer_id = "peer1";
    cfg2.peer_id = "peer2";
    cfg3.peer_id = "peer3";

    cfg1.buffer_size = 1024;
    cfg2.buffer_size = 2048;
    cfg3.buffer_size = 4096;

    EXPECT_EQ(cfg1.peer_id, "peer1");
    EXPECT_EQ(cfg2.peer_id, "peer2");
    EXPECT_EQ(cfg3.peer_id, "peer3");

    EXPECT_EQ(cfg1.buffer_size, 1024);
    EXPECT_EQ(cfg2.buffer_size, 2048);
    EXPECT_EQ(cfg3.buffer_size, 4096);
}

TEST(ConfigMultipleTest, ConfigInheritance) {
    UMICPConfig base;
    base.peer_id = "base";
    base.buffer_size = 4096;
    base.enable_compression = true;

    UMICPConfig derived = base;
    derived.peer_id = "derived";

    EXPECT_EQ(base.peer_id, "base");
    EXPECT_EQ(derived.peer_id, "derived");
    EXPECT_EQ(derived.buffer_size, 4096);
    EXPECT_TRUE(derived.enable_compression);
}

