/**
 * UMICP Native Types Test
 * Tests for v1.1.0 native JSON type support in capabilities
 */

#include "../include/umicp_types.h"
#include "../include/envelope.h"
#include <nlohmann/json.hpp>
#include <cassert>
#include <iostream>

using namespace umicp;
using json = nlohmann::json;

void test_native_int() {
    std::cout << "Test: Native integer in capabilities... ";

    auto envelope = EnvelopeBuilder()
        .from("test-client")
        .to("test-server")
        .operation(OperationType::DATA)
        .capability_int("count", 42)
        .build();

    assert(envelope.is_success());
    auto env = envelope.value.value();

    // Verify native type preserved
    assert(env.capabilities.has_value());
    auto caps = env.get_capabilities();
    assert(caps["count"].is_number_integer());
    assert(caps["count"].get<int>() == 42);

    std::cout << "✅ PASSED\n";
}

void test_native_bool() {
    std::cout << "Test: Native boolean in capabilities... ";

    auto envelope = EnvelopeBuilder()
        .from("test-client")
        .to("test-server")
        .capability_bool("enabled", true)
        .capability_bool("disabled", false)
        .build();

    assert(envelope.is_success());
    auto caps = envelope.value->get_capabilities();

    assert(caps["enabled"].is_boolean());
    assert(caps["enabled"].get<bool>() == true);
    assert(caps["disabled"].get<bool>() == false);

    std::cout << "✅ PASSED\n";
}

void test_native_double() {
    std::cout << "Test: Native double in capabilities... ";

    auto envelope = EnvelopeBuilder()
        .from("test-client")
        .to("test-server")
        .capability_double("threshold", 0.75)
        .build();

    assert(envelope.is_success());
    auto caps = envelope.value->get_capabilities();

    assert(caps["threshold"].is_number_float());
    assert(std::abs(caps["threshold"].get<double>() - 0.75) < 0.001);

    std::cout << "✅ PASSED\n";
}

void test_native_array() {
    std::cout << "Test: Native array in capabilities... ";

    auto envelope = EnvelopeBuilder()
        .from("test-client")
        .to("test-server")
        .capability("tags", json::array({"rust", "cpp", "umicp"}))
        .build();

    assert(envelope.is_success());
    auto caps = envelope.value->get_capabilities();

    assert(caps["tags"].is_array());
    auto tags = caps["tags"].get<std::vector<std::string>>();
    assert(tags.size() == 3);
    assert(tags[0] == "rust");
    assert(tags[1] == "cpp");
    assert(tags[2] == "umicp");

    std::cout << "✅ PASSED\n";
}

void test_native_object() {
    std::cout << "Test: Native object in capabilities... ";

    json metadata = {
        {"author", "HiveLLM"},
        {"version", "1.1.0"},
        {"count", 42}
    };

    auto envelope = EnvelopeBuilder()
        .from("test-client")
        .to("test-server")
        .capability("metadata", metadata)
        .build();

    assert(envelope.is_success());
    auto caps = envelope.value->get_capabilities();

    assert(caps["metadata"].is_object());
    auto meta = caps["metadata"];
    assert(meta["author"].get<std::string>() == "HiveLLM");
    assert(meta["version"].get<std::string>() == "1.1.0");
    assert(meta["count"].get<int>() == 42);

    std::cout << "✅ PASSED\n";
}

void test_mixed_types() {
    std::cout << "Test: Mixed types in capabilities... ";

    auto envelope = EnvelopeBuilder()
        .from("test-client")
        .to("test-server")
        .capability_str("message", "hello")
        .capability_int("count", 10)
        .capability_bool("enabled", true)
        .capability_double("score", 95.5)
        .capability("tags", json::array({"a", "b"}))
        .build();

    assert(envelope.is_success());
    auto caps = envelope.value->get_capabilities();

    assert(caps["message"].get<std::string>() == "hello");
    assert(caps["count"].get<int>() == 10);
    assert(caps["enabled"].get<bool>() == true);
    assert(std::abs(caps["score"].get<double>() - 95.5) < 0.001);
    assert(caps["tags"].is_array());

    std::cout << "✅ PASSED\n";
}

void test_serialization_deserialization() {
    std::cout << "Test: Serialization/Deserialization with native types... ";

    auto original = EnvelopeBuilder()
        .from("client")
        .to("server")
        .operation(OperationType::DATA)
        .capability_int("count", 42)
        .capability_bool("enabled", true)
        .capability("tags", json::array({"test"}))
        .build();

    assert(original.is_success());

    // Serialize
    auto serialized = EnvelopeProcessor::serialize(*original.value);
    assert(serialized.is_success());

    // Deserialize
    auto deserialized = EnvelopeProcessor::deserialize(*serialized.value);
    assert(deserialized.is_success());

    // Verify types preserved
    auto caps = deserialized.value->get_capabilities();
    assert(caps["count"].is_number_integer());
    assert(caps["count"].get<int>() == 42);
    assert(caps["enabled"].is_boolean());
    assert(caps["enabled"].get<bool>() == true);
    assert(caps["tags"].is_array());

    std::cout << "✅ PASSED\n";
}

void test_backward_compatibility_strings() {
    std::cout << "Test: Backward compatibility with string values... ";

    // Old-style JSON with string values
    std::string old_json = R"({
        "v": "1.0",
        "msg_id": "test-123",
        "ts": "2025-10-16T12:00:00.000Z",
        "from": "client",
        "to": "server",
        "op": "DATA",
        "capabilities": {
            "count": "42",
            "enabled": "true"
        }
    })";

    auto envelope = EnvelopeProcessor::deserialize(old_json);
    assert(envelope.is_success());

    auto caps = envelope.value->get_capabilities();
    // Values come in as strings (backward compatible)
    assert(caps["count"].is_string());
    assert(caps["count"].get<std::string>() == "42");

    std::cout << "✅ PASSED\n";
}

int main() {
    std::cout << "\n=== UMICP C++ Core v1.1.0 - Native Types Tests ===\n\n";

    try {
        test_native_int();
        test_native_bool();
        test_native_double();
        test_native_array();
        test_native_object();
        test_mixed_types();
        test_serialization_deserialization();
        test_backward_compatibility_strings();

        std::cout << "\n✅ All tests PASSED! (8/8)\n";
        std::cout << "C++ Core v1.1.0 is 100% functional with native types.\n\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}

