package umicp

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestOperationSchemaBuilder(t *testing.T) {
	t.Run("Basic Operation Schema", func(t *testing.T) {
		schema := NewOperationSchema("test_op", JSONSchema{
			"type": "object",
			"properties": map[string]interface{}{
				"param1": map[string]string{"type": "string"},
			},
		}).Build()

		assert.Equal(t, "test_op", schema.Name)
		assert.NotNil(t, schema.InputSchema)
		assert.Equal(t, "object", schema.InputSchema["type"])
	})

	t.Run("Complete Operation Schema", func(t *testing.T) {
		schema := NewOperationSchema("complex_op", JSONSchema{
			"type": "object",
		}).
			WithTitle("Complex Operation").
			WithDescription("A complex test operation").
			WithOutputSchema(JSONSchema{"type": "boolean"}).
			WithAnnotations(map[string]interface{}{
				"version":    "1.0",
				"deprecated": false,
			}).
			Build()

		assert.Equal(t, "complex_op", schema.Name)
		assert.Equal(t, "Complex Operation", schema.Title)
		assert.Equal(t, "A complex test operation", schema.Description)
		assert.NotNil(t, schema.OutputSchema)
		assert.NotNil(t, schema.Annotations)
		assert.Equal(t, "1.0", schema.Annotations["version"])
	})

	t.Run("Fluent Builder Interface", func(t *testing.T) {
		schema := NewOperationSchema("fluent", JSONSchema{}).
			WithTitle("Fluent").
			WithDescription("Test").
			Build()

		assert.Equal(t, "fluent", schema.Name)
		assert.Equal(t, "Fluent", schema.Title)
		assert.Equal(t, "Test", schema.Description)
	})
}

func TestServerInfoBuilder(t *testing.T) {
	t.Run("Basic Server Info", func(t *testing.T) {
		info := NewServerInfo("test-server", "1.0.0", "UMICP/1.0").Build()

		assert.Equal(t, "test-server", info.Server)
		assert.Equal(t, "1.0.0", info.Version)
		assert.Equal(t, "UMICP/1.0", info.Protocol)
	})

	t.Run("Complete Server Info", func(t *testing.T) {
		info := NewServerInfo("full-server", "2.0.0", "UMICP/2.0").
			WithFeatures([]string{"discovery", "streaming", "compression"}).
			WithOperationsCount(42).
			WithMCPCompatible(true).
			WithMetadata(map[string]interface{}{
				"region": "us-west-2",
				"tier":   "premium",
			}).
			Build()

		assert.Equal(t, "full-server", info.Server)
		assert.Equal(t, "2.0.0", info.Version)
		assert.Equal(t, "UMICP/2.0", info.Protocol)
		assert.Equal(t, []string{"discovery", "streaming", "compression"}, info.Features)
		assert.Equal(t, 42, info.OperationsCount)
		assert.True(t, info.MCPCompatible)
		assert.Equal(t, "us-west-2", info.Metadata["region"])
	})

	t.Run("Fluent Builder Interface", func(t *testing.T) {
		info := NewServerInfo("fluent", "1.0.0", "UMICP/1.0").
			WithFeatures([]string{"test"}).
			WithMCPCompatible(true).
			Build()

		assert.Equal(t, "fluent", info.Server)
		assert.True(t, info.MCPCompatible)
		assert.Len(t, info.Features, 1)
	})
}

func TestSimpleDiscoverableService(t *testing.T) {
	operations := []OperationSchema{
		{
			Name: "add",
			InputSchema: JSONSchema{
				"type": "object",
				"properties": map[string]interface{}{
					"a": map[string]string{"type": "number"},
					"b": map[string]string{"type": "number"},
				},
			},
			Title:       "Add Numbers",
			Description: "Adds two numbers together",
		},
		{
			Name: "multiply",
			InputSchema: JSONSchema{
				"type": "object",
			},
			Title: "Multiply Numbers",
		},
	}

	serverInfo := ServerInfo{
		Server:        "math-service",
		Version:       "1.0.0",
		Protocol:      "UMICP/1.0",
		MCPCompatible: true,
	}

	service := NewSimpleDiscoverableService(operations, serverInfo)

	t.Run("List All Operations", func(t *testing.T) {
		ops := service.ListOperations()
		require.Len(t, ops, 2)
		assert.Equal(t, "add", ops[0].Name)
		assert.Equal(t, "multiply", ops[1].Name)
	})

	t.Run("Get Schema By Name", func(t *testing.T) {
		schema := service.GetSchema("add")
		require.NotNil(t, schema)
		assert.Equal(t, "add", schema.Name)
		assert.Equal(t, "Add Numbers", schema.Title)
	})

	t.Run("Get Non-Existent Schema", func(t *testing.T) {
		schema := service.GetSchema("nonexistent")
		assert.Nil(t, schema)
	})

	t.Run("Get Server Info With Operations Count", func(t *testing.T) {
		info := service.GetServerInfo()
		assert.Equal(t, "math-service", info.Server)
		assert.Equal(t, "1.0.0", info.Version)
		assert.Equal(t, 2, info.OperationsCount)
		assert.True(t, info.MCPCompatible)
	})
}

func TestDiscoveryHelpers(t *testing.T) {
	operations := []OperationSchema{
		{
			Name:        "test_op",
			InputSchema: JSONSchema{"type": "object"},
			Title:       "Test Operation",
		},
	}

	serverInfo := ServerInfo{
		Server:        "test-server",
		Version:       "1.0.0",
		Protocol:      "UMICP/1.0",
		MCPCompatible: true,
	}

	service := NewSimpleDiscoverableService(operations, serverInfo)
	helpers := DiscoveryHelpers{}

	t.Run("Generate Operations Response", func(t *testing.T) {
		response := helpers.GenerateOperationsResponse(service)

		assert.Len(t, response["operations"], 1)
		assert.Equal(t, 1, response["count"])
		assert.Equal(t, "UMICP/1.0", response["protocol"])
		assert.True(t, response["mcp_compatible"].(bool))
	})

	t.Run("Generate Schema Response For Existing Operation", func(t *testing.T) {
		response := helpers.GenerateSchemaResponse(service, "test_op")

		assert.Equal(t, "test_op", response["name"])
		assert.NotNil(t, response["input_schema"])
		assert.Equal(t, "Test Operation", response["title"])
		assert.Nil(t, response["error"])
	})

	t.Run("Generate Error Response For Non-Existent Operation", func(t *testing.T) {
		response := helpers.GenerateSchemaResponse(service, "missing")

		assert.Equal(t, "Operation not found", response["error"])
		assert.Equal(t, "missing", response["operation"])
	})

	t.Run("Generate Server Info Response", func(t *testing.T) {
		response := helpers.GenerateServerInfoResponse(service)

		assert.Equal(t, "test-server", response.Server)
		assert.Equal(t, "1.0.0", response.Version)
		assert.Equal(t, "UMICP/1.0", response.Protocol)
		assert.Equal(t, 1, response.OperationsCount)
	})

	t.Run("Include Optional Fields In Schema Response", func(t *testing.T) {
		richSchema := OperationSchema{
			Name:         "rich_op",
			InputSchema:  JSONSchema{"type": "object"},
			Title:        "Rich Operation",
			Description:  "A fully documented operation",
			OutputSchema: JSONSchema{"type": "string"},
			Annotations: map[string]interface{}{
				"version": "2.0",
			},
		}

		richService := NewSimpleDiscoverableService([]OperationSchema{richSchema}, serverInfo)
		response := helpers.GenerateSchemaResponse(richService, "rich_op")

		assert.Equal(t, "Rich Operation", response["title"])
		assert.Equal(t, "A fully documented operation", response["description"])
		assert.NotNil(t, response["output_schema"])
		assert.NotNil(t, response["annotations"])
	})
}

func TestDiscoverableServiceInterface(t *testing.T) {
	type CustomDiscoverableService struct {
		ops []OperationSchema
	}

	customService := &CustomDiscoverableService{
		ops: []OperationSchema{
			{
				Name:        "custom_op",
				InputSchema: JSONSchema{"type": "string"},
			},
		},
	}

	// Implement DiscoverableService interface
	var _ DiscoverableService = &SimpleDiscoverableService{}

	// Test that custom implementations work
	listOps := func(s *CustomDiscoverableService) []OperationSchema {
		return s.ops
	}

	ops := listOps(customService)
	assert.Len(t, ops, 1)
	assert.Equal(t, "custom_op", ops[0].Name)
}

func TestEmptyOperations(t *testing.T) {
	service := NewSimpleDiscoverableService([]OperationSchema{}, ServerInfo{
		Server:   "empty-server",
		Version:  "1.0.0",
		Protocol: "UMICP/1.0",
	})

	t.Run("List Empty Operations", func(t *testing.T) {
		ops := service.ListOperations()
		assert.Len(t, ops, 0)
	})

	t.Run("Operations Count Is Zero", func(t *testing.T) {
		info := service.GetServerInfo()
		assert.Equal(t, 0, info.OperationsCount)
	})
}
