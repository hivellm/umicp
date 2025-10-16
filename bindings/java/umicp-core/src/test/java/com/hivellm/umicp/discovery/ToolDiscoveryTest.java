package com.hivellm.umicp.discovery;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.*;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Tests for Tool Discovery functionality.
 */
class ToolDiscoveryTest {

    private List<OperationSchema> operations;
    private ServerInfo serverInfo;
    private SimpleDiscoverableService service;

    @BeforeEach
    void setUp() {
        Map<String, Object> addInputSchema = new HashMap<>();
        addInputSchema.put("type", "object");
        Map<String, Object> properties = new HashMap<>();
        properties.put("a", Map.of("type", "number"));
        properties.put("b", Map.of("type", "number"));
        addInputSchema.put("properties", properties);

        OperationSchema addOp = new OperationSchema.Builder("add", addInputSchema)
                .withTitle("Add Numbers")
                .withDescription("Adds two numbers together")
                .build();

        Map<String, Object> multiplyInputSchema = new HashMap<>();
        multiplyInputSchema.put("type", "object");

        OperationSchema multiplyOp = new OperationSchema("multiply", multiplyInputSchema);

        operations = Arrays.asList(addOp, multiplyOp);

        serverInfo = new ServerInfo.Builder("math-service", "1.0.0", "UMICP/1.0")
                .withMcpCompatible(true)
                .build();

        service = new SimpleDiscoverableService(operations, serverInfo);
    }

    @Test
    void testOperationSchemaBasic() {
        Map<String, Object> inputSchema = new HashMap<>();
        inputSchema.put("type", "string");

        OperationSchema schema = new OperationSchema("test_op", inputSchema);

        assertEquals("test_op", schema.getName());
        assertNotNull(schema.getInputSchema());
        assertNull(schema.getTitle());
        assertNull(schema.getDescription());
    }

    @Test
    void testOperationSchemaBuilder() {
        Map<String, Object> inputSchema = Map.of("type", "object");
        Map<String, Object> outputSchema = Map.of("type", "boolean");
        Map<String, Object> annotations = Map.of("version", "1.0", "deprecated", false);

        OperationSchema schema = new OperationSchema.Builder("complex_op", inputSchema)
                .withTitle("Complex Operation")
                .withDescription("A complex test operation")
                .withOutputSchema(outputSchema)
                .withAnnotations(annotations)
                .build();

        assertEquals("complex_op", schema.getName());
        assertEquals("Complex Operation", schema.getTitle());
        assertEquals("A complex test operation", schema.getDescription());
        assertNotNull(schema.getOutputSchema());
        assertNotNull(schema.getAnnotations());
    }

    @Test
    void testOperationSchemaInvalidName() {
        Map<String, Object> inputSchema = Map.of("type", "object");

        assertThrows(IllegalArgumentException.class, () ->
            new OperationSchema("", inputSchema)
        );

        assertThrows(IllegalArgumentException.class, () ->
            new OperationSchema(null, inputSchema)
        );
    }

    @Test
    void testServerInfoBasic() {
        ServerInfo info = new ServerInfo("test-server", "1.0.0", "UMICP/1.0");

        assertEquals("test-server", info.getServer());
        assertEquals("1.0.0", info.getVersion());
        assertEquals("UMICP/1.0", info.getProtocol());
        assertNull(info.getFeatures());
    }

    @Test
    void testServerInfoBuilder() {
        ServerInfo info = new ServerInfo.Builder("full-server", "2.0.0", "UMICP/2.0")
                .withFeatures(Arrays.asList("discovery", "streaming", "compression"))
                .withOperationsCount(42)
                .withMcpCompatible(true)
                .withMetadata(Map.of("region", "us-west-2", "tier", "premium"))
                .build();

        assertEquals("full-server", info.getServer());
        assertEquals("2.0.0", info.getVersion());
        assertEquals("UMICP/2.0", info.getProtocol());
        assertEquals(3, info.getFeatures().size());
        assertEquals(42, info.getOperationsCount());
        assertTrue(info.getMcpCompatible());
        assertEquals("us-west-2", info.getMetadata().get("region"));
    }

    @Test
    void testServerInfoInvalidFields() {
        assertThrows(IllegalArgumentException.class, () ->
            new ServerInfo("", "1.0.0", "UMICP/1.0")
        );

        assertThrows(IllegalArgumentException.class, () ->
            new ServerInfo("server", "", "UMICP/1.0")
        );

        assertThrows(IllegalArgumentException.class, () ->
            new ServerInfo("server", "1.0.0", "")
        );
    }

    @Test
    void testSimpleDiscoverableServiceListOperations() {
        List<OperationSchema> ops = service.listOperations();

        assertEquals(2, ops.size());
        assertEquals("add", ops.get(0).getName());
        assertEquals("multiply", ops.get(1).getName());
    }

    @Test
    void testSimpleDiscoverableServiceGetSchema() {
        OperationSchema schema = service.getSchema("add");

        assertNotNull(schema);
        assertEquals("add", schema.getName());
        assertEquals("Add Numbers", schema.getTitle());
    }

    @Test
    void testSimpleDiscoverableServiceGetNonExistentSchema() {
        OperationSchema schema = service.getSchema("nonexistent");

        assertNull(schema);
    }

    @Test
    void testSimpleDiscoverableServiceGetServerInfo() {
        ServerInfo info = service.getServerInfo();

        assertEquals("math-service", info.getServer());
        assertEquals("1.0.0", info.getVersion());
        assertEquals(2, info.getOperationsCount());
        assertTrue(info.getMcpCompatible());
    }

    @Test
    void testSimpleDiscoverableServiceNullOperations() {
        assertThrows(IllegalArgumentException.class, () ->
            new SimpleDiscoverableService(null, serverInfo)
        );
    }

    @Test
    void testSimpleDiscoverableServiceNullServerInfo() {
        assertThrows(IllegalArgumentException.class, () ->
            new SimpleDiscoverableService(operations, null)
        );
    }

    @Test
    void testDiscoveryHelpersGenerateOperationsResponse() {
        Map<String, Object> response = DiscoveryHelpers.generateOperationsResponse(service);

        @SuppressWarnings("unchecked")
        List<OperationSchema> ops = (List<OperationSchema>) response.get("operations");
        assertEquals(2, ops.size());
        assertEquals(2, response.get("count"));
        assertEquals("UMICP/1.0", response.get("protocol"));
        assertEquals(true, response.get("mcp_compatible"));
    }

    @Test
    void testDiscoveryHelpersGenerateSchemaResponseSuccess() {
        Map<String, Object> response = DiscoveryHelpers.generateSchemaResponse(service, "add");

        assertEquals("add", response.get("name"));
        assertNotNull(response.get("input_schema"));
        assertEquals("Add Numbers", response.get("title"));
        assertNull(response.get("error"));
    }

    @Test
    void testDiscoveryHelpersGenerateSchemaResponseError() {
        Map<String, Object> response = DiscoveryHelpers.generateSchemaResponse(service, "missing");

        assertEquals("Operation not found", response.get("error"));
        assertEquals("missing", response.get("operation"));
    }

    @Test
    void testDiscoveryHelpersGenerateServerInfoResponse() {
        ServerInfo info = DiscoveryHelpers.generateServerInfoResponse(service);

        assertEquals("math-service", info.getServer());
        assertEquals("1.0.0", info.getVersion());
        assertEquals("UMICP/1.0", info.getProtocol());
        assertEquals(2, info.getOperationsCount());
    }

    @Test
    void testEmptyOperations() {
        SimpleDiscoverableService emptyService = new SimpleDiscoverableService(
                Collections.emptyList(),
                new ServerInfo("empty-server", "1.0.0", "UMICP/1.0")
        );

        assertEquals(0, emptyService.listOperations().size());
        assertEquals(0, emptyService.getServerInfo().getOperationsCount());
    }

    @Test
    void testOperationSchemaWithAllFields() {
        Map<String, Object> inputSchema = Map.of("type", "object");
        Map<String, Object> outputSchema = Map.of("type", "string");
        Map<String, Object> annotations = Map.of("version", "2.0");

        OperationSchema richSchema = new OperationSchema.Builder("rich_op", inputSchema)
                .withTitle("Rich Operation")
                .withDescription("A fully documented operation")
                .withOutputSchema(outputSchema)
                .withAnnotations(annotations)
                .build();

        SimpleDiscoverableService richService = new SimpleDiscoverableService(
                Collections.singletonList(richSchema),
                serverInfo
        );

        Map<String, Object> response = DiscoveryHelpers.generateSchemaResponse(richService, "rich_op");

        assertEquals("Rich Operation", response.get("title"));
        assertEquals("A fully documented operation", response.get("description"));
        assertNotNull(response.get("output_schema"));
        assertNotNull(response.get("annotations"));
    }
}
