package com.hivellm.umicp.discovery;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Server information for discovery.
 * Contains metadata about the server and its capabilities.
 *
 * @author HiveLLM Team
 * @version 0.2.0
 * @since 0.2.0
 */
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ServerInfo {
    
    @JsonProperty("server")
    private final String server;
    
    @JsonProperty("version")
    private final String version;
    
    @JsonProperty("protocol")
    private final String protocol;
    
    @JsonProperty("features")
    private final List<String> features;
    
    @JsonProperty("operations_count")
    private final Integer operationsCount;
    
    @JsonProperty("mcp_compatible")
    private final Boolean mcpCompatible;
    
    @JsonProperty("metadata")
    private final Map<String, Object> metadata;
    
    /**
     * Constructs ServerInfo with all fields.
     *
     * @param server Server name/identifier
     * @param version Server version
     * @param protocol Protocol version
     * @param features List of supported features
     * @param operationsCount Number of available operations
     * @param mcpCompatible MCP protocol compatibility flag
     * @param metadata Additional server metadata
     */
    public ServerInfo(
            String server,
            String version,
            String protocol,
            List<String> features,
            Integer operationsCount,
            Boolean mcpCompatible,
            Map<String, Object> metadata
    ) {
        if (server == null || server.isEmpty()) {
            throw new IllegalArgumentException("Server name cannot be null or empty");
        }
        if (version == null || version.isEmpty()) {
            throw new IllegalArgumentException("Version cannot be null or empty");
        }
        if (protocol == null || protocol.isEmpty()) {
            throw new IllegalArgumentException("Protocol cannot be null or empty");
        }
        
        this.server = server;
        this.version = version;
        this.protocol = protocol;
        this.features = features;
        this.operationsCount = operationsCount;
        this.mcpCompatible = mcpCompatible;
        this.metadata = metadata;
    }
    
    /**
     * Constructs minimal ServerInfo.
     *
     * @param server Server name/identifier
     * @param version Server version
     * @param protocol Protocol version
     */
    public ServerInfo(String server, String version, String protocol) {
        this(server, version, protocol, null, null, null, null);
    }
    
    // Getters
    
    public String getServer() {
        return server;
    }
    
    public String getVersion() {
        return version;
    }
    
    public String getProtocol() {
        return protocol;
    }
    
    public List<String> getFeatures() {
        return features;
    }
    
    public Integer getOperationsCount() {
        return operationsCount;
    }
    
    public Boolean getMcpCompatible() {
        return mcpCompatible;
    }
    
    public Map<String, Object> getMetadata() {
        return metadata;
    }
    
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ServerInfo that = (ServerInfo) o;
        return Objects.equals(server, that.server) &&
                Objects.equals(version, that.version) &&
                Objects.equals(protocol, that.protocol) &&
                Objects.equals(features, that.features) &&
                Objects.equals(operationsCount, that.operationsCount) &&
                Objects.equals(mcpCompatible, that.mcpCompatible) &&
                Objects.equals(metadata, that.metadata);
    }
    
    @Override
    public int hashCode() {
        return Objects.hash(server, version, protocol, features, operationsCount, mcpCompatible, metadata);
    }
    
    @Override
    public String toString() {
        return "ServerInfo{" +
                "server='" + server + '\'' +
                ", version='" + version + '\'' +
                ", protocol='" + protocol + '\'' +
                ", operationsCount=" + operationsCount +
                '}';
    }
    
    /**
     * Builder for ServerInfo.
     */
    public static class Builder {
        private final String server;
        private final String version;
        private final String protocol;
        private List<String> features;
        private Integer operationsCount;
        private Boolean mcpCompatible;
        private Map<String, Object> metadata;
        
        /**
         * Creates a new builder.
         *
         * @param server Server name
         * @param version Server version
         * @param protocol Protocol version
         */
        public Builder(String server, String version, String protocol) {
            this.server = server;
            this.version = version;
            this.protocol = protocol;
        }
        
        public Builder withFeatures(List<String> features) {
            this.features = features;
            return this;
        }
        
        public Builder withOperationsCount(Integer operationsCount) {
            this.operationsCount = operationsCount;
            return this;
        }
        
        public Builder withMcpCompatible(Boolean mcpCompatible) {
            this.mcpCompatible = mcpCompatible;
            return this;
        }
        
        public Builder withMetadata(Map<String, Object> metadata) {
            this.metadata = metadata;
            return this;
        }
        
        public ServerInfo build() {
            return new ServerInfo(server, version, protocol, features, operationsCount, mcpCompatible, metadata);
        }
    }
}
