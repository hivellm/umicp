package umicp

// UMICP v0.2.0 Tool Discovery
// MCP-compatible automatic tool introspection

// JSONSchema represents a JSON Schema for operation parameters
type JSONSchema map[string]interface{}

// OperationSchema defines the schema for an operation compatible with MCP
type OperationSchema struct {
	// Name of the operation
	Name string `json:"name"`

	// InputSchema is the JSON Schema for input parameters
	InputSchema JSONSchema `json:"input_schema"`

	// Title is a human-readable operation title
	Title string `json:"title,omitempty"`

	// Description explains what the operation does
	Description string `json:"description,omitempty"`

	// OutputSchema is the JSON Schema for the output/response
	OutputSchema JSONSchema `json:"output_schema,omitempty"`

	// Annotations contains additional metadata
	Annotations map[string]interface{} `json:"annotations,omitempty"`
}

// ServerInfo contains server information for discovery
type ServerInfo struct {
	// Server name/identifier
	Server string `json:"server"`

	// Version of the server
	Version string `json:"version"`

	// Protocol version
	Protocol string `json:"protocol"`

	// Features is a list of supported features
	Features []string `json:"features,omitempty"`

	// OperationsCount is the number of available operations
	OperationsCount int `json:"operations_count,omitempty"`

	// MCPCompatible indicates MCP protocol compatibility
	MCPCompatible bool `json:"mcp_compatible,omitempty"`

	// Metadata contains additional server metadata
	Metadata map[string]interface{} `json:"metadata,omitempty"`
}

// DiscoverableService is the interface for services that support tool discovery
type DiscoverableService interface {
	// ListOperations returns all available operations with their schemas
	ListOperations() []OperationSchema

	// GetSchema returns the schema for a specific operation by name
	GetSchema(name string) *OperationSchema

	// GetServerInfo returns server information and metadata
	GetServerInfo() ServerInfo
}

// OperationSchemaBuilder helps build OperationSchema instances
type OperationSchemaBuilder struct {
	schema OperationSchema
}

// NewOperationSchema creates a new OperationSchemaBuilder
func NewOperationSchema(name string, inputSchema JSONSchema) *OperationSchemaBuilder {
	return &OperationSchemaBuilder{
		schema: OperationSchema{
			Name:        name,
			InputSchema: inputSchema,
		},
	}
}

// WithTitle sets the operation title
func (b *OperationSchemaBuilder) WithTitle(title string) *OperationSchemaBuilder {
	b.schema.Title = title
	return b
}

// WithDescription sets the operation description
func (b *OperationSchemaBuilder) WithDescription(description string) *OperationSchemaBuilder {
	b.schema.Description = description
	return b
}

// WithOutputSchema sets the output schema
func (b *OperationSchemaBuilder) WithOutputSchema(outputSchema JSONSchema) *OperationSchemaBuilder {
	b.schema.OutputSchema = outputSchema
	return b
}

// WithAnnotations sets the annotations
func (b *OperationSchemaBuilder) WithAnnotations(annotations map[string]interface{}) *OperationSchemaBuilder {
	b.schema.Annotations = annotations
	return b
}

// Build returns the built OperationSchema
func (b *OperationSchemaBuilder) Build() OperationSchema {
	return b.schema
}

// ServerInfoBuilder helps build ServerInfo instances
type ServerInfoBuilder struct {
	info ServerInfo
}

// NewServerInfo creates a new ServerInfoBuilder
func NewServerInfo(server, version, protocol string) *ServerInfoBuilder {
	return &ServerInfoBuilder{
		info: ServerInfo{
			Server:   server,
			Version:  version,
			Protocol: protocol,
		},
	}
}

// WithFeatures sets the server features
func (b *ServerInfoBuilder) WithFeatures(features []string) *ServerInfoBuilder {
	b.info.Features = features
	return b
}

// WithOperationsCount sets the operations count
func (b *ServerInfoBuilder) WithOperationsCount(count int) *ServerInfoBuilder {
	b.info.OperationsCount = count
	return b
}

// WithMCPCompatible sets the MCP compatibility flag
func (b *ServerInfoBuilder) WithMCPCompatible(compatible bool) *ServerInfoBuilder {
	b.info.MCPCompatible = compatible
	return b
}

// WithMetadata sets the server metadata
func (b *ServerInfoBuilder) WithMetadata(metadata map[string]interface{}) *ServerInfoBuilder {
	b.info.Metadata = metadata
	return b
}

// Build returns the built ServerInfo
func (b *ServerInfoBuilder) Build() ServerInfo {
	return b.info
}

// DiscoveryHelpers provides helper functions for generating discovery responses
type DiscoveryHelpers struct{}

// GenerateOperationsResponse generates a JSON response for _list_operations
func (DiscoveryHelpers) GenerateOperationsResponse(service DiscoverableService) map[string]interface{} {
	operations := service.ListOperations()
	info := service.GetServerInfo()

	return map[string]interface{}{
		"operations":     operations,
		"count":          len(operations),
		"protocol":       info.Protocol,
		"mcp_compatible": info.MCPCompatible,
	}
}

// GenerateSchemaResponse generates a JSON response for _get_schema
func (DiscoveryHelpers) GenerateSchemaResponse(service DiscoverableService, operationName string) map[string]interface{} {
	schema := service.GetSchema(operationName)

	if schema != nil {
		response := map[string]interface{}{
			"name":         schema.Name,
			"input_schema": schema.InputSchema,
		}

		if schema.Title != "" {
			response["title"] = schema.Title
		}
		if schema.Description != "" {
			response["description"] = schema.Description
		}
		if schema.OutputSchema != nil {
			response["output_schema"] = schema.OutputSchema
		}
		if schema.Annotations != nil {
			response["annotations"] = schema.Annotations
		}

		return response
	}

	return map[string]interface{}{
		"error":     "Operation not found",
		"operation": operationName,
	}
}

// GenerateServerInfoResponse generates a JSON response for _server_info
func (DiscoveryHelpers) GenerateServerInfoResponse(service DiscoverableService) ServerInfo {
	return service.GetServerInfo()
}

// SimpleDiscoverableService is a simple implementation of DiscoverableService
type SimpleDiscoverableService struct {
	operations []OperationSchema
	serverInfo ServerInfo
}

// NewSimpleDiscoverableService creates a new SimpleDiscoverableService
func NewSimpleDiscoverableService(operations []OperationSchema, serverInfo ServerInfo) *SimpleDiscoverableService {
	return &SimpleDiscoverableService{
		operations: operations,
		serverInfo: serverInfo,
	}
}

// ListOperations returns all operations
func (s *SimpleDiscoverableService) ListOperations() []OperationSchema {
	return s.operations
}

// GetSchema returns the schema for a specific operation
func (s *SimpleDiscoverableService) GetSchema(name string) *OperationSchema {
	for i := range s.operations {
		if s.operations[i].Name == name {
			return &s.operations[i]
		}
	}
	return nil
}

// GetServerInfo returns server information with operations count
func (s *SimpleDiscoverableService) GetServerInfo() ServerInfo {
	info := s.serverInfo
	info.OperationsCount = len(s.operations)
	return info
}
