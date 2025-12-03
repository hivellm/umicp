package discovery

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

// TestService implements DiscoverableService for testing
type TestService struct{}

func (t *TestService) ListOperations() []OperationSchema {
	return []OperationSchema{
		{
			Name:        "search_vectors",
			Title:       "Search Vectors",
			Description: "Search for semantically similar content",
			InputSchema: JSONSchema{
				"type": "object",
				"properties": map[string]interface{}{
					"collection": map[string]interface{}{"type": "string"},
					"query":      map[string]interface{}{"type": "string"},
					"limit":      map[string]interface{}{"type": "integer", "default": 10},
				},
				"required": []string{"collection", "query"},
			},
			Annotations: map[string]interface{}{"read_only": true},
		},
		{
			Name:  "create_collection",
			Title: "Create Collection",
			InputSchema: JSONSchema{
				"type": "object",
				"properties": map[string]interface{}{
					"name":      map[string]interface{}{"type": "string"},
					"dimension": map[string]interface{}{"type": "integer"},
				},
				"required": []string{"name", "dimension"},
			},
		},
	}
}

func (t *TestService) GetSchema(name string) *OperationSchema {
	for _, op := range t.ListOperations() {
		if op.Name == name {
			return &op
		}
	}
	return nil
}

func (t *TestService) GetServerInfo() ServerInfo {
	count := 2
	compatible := true
	return ServerInfo{
		Server:          "test-service",
		Version:         "1.0.0",
		Protocol:        "UMICP/0.2",
		Features:        []string{"discovery", "search"},
		OperationsCount: &count,
		McpCompatible:   &compatible,
	}
}

func TestOperationSchemaBuilder(t *testing.T) {
	schema := NewOperationSchema("test_op", JSONSchema{"type": "object"}).
		WithTitle("Test Operation").
		WithDescription("A test").
		WithAnnotations(map[string]interface{}{"read_only": true}).
		Build()

	assert.Equal(t, "test_op", schema.Name)
	assert.Equal(t, "Test Operation", schema.Title)
	assert.Equal(t, "A test", schema.Description)
	assert.NotNil(t, schema.Annotations)
	assert.Equal(t, true, schema.Annotations["read_only"])
}

func TestServerInfoBuilder(t *testing.T) {
	info := NewServerInfo("service", "1.0", "UMICP/0.2").
		WithFeatures([]string{"discovery"}).
		WithOperationsCount(5).
		WithMcpCompatible(true).
		Build()

	assert.Equal(t, "service", info.Server)
	assert.Equal(t, "1.0", info.Version)
	assert.Equal(t, "UMICP/0.2", info.Protocol)
	assert.NotNil(t, info.Features)
	assert.Contains(t, info.Features, "discovery")
	assert.NotNil(t, info.OperationsCount)
	assert.Equal(t, 5, *info.OperationsCount)
	assert.NotNil(t, info.McpCompatible)
	assert.True(t, *info.McpCompatible)
}

func TestDiscoverableServiceListOperations(t *testing.T) {
	service := &TestService{}
	operations := service.ListOperations()

	assert.Equal(t, 2, len(operations))
	assert.Equal(t, "search_vectors", operations[0].Name)
	assert.Equal(t, "create_collection", operations[1].Name)
}

func TestDiscoverableServiceGetSchema(t *testing.T) {
	service := &TestService{}

	schema := service.GetSchema("search_vectors")
	assert.NotNil(t, schema)
	assert.Equal(t, "search_vectors", schema.Name)
	assert.Equal(t, "Search Vectors", schema.Title)
}

func TestDiscoverableServiceGetSchemaNotFound(t *testing.T) {
	service := &TestService{}

	schema := service.GetSchema("non_existent")
	assert.Nil(t, schema)
}

func TestDiscoverableServiceGetServerInfo(t *testing.T) {
	service := &TestService{}
	info := service.GetServerInfo()

	assert.Equal(t, "test-service", info.Server)
	assert.Equal(t, "1.0.0", info.Version)
	assert.Equal(t, "UMICP/0.2", info.Protocol)
	assert.Contains(t, info.Features, "discovery")
}

func TestGenerateOperationsResponse(t *testing.T) {
	service := &TestService{}
	response := GenerateOperationsResponse(service)

	assert.NotNil(t, response["operations"])
	assert.Equal(t, 2, response["count"])
	assert.Equal(t, "UMICP/0.2", response["protocol"])
	assert.Equal(t, true, response["mcp_compatible"])
}

func TestGenerateSchemaResponseFound(t *testing.T) {
	service := &TestService{}
	response := GenerateSchemaResponse(service, "search_vectors")

	assert.Equal(t, "search_vectors", response["name"])
	assert.Equal(t, "Search Vectors", response["title"])
	assert.Nil(t, response["error"])
}

func TestGenerateSchemaResponseNotFound(t *testing.T) {
	service := &TestService{}
	response := GenerateSchemaResponse(service, "invalid")

	assert.Equal(t, "Operation not found", response["error"])
	assert.Equal(t, "invalid", response["operation"])
}

func TestGenerateServerInfoResponse(t *testing.T) {
	service := &TestService{}
	info := GenerateServerInfoResponse(service)

	assert.Equal(t, "test-service", info.Server)
	assert.Equal(t, "1.0.0", info.Version)
	assert.True(t, *info.McpCompatible)
}

func TestCreateDiscoverableService(t *testing.T) {
	operations := []OperationSchema{
		{
			Name:        "test_op",
			InputSchema: JSONSchema{"type": "object"},
		},
	}

	info := ServerInfo{
		Server:   "test",
		Version:  "1.0",
		Protocol: "UMICP/0.2",
	}

	service := CreateDiscoverableService(operations, info)

	assert.Equal(t, 1, len(service.ListOperations()))
	assert.NotNil(t, service.GetSchema("test_op"))
	assert.Equal(t, 1, *service.GetServerInfo().OperationsCount)
}

func TestNativeTypesInCapabilities(t *testing.T) {
	// Test that we can use native types in capabilities
	capabilities := map[string]interface{}{
		"max_tokens":  100,
		"temperature": 0.7,
		"enabled":     true,
		"models":      []string{"gpt-4", "claude-3"},
		"config": map[string]interface{}{
			"timeout": 30,
		},
		"optional": nil,
	}

	// Verify type assertions work
	assert.Equal(t, 100, capabilities["max_tokens"])
	assert.Equal(t, 0.7, capabilities["temperature"])
	assert.Equal(t, true, capabilities["enabled"])
	assert.IsType(t, []string{}, capabilities["models"])
	assert.IsType(t, map[string]interface{}{}, capabilities["config"])
	assert.Nil(t, capabilities["optional"])
}
