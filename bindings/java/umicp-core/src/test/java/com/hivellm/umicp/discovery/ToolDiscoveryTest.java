package com.hivellm.umicp.discovery;

import org.junit.jupiter.api.Test;

import java.util.List;
import java.util.Map;

import static org.assertj.core.api.Assertions.assertThat;

class TestService implements DiscoverableService {
    @Override
    public List<OperationSchema> listOperations() {
        return List.of(
            OperationSchema.builder("search_vectors", Map.of(
                "type", "object",
                "properties", Map.of(
                    "collection", Map.of("type", "string"),
                    "query", Map.of("type", "string"),
                    "limit", Map.of("type", "integer", "default", 10)
                ),
                "required", List.of("collection", "query")
            ))
            .withTitle("Search Vectors")
            .withDescription("Search for semantically similar content")
            .withAnnotations(Map.of("read_only", true))
            .build(),

            OperationSchema.builder("create_collection", Map.of(
                "type", "object",
                "properties", Map.of(
                    "name", Map.of("type", "string"),
                    "dimension", Map.of("type", "integer")
                ),
                "required", List.of("name", "dimension")
            ))
            .withTitle("Create Collection")
            .build()
        );
    }

    @Override
    public ServerInfo getServerInfo() {
        return ServerInfo.builder("test-service", "1.0.0", "UMICP/0.2")
            .withFeatures(List.of("discovery", "search"))
            .withOperationsCount(2)
            .withMcpCompatible(true)
            .build();
    }
}

class ToolDiscoveryTest {

    @Test
    void testOperationSchemaCreation() {
        OperationSchema schema = OperationSchema.builder(
            "test_op",
            Map.of("type", "object")
        ).build();

        assertThat(schema.getName()).isEqualTo("test_op");
        assertThat(schema.getInputSchema()).containsEntry("type", "object");
    }

    @Test
    void testOperationSchemaBuilder() {
        OperationSchema schema = OperationSchema.builder("test", Map.of("type", "object"))
            .withTitle("Test Operation")
            .withDescription("A test")
            .withAnnotations(Map.of("read_only", true))
            .build();

        assertThat(schema.getTitle()).isEqualTo("Test Operation");
        assertThat(schema.getDescription()).isEqualTo("A test");
        assertThat(schema.getAnnotations()).containsEntry("read_only", true);
    }

    @Test
    void testServerInfoCreation() {
        ServerInfo info = ServerInfo.builder("my-service", "1.0.0", "UMICP/0.2").build();

        assertThat(info.getServer()).isEqualTo("my-service");
        assertThat(info.getVersion()).isEqualTo("1.0.0");
        assertThat(info.getProtocol()).isEqualTo("UMICP/0.2");
    }

    @Test
    void testServerInfoBuilder() {
        ServerInfo info = ServerInfo.builder("test", "1.0", "UMICP/0.2")
            .withFeatures(List.of("discovery"))
            .withOperationsCount(5)
            .withMcpCompatible(true)
            .withMetadata(Map.of("region", "us-east"))
            .build();

        assertThat(info.getFeatures()).contains("discovery");
        assertThat(info.getOperationsCount()).isEqualTo(5);
        assertThat(info.getMcpCompatible()).isTrue();
        assertThat(info.getMetadata()).containsEntry("region", "us-east");
    }

    @Test
    void testDiscoverableServiceListOperations() {
        TestService service = new TestService();
        List<OperationSchema> operations = service.listOperations();

        assertThat(operations).hasSize(2);
        assertThat(operations.get(0).getName()).isEqualTo("search_vectors");
        assertThat(operations.get(1).getName()).isEqualTo("create_collection");
    }

    @Test
    void testDiscoverableServiceGetSchema() {
        TestService service = new TestService();
        OperationSchema schema = service.getSchema("search_vectors");

        assertThat(schema).isNotNull();
        assertThat(schema.getName()).isEqualTo("search_vectors");
        assertThat(schema.getTitle()).isEqualTo("Search Vectors");
    }

    @Test
    void testDiscoverableServiceGetSchemaNotFound() {
        TestService service = new TestService();
        OperationSchema schema = service.getSchema("non_existent");

        assertThat(schema).isNull();
    }

    @Test
    void testDiscoverableServiceGetServerInfo() {
        TestService service = new TestService();
        ServerInfo info = service.getServerInfo();

        assertThat(info.getServer()).isEqualTo("test-service");
        assertThat(info.getVersion()).isEqualTo("1.0.0");
        assertThat(info.getProtocol()).isEqualTo("UMICP/0.2");
        assertThat(info.getFeatures()).contains("discovery");
    }

    @Test
    void testGenerateOperationsResponse() {
        TestService service = new TestService();
        Map<String, Object> response = DiscoveryHelpers.generateOperationsResponse(service);

        assertThat(response).containsKey("operations");
        assertThat(response).containsEntry("count", 2);
        assertThat(response).containsEntry("protocol", "UMICP/0.2");
        assertThat(response).containsEntry("mcp_compatible", true);
    }

    @Test
    void testGenerateSchemaResponseFound() {
        TestService service = new TestService();
        Map<String, Object> response = DiscoveryHelpers.generateSchemaResponse(service, "search_vectors");

        assertThat(response).containsEntry("name", "search_vectors");
        assertThat(response).containsEntry("title", "Search Vectors");
        assertThat(response).doesNotContainKey("error");
    }

    @Test
    void testGenerateSchemaResponseNotFound() {
        TestService service = new TestService();
        Map<String, Object> response = DiscoveryHelpers.generateSchemaResponse(service, "invalid");

        assertThat(response).containsEntry("error", "Operation not found");
        assertThat(response).containsEntry("operation", "invalid");
    }

    @Test
    void testGenerateServerInfoResponse() {
        TestService service = new TestService();
        Map<String, Object> response = DiscoveryHelpers.generateServerInfoResponse(service);

        assertThat(response).containsEntry("server", "test-service");
        assertThat(response).containsEntry("version", "1.0.0");
        assertThat(response).containsEntry("mcp_compatible", true);
    }

    @Test
    void testNativeTypesInCapabilities() {
        // Test that we can use native types in capabilities
        Map<String, Object> capabilities = Map.of(
            "max_tokens", 100,
            "temperature", 0.7,
            "enabled", true,
            "models", List.of("gpt-4", "claude-3"),
            "config", Map.of("timeout", 30)
        );

        assertThat(capabilities.get("max_tokens")).isInstanceOf(Integer.class);
        assertThat(capabilities.get("temperature")).isInstanceOf(Double.class);
        assertThat(capabilities.get("enabled")).isInstanceOf(Boolean.class);
        assertThat(capabilities.get("models")).isInstanceOf(List.class);
        assertThat(capabilities.get("config")).isInstanceOf(Map.class);
    }
}

