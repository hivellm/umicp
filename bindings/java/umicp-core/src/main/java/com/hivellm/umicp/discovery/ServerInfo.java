package com.hivellm.umicp.discovery;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Server information for discovery
 */
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ServerInfo {

    @JsonProperty("server")
    @NotNull
    private final String server;

    @JsonProperty("version")
    @NotNull
    private final String version;

    @JsonProperty("protocol")
    @NotNull
    private final String protocol;

    @JsonProperty("features")
    @Nullable
    private final List<String> features;

    @JsonProperty("operations_count")
    @Nullable
    private final Integer operationsCount;

    @JsonProperty("mcp_compatible")
    @Nullable
    private final Boolean mcpCompatible;

    @JsonProperty("metadata")
    @Nullable
    private final Map<String, Object> metadata;

    public ServerInfo(
            @NotNull String server,
            @NotNull String version,
            @NotNull String protocol,
            @Nullable List<String> features,
            @Nullable Integer operationsCount,
            @Nullable Boolean mcpCompatible,
            @Nullable Map<String, Object> metadata
    ) {
        this.server = Objects.requireNonNull(server, "server cannot be null");
        this.version = Objects.requireNonNull(version, "version cannot be null");
        this.protocol = Objects.requireNonNull(protocol, "protocol cannot be null");
        this.features = features;
        this.operationsCount = operationsCount;
        this.mcpCompatible = mcpCompatible;
        this.metadata = metadata;
    }

    @NotNull
    public String getServer() {
        return server;
    }

    @NotNull
    public String getVersion() {
        return version;
    }

    @NotNull
    public String getProtocol() {
        return protocol;
    }

    @Nullable
    public List<String> getFeatures() {
        return features;
    }

    @Nullable
    public Integer getOperationsCount() {
        return operationsCount;
    }

    @Nullable
    public Boolean getMcpCompatible() {
        return mcpCompatible;
    }

    @Nullable
    public Map<String, Object> getMetadata() {
        return metadata;
    }

    /**
     * Builder for ServerInfo
     */
    public static class Builder {
        private final String server;
        private final String version;
        private final String protocol;
        private List<String> features;
        private Integer operationsCount;
        private Boolean mcpCompatible;
        private Map<String, Object> metadata;

        public Builder(@NotNull String server, @NotNull String version, @NotNull String protocol) {
            this.server = server;
            this.version = version;
            this.protocol = protocol;
        }

        public Builder withFeatures(List<String> features) {
            this.features = features;
            return this;
        }

        public Builder withOperationsCount(Integer count) {
            this.operationsCount = count;
            return this;
        }

        public Builder withMcpCompatible(Boolean compatible) {
            this.mcpCompatible = compatible;
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

    public static Builder builder(@NotNull String server, @NotNull String version, @NotNull String protocol) {
        return new Builder(server, version, protocol);
    }
}

