package discovery

// Tool Discovery for UMICP v0.2.0
// MCP-compatible automatic tool introspection

// JSONSchema represents a JSON Schema (map with any structure)
type JSONSchema map[string]interface{}

// OperationSchema represents an operation with its input/output schemas
type OperationSchema struct {
	Name         string                 `json:"name"`
	Title        string                 `json:"title,omitempty"`
	Description  string                 `json:"description,omitempty"`
	InputSchema  JSONSchema             `json:"input_schema"`
	OutputSchema *JSONSchema            `json:"output_schema,omitempty"`
	Annotations  map[string]interface{} `json:"annotations,omitempty"`
}

// ServerInfo contains server metadata for discovery
type ServerInfo struct {
	Server          string                 `json:"server"`
	Version         string                 `json:"version"`
	Protocol        string                 `json:"protocol"`
	Features        []string               `json:"features,omitempty"`
	OperationsCount *int                   `json:"operations_count,omitempty"`
	McpCompatible   *bool                  `json:"mcp_compatible,omitempty"`
	Metadata        map[string]interface{} `json:"metadata,omitempty"`
}

// DiscoverableService interface for services that support tool discovery
type DiscoverableService interface {
	// ListOperations returns all available operations with their schemas
	ListOperations() []OperationSchema

	// GetSchema returns schema for a specific operation by name
	GetSchema(name string) *OperationSchema

	// GetServerInfo returns server information and metadata
	GetServerInfo() ServerInfo
}

// OperationSchemaBuilder provides a builder pattern for OperationSchema
type OperationSchemaBuilder struct {
	schema OperationSchema
}

// NewOperationSchema creates a new operation schema builder
func NewOperationSchema(name string, inputSchema JSONSchema) *OperationSchemaBuilder {
	return &OperationSchemaBuilder{
		schema: OperationSchema{
			Name:        name,
			InputSchema: inputSchema,
		},
	}
}

// WithTitle sets the title
func (b *OperationSchemaBuilder) WithTitle(title string) *OperationSchemaBuilder {
	b.schema.Title = title
	return b
}

// WithDescription sets the description
func (b *OperationSchemaBuilder) WithDescription(desc string) *OperationSchemaBuilder {
	b.schema.Description = desc
	return b
}

// WithOutputSchema sets the output schema
func (b *OperationSchemaBuilder) WithOutputSchema(schema JSONSchema) *OperationSchemaBuilder {
	b.schema.OutputSchema = &schema
	return b
}

// WithAnnotations sets annotations
func (b *OperationSchemaBuilder) WithAnnotations(annotations map[string]interface{}) *OperationSchemaBuilder {
	b.schema.Annotations = annotations
	return b
}

// Build returns the built OperationSchema
func (b *OperationSchemaBuilder) Build() OperationSchema {
	return b.schema
}

// ServerInfoBuilder provides a builder pattern for ServerInfo
type ServerInfoBuilder struct {
	info ServerInfo
}

// NewServerInfo creates a new server info builder
func NewServerInfo(server, version, protocol string) *ServerInfoBuilder {
	return &ServerInfoBuilder{
		info: ServerInfo{
			Server:   server,
			Version:  version,
			Protocol: protocol,
		},
	}
}

// WithFeatures sets the features
func (b *ServerInfoBuilder) WithFeatures(features []string) *ServerInfoBuilder {
	b.info.Features = features
	return b
}

// WithOperationsCount sets the operations count
func (b *ServerInfoBuilder) WithOperationsCount(count int) *ServerInfoBuilder {
	b.info.OperationsCount = &count
	return b
}

// WithMcpCompatible sets MCP compatibility
func (b *ServerInfoBuilder) WithMcpCompatible(compatible bool) *ServerInfoBuilder {
	b.info.McpCompatible = &compatible
	return b
}

// WithMetadata sets metadata
func (b *ServerInfoBuilder) WithMetadata(metadata map[string]interface{}) *ServerInfoBuilder {
	b.info.Metadata = metadata
	return b
}

// Build returns the built ServerInfo
func (b *ServerInfoBuilder) Build() ServerInfo {
	return b.info
}

// GenerateOperationsResponse generates a response for _list_operations
func GenerateOperationsResponse(service DiscoverableService) map[string]interface{} {
	operations := service.ListOperations()
	info := service.GetServerInfo()

	mcpCompatible := false
	if info.McpCompatible != nil {
		mcpCompatible = *info.McpCompatible
	}

	return map[string]interface{}{
		"operations":     operations,
		"count":          len(operations),
		"protocol":       info.Protocol,
		"mcp_compatible": mcpCompatible,
	}
}

// GenerateSchemaResponse generates a response for _get_schema
func GenerateSchemaResponse(service DiscoverableService, operationName string) map[string]interface{} {
	schema := service.GetSchema(operationName)

	if schema != nil {
		return map[string]interface{}{
			"name":          schema.Name,
			"title":         schema.Title,
			"description":   schema.Description,
			"input_schema":  schema.InputSchema,
			"output_schema": schema.OutputSchema,
			"annotations":   schema.Annotations,
		}
	}

	return map[string]interface{}{
		"error":     "Operation not found",
		"operation": operationName,
	}
}

// GenerateServerInfoResponse generates a response for _server_info
func GenerateServerInfoResponse(service DiscoverableService) ServerInfo {
	return service.GetServerInfo()
}

// CreateDiscoverableService creates a simple discoverable service from operations and info
func CreateDiscoverableService(operations []OperationSchema, serverInfo ServerInfo) DiscoverableService {
	count := len(operations)
	serverInfo.OperationsCount = &count

	return &simpleDiscoverableService{
		operations: operations,
		serverInfo: serverInfo,
	}
}

// simpleDiscoverableService is a simple implementation of DiscoverableService
type simpleDiscoverableService struct {
	operations []OperationSchema
	serverInfo ServerInfo
}

func (s *simpleDiscoverableService) ListOperations() []OperationSchema {
	return s.operations
}

func (s *simpleDiscoverableService) GetSchema(name string) *OperationSchema {
	for i := range s.operations {
		if s.operations[i].Name == name {
			return &s.operations[i]
		}
	}
	return nil
}

func (s *simpleDiscoverableService) GetServerInfo() ServerInfo {
	return s.serverInfo
}
