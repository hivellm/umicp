/**
 * SSL/TLS and Compression Tests
 * Tests for secure transport and message compression
 */

#include <gtest/gtest.h>
#include "umicp_types.h"
#include "compression.h"
#include "transport.h"
#include "websocket_lws.h"
#include "protocol.h"
#include "security.h"
#include "../utils/test_helpers.h"

using namespace umicp;
using namespace umicp::testing;

class SSLCompressionTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();
    }

    void TearDown() override {
        // Cleanup SSL certificates if created
    }
};

// ===============================================
// Compression Tests
// ===============================================

TEST_F(SSLCompressionTest, Compression_ZlibBasic) {
    CompressionManager compression(CompressionAlgorithm::ZLIB);

    // Create test data
    std::string original = "This is a test string for compression. ";
    for (int i = 0; i < 100; ++i) {
        original += "Additional test data to make compression worthwhile. ";
    }

    ByteBuffer data(original.begin(), original.end());

    // Compress
    auto compress_result = compression.compress(data);
    ASSERT_TRUE(compress_result.is_success());
    ASSERT_LT(compress_result.value->size(), data.size()); // Should be smaller

    // Decompress
    auto decompress_result = compression.decompress(*compress_result.value);
    ASSERT_TRUE(decompress_result.is_success());
    ASSERT_EQ(*decompress_result.value, data); // Should match original
}

TEST_F(SSLCompressionTest, Compression_EmptyData) {
    CompressionManager compression(CompressionAlgorithm::ZLIB);

    ByteBuffer empty_data;
    auto result = compression.compress(empty_data);
    ASSERT_TRUE(result.is_success());
    ASSERT_EQ(*result.value, empty_data);
}

TEST_F(SSLCompressionTest, Compression_SmallData) {
    CompressionManager compression(CompressionAlgorithm::ZLIB);

    // Data smaller than compression threshold
    ByteBuffer small_data = {'a', 'b', 'c'};
    auto result = compression.compress(small_data);
    ASSERT_TRUE(result.is_success());
    // Small data might not compress much or at all
    ASSERT_LE(result.value->size(), small_data.size() * 2);
}

TEST_F(SSLCompressionTest, Compression_AlgorithmSwitching) {
    CompressionManager compression(CompressionAlgorithm::NONE);

    ByteBuffer data(1000, 'x'); // Large repetitive data

    // Test no compression
    auto none_result = compression.compress(data);
    ASSERT_TRUE(none_result.is_success());
    ASSERT_EQ(*none_result.value, data); // Should be identical

    // Switch to ZLIB
    compression.set_algorithm(CompressionAlgorithm::ZLIB);
    auto zlib_result = compression.compress(data);
    ASSERT_TRUE(zlib_result.is_success());
    ASSERT_LT(zlib_result.value->size(), data.size()); // Should be compressed
}

TEST_F(SSLCompressionTest, Compression_InvalidAlgorithm) {
    CompressionManager compression(static_cast<CompressionAlgorithm>(999));

    ByteBuffer data = {'t', 'e', 's', 't'};
    auto result = compression.compress(data);
    ASSERT_FALSE(result.is_success());
    ASSERT_EQ(result.code, ErrorCode::NOT_IMPLEMENTED);
}

// ===============================================
// SSL/TLS Configuration Tests
// ===============================================

TEST_F(SSLCompressionTest, SSL_ConfigCreation) {
    SSLConfig ssl_config;
    ssl_config.enable_ssl = true;
    ssl_config.verify_peer = true;
    ssl_config.verify_host = true;
    ssl_config.ca_file = "/etc/ssl/certs/ca-certificates.crt";
    ssl_config.cert_file = "/path/to/cert.pem";
    ssl_config.key_file = "/path/to/key.pem";

    ASSERT_TRUE(ssl_config.enable_ssl);
    ASSERT_TRUE(ssl_config.verify_peer);
    ASSERT_TRUE(ssl_config.verify_host);
    ASSERT_EQ(ssl_config.ca_file, "/etc/ssl/certs/ca-certificates.crt");
}

TEST_F(SSLCompressionTest, Transport_SSLConfigIntegration) {
    TransportConfig config;
    config.type = TransportType::WEBSOCKET;
    config.host = "secure.example.com";
    config.port = 443;

    SSLConfig ssl_config;
    ssl_config.enable_ssl = true;
    ssl_config.verify_peer = true;
    ssl_config.cert_file = "/etc/ssl/certs/client.crt";
    ssl_config.key_file = "/etc/ssl/private/client.key";

    config.ssl_config = ssl_config;

    ASSERT_TRUE(config.ssl_config.has_value());
    ASSERT_TRUE(config.ssl_config->enable_ssl);
    ASSERT_EQ(config.ssl_config->cert_file, "/etc/ssl/certs/client.crt");
}

TEST_F(SSLCompressionTest, SSL_DisabledConfig) {
    SSLConfig ssl_config;
    // Default should be SSL disabled
    ASSERT_FALSE(ssl_config.enable_ssl);
    ASSERT_TRUE(ssl_config.verify_peer); // But verification enabled by default
}

// ===============================================
// Protocol Compression Integration Tests
// ===============================================

TEST_F(SSLCompressionTest, Protocol_CompressionIntegration) {
    Protocol protocol("test-node");

    UMICPConfig config;
    config.enable_compression = true;
    config.compression_threshold = 100;
    config.compression_algorithm = CompressionAlgorithm::ZLIB;

    auto configure_result = protocol.configure(config);
    ASSERT_TRUE(configure_result.is_success());

    // Test that protocol is configured with compression
    // This is mainly a configuration test
    ASSERT_TRUE(config.enable_compression);
    ASSERT_EQ(config.compression_threshold, 100);
    ASSERT_EQ(config.compression_algorithm, CompressionAlgorithm::ZLIB);
}

TEST_F(SSLCompressionTest, Protocol_CompressionDisabled) {
    Protocol protocol("test-node");

    UMICPConfig config;
    config.enable_compression = false;

    auto configure_result = protocol.configure(config);
    ASSERT_TRUE(configure_result.is_success());
    ASSERT_FALSE(config.enable_compression);
}

// ===============================================
// Security Integration Tests
// ===============================================

TEST_F(SSLCompressionTest, Security_SSLRequirement) {
    SecurityManager security("test-node");

    // Test that security manager can be created and configured
    auto keypair_result = security.generate_keypair();
    ASSERT_TRUE(keypair_result.is_success());

    // Test signature operations
    ByteBuffer test_data = {'t', 'e', 's', 't'};
    auto sign_result = security.sign_data(test_data);
    ASSERT_TRUE(sign_result.is_success());
    ASSERT_FALSE(sign_result.value->empty());
}

TEST_F(SSLCompressionTest, Security_EncryptionDecryption) {
    SecurityManager security("test-node");

    // Generate keys and establish session
    auto keypair_result = security.generate_keypair();
    ASSERT_TRUE(keypair_result.is_success());

    // For MVP, encryption/decryption might not be fully implemented
    // This test mainly verifies the interface
    ByteBuffer test_data = {'s', 'e', 'c', 'r', 'e', 't'};
    auto encrypt_result = security.encrypt_data(test_data);

    // Depending on implementation, this might succeed or fail
    // The important thing is that it doesn't crash
    if (encrypt_result.is_success()) {
        ASSERT_FALSE(encrypt_result.value->empty());
    } else {
        // Expected to fail in MVP without session
        ASSERT_EQ(encrypt_result.code, ErrorCode::INVALID_ARGUMENT);
    }
}

// ===============================================
// HTTP/2 SSL/TLS Tests
// ===============================================

TEST_F(SSLCompressionTest, HTTP2_SSLConfigIntegration) {
    // Test HTTP/2 transport with SSL configuration
    TransportConfig config;
    config.type = TransportType::HTTP2;
    config.host = "secure-http2.example.com";
    config.port = 443;

    SSLConfig ssl_config;
    ssl_config.enable_ssl = true;
    ssl_config.verify_peer = true;
    ssl_config.ca_file = "/etc/ssl/certs/ca-certificates.crt";
    ssl_config.cert_file = "/etc/ssl/certs/client.crt";
    ssl_config.key_file = "/etc/ssl/private/client.key";

    config.ssl_config = ssl_config;

    // Verify configuration is properly set for HTTP/2
    ASSERT_TRUE(config.ssl_config.has_value());
    ASSERT_TRUE(config.ssl_config->enable_ssl);
    ASSERT_EQ(config.ssl_config->ca_file, "/etc/ssl/certs/ca-certificates.crt");
    ASSERT_EQ(config.type, TransportType::HTTP2);
}

TEST_F(SSLCompressionTest, HTTP2_SSLDisabled) {
    // Test HTTP/2 transport with SSL disabled
    TransportConfig config;
    config.type = TransportType::HTTP2;
    config.host = "insecure-http2.example.com";
    config.port = 8080;

    SSLConfig ssl_config;
    ssl_config.enable_ssl = false;

    config.ssl_config = ssl_config;

    // Verify SSL is properly disabled
    ASSERT_TRUE(config.ssl_config.has_value());
    ASSERT_FALSE(config.ssl_config->enable_ssl);
    ASSERT_EQ(config.port, 8080); // Should use regular HTTP port
}

TEST_F(SSLCompressionTest, HTTP2_SSLValidationOptions) {
    // Test various SSL validation options for HTTP/2
    SSLConfig ssl_config;
    ssl_config.enable_ssl = true;

    // Test peer verification options
    ssl_config.verify_peer = true;
    ASSERT_TRUE(ssl_config.verify_peer);

    ssl_config.verify_peer = false;
    ASSERT_FALSE(ssl_config.verify_peer);

    // Test host verification
    ssl_config.verify_host = true;
    ASSERT_TRUE(ssl_config.verify_host);

    // Test certificate files
    ssl_config.ca_file = "/etc/ssl/certs/ca-bundle.crt";
    ssl_config.cert_file = "/path/to/client.pem";
    ssl_config.key_file = "/path/to/client.key";

    ASSERT_EQ(ssl_config.ca_file, "/etc/ssl/certs/ca-bundle.crt");
    ASSERT_EQ(ssl_config.cert_file, "/path/to/client.pem");
    ASSERT_EQ(ssl_config.key_file, "/path/to/client.key");
}

// ===============================================
// BIP-05 Configuration Integration Tests
// ===============================================

TEST_F(SSLCompressionTest, BIP05_SSLConfigIntegration) {
    // Test that UMICPConfig settings are properly applied to transports
    UMICPConfig umicp_config;
    umicp_config.validate_certificates = true;
    umicp_config.enable_compression = true;
    umicp_config.compression_threshold = 512;

    // Test WebSocket transport configuration
    TransportConfig ws_config;
    ws_config.type = TransportType::WEBSOCKET;
    ws_config.host = "example.com";
    ws_config.port = 80; // HTTP port

    TransportConfig enhanced_ws = TransportFactory::apply_umicp_config(ws_config, umicp_config);

    // Should have SSL enabled and port changed to HTTPS
    ASSERT_TRUE(enhanced_ws.ssl_config.has_value());
    ASSERT_TRUE(enhanced_ws.ssl_config->enable_ssl);
    ASSERT_EQ(enhanced_ws.port, 443); // Should be changed to HTTPS port

    // Test HTTP/2 transport configuration
    TransportConfig h2_config;
    h2_config.type = TransportType::HTTP2;
    h2_config.host = "api.example.com";
    h2_config.port = 8080; // Custom HTTP port

    TransportConfig enhanced_h2 = TransportFactory::apply_umicp_config(h2_config, umicp_config);

    // Should have SSL enabled and port changed to SSL port
    ASSERT_TRUE(enhanced_h2.ssl_config.has_value());
    ASSERT_TRUE(enhanced_h2.ssl_config->enable_ssl);
    ASSERT_EQ(enhanced_h2.port, 8443); // Should be changed to SSL port
}

TEST_F(SSLCompressionTest, BIP05_CompressionConfigIntegration) {
    // Test compression configuration from UMICPConfig
    UMICPConfig umicp_config;
    umicp_config.enable_compression = true;
    umicp_config.compression_threshold = 256;
    umicp_config.compression_algorithm = CompressionAlgorithm::ZLIB;

    // Create protocol with compression config
    Protocol protocol("test-node");
    auto config_result = protocol.configure(umicp_config);
    ASSERT_TRUE(config_result.is_success());

    // Test compression with small data (below threshold)
    ByteBuffer small_data(100, 'x'); // 100 bytes < 256 threshold
    auto compress_small = CompressionManager::should_compress(small_data, umicp_config.compression_threshold, umicp_config.compression_algorithm);
    ASSERT_FALSE(compress_small); // Should not compress small data

    // Test compression with large data (above threshold)
    ByteBuffer large_data(500, 'y'); // 500 bytes > 256 threshold
    auto compress_large = CompressionManager::should_compress(large_data, umicp_config.compression_threshold, umicp_config.compression_algorithm);
    ASSERT_TRUE(compress_large); // Should compress large data
}

TEST_F(SSLCompressionTest, BIP05_SSLDisabledConfig) {
    // Test that SSL is not forced when validate_certificates is false
    UMICPConfig umicp_config;
    umicp_config.validate_certificates = false; // SSL disabled

    TransportConfig ws_config;
    ws_config.type = TransportType::WEBSOCKET;
    ws_config.host = "insecure.example.com";
    ws_config.port = 80;

    TransportConfig enhanced_ws = TransportFactory::apply_umicp_config(ws_config, umicp_config);

    // Should not have SSL configuration when disabled
    if (enhanced_ws.ssl_config) {
        ASSERT_FALSE(enhanced_ws.ssl_config->enable_ssl);
    }
    ASSERT_EQ(enhanced_ws.port, 80); // Port should remain unchanged
}

TEST_F(SSLCompressionTest, BIP05_CompressionDisabledConfig) {
    // Test compression behavior when disabled
    UMICPConfig umicp_config;
    umicp_config.enable_compression = false;

    ByteBuffer data(1000, 'z');
    auto should_compress = CompressionManager::should_compress(data, 512, CompressionAlgorithm::ZLIB);
    ASSERT_FALSE(should_compress); // Should never compress when disabled globally
}

// ===============================================
// End-to-End Integration Tests
// ===============================================

TEST_F(SSLCompressionTest, EndToEnd_CompressionWorkflow) {
    // Test the complete compression workflow
    CompressionManager compression(CompressionAlgorithm::ZLIB);

    // Create large test data
    ByteBuffer original_data(2000, 'A');
    for (size_t i = 0; i < original_data.size(); ++i) {
        original_data[i] = static_cast<uint8_t>(i % 256);
    }

    // Compress
    auto compress_result = compression.compress(original_data);
    ASSERT_TRUE(compress_result.is_success());

    // Verify compression actually happened
    ASSERT_LT(compress_result.value->size(), original_data.size());

    // Decompress
    auto decompress_result = compression.decompress(*compress_result.value);
    ASSERT_TRUE(decompress_result.is_success());

    // Verify round-trip integrity
    ASSERT_EQ(*decompress_result.value, original_data);
}

TEST_F(SSLCompressionTest, EndToEnd_SSLConfiguration) {
    // Test SSL configuration creation and validation
    TransportConfig config;
    config.type = TransportType::WEBSOCKET;
    config.host = "secure.example.com";
    config.port = 443;

    SSLConfig ssl_config;
    ssl_config.enable_ssl = true;
    ssl_config.verify_peer = true;
    ssl_config.ca_file = "/etc/ssl/certs/ca-certificates.crt";

    config.ssl_config = ssl_config;

    // Verify configuration is properly set
    ASSERT_TRUE(config.ssl_config.has_value());
    ASSERT_TRUE(config.ssl_config->enable_ssl);
    ASSERT_TRUE(config.ssl_config->verify_peer);
    ASSERT_EQ(config.ssl_config->ca_file, "/etc/ssl/certs/ca-certificates.crt");
    ASSERT_EQ(config.host, "secure.example.com");
    ASSERT_EQ(config.port, 443);
}

// ===============================================
// Production-Ready Features Tests
// ===============================================

TEST_F(SSLCompressionTest, ChaCha20Poly1305_EncryptionDecryption) {
    // Test ChaCha20-Poly1305 encryption/decryption
    SecurityManager security("test-node");

    // Generate keys and establish session
    auto keypair_result = security.generate_keypair();
    ASSERT_TRUE(keypair_result.is_success());

    // Create test data
    ByteBuffer test_data = {'s', 'e', 'c', 'r', 'e', 't', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'};
    auto encrypt_result = security.encrypt_data(test_data);
    ASSERT_TRUE(encrypt_result.is_success());
    ASSERT_FALSE(encrypt_result.value->empty());

    // Test decryption
    auto decrypt_result = security.decrypt_data(*encrypt_result.value);
    ASSERT_TRUE(decrypt_result.is_success());
    ASSERT_EQ(*decrypt_result.value, test_data);
}

TEST_F(SSLCompressionTest, LZ4_Compression) {
    // Test LZ4 compression
    CompressionManager compression(CompressionAlgorithm::LZ4);

    // Create test data (repetitive for good compression)
    std::string test_string = "This is a test string for LZ4 compression. ";
    for (int i = 0; i < 100; ++i) {
        test_string += "Additional test data to make compression worthwhile. ";
    }

    ByteBuffer data(test_string.begin(), test_string.end());

    // Test compression
    auto compress_result = compression.compress(data);
#ifdef HAVE_LZ4
    ASSERT_TRUE(compress_result.is_success());
    ASSERT_LT(compress_result.value->size(), data.size()); // Should be smaller

    // Test decompression
    auto decompress_result = compression.decompress(*compress_result.value);
    ASSERT_TRUE(decompress_result.is_success());
    ASSERT_EQ(*decompress_result.value, data);
#else
    // LZ4 not available, should return error
    ASSERT_FALSE(compress_result.is_success());
    ASSERT_EQ(compress_result.code, ErrorCode::NOT_IMPLEMENTED);
#endif
}

TEST_F(SSLCompressionTest, HardwareAcceleration_AES_NI) {
    // Test hardware acceleration detection
    SecurityManager security("test-node");

    // This is a compile-time test - we can't test runtime AES-NI without specific hardware
    // But we can verify the interface exists
    auto keypair_result = security.generate_keypair();
    ASSERT_TRUE(keypair_result.is_success());

    ByteBuffer test_data = {'t', 'e', 's', 't'};
    auto encrypt_result = security.encrypt_data(test_data);

    // Should succeed (with ChaCha20 fallback if AES-NI not available)
    ASSERT_TRUE(encrypt_result.is_success() || encrypt_result.code == ErrorCode::NOT_IMPLEMENTED);
}

TEST_F(SSLCompressionTest, Advanced_SSL_Configuration) {
    // Test advanced SSL configuration options
    SSLConfig ssl_config;
    ssl_config.enable_ssl = true;
    ssl_config.verify_peer = true;
    ssl_config.verify_host = true;
    ssl_config.check_certificate_revocation = true;
    ssl_config.require_client_certificate = true;
    ssl_config.minimum_tls_version = 13; // TLS 1.3
    ssl_config.enable_ocsp_stapling = true;
    ssl_config.enable_certificate_transparency = true;

    // Verify all advanced options are set
    ASSERT_TRUE(ssl_config.enable_ssl);
    ASSERT_TRUE(ssl_config.verify_peer);
    ASSERT_TRUE(ssl_config.verify_host);
    ASSERT_TRUE(ssl_config.check_certificate_revocation);
    ASSERT_TRUE(ssl_config.require_client_certificate);
    ASSERT_EQ(ssl_config.minimum_tls_version, 13);
    ASSERT_TRUE(ssl_config.enable_ocsp_stapling);
    ASSERT_TRUE(ssl_config.enable_certificate_transparency);
}
