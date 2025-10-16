package com.hivellm.umicp.discovery;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.Map;
import java.util.Objects;

/**
 * Operation schema compatible with MCP JSON Schema
 */
@JsonInclude(JsonInclude.Include.NON_NULL)
public class OperationSchema {

    @JsonProperty("name")
    @NotNull
    private final String name;

    @JsonProperty("title")
    @Nullable
    private final String title;

    @JsonProperty("description")
    @Nullable
    private final String description;

    @JsonProperty("input_schema")
    @NotNull
    private final Map<String, Object> inputSchema;

    @JsonProperty("output_schema")
    @Nullable
    private final Map<String, Object> outputSchema;

    @JsonProperty("annotations")
    @Nullable
    private final Map<String, Object> annotations;

    public OperationSchema(
            @NotNull String name,
            @NotNull Map<String, Object> inputSchema,
            @Nullable String title,
            @Nullable String description,
            @Nullable Map<String, Object> outputSchema,
            @Nullable Map<String, Object> annotations
    ) {
        this.name = Objects.requireNonNull(name, "name cannot be null");
        this.inputSchema = Objects.requireNonNull(inputSchema, "inputSchema cannot be null");
        this.title = title;
        this.description = description;
        this.outputSchema = outputSchema;
        this.annotations = annotations;
    }

    @NotNull
    public String getName() {
        return name;
    }

    @Nullable
    public String getTitle() {
        return title;
    }

    @Nullable
    public String getDescription() {
        return description;
    }

    @NotNull
    public Map<String, Object> getInputSchema() {
        return inputSchema;
    }

    @Nullable
    public Map<String, Object> getOutputSchema() {
        return outputSchema;
    }

    @Nullable
    public Map<String, Object> getAnnotations() {
        return annotations;
    }

    /**
     * Builder for OperationSchema
     */
    public static class Builder {
        private final String name;
        private final Map<String, Object> inputSchema;
        private String title;
        private String description;
        private Map<String, Object> outputSchema;
        private Map<String, Object> annotations;

        public Builder(@NotNull String name, @NotNull Map<String, Object> inputSchema) {
            this.name = name;
            this.inputSchema = inputSchema;
        }

        public Builder withTitle(String title) {
            this.title = title;
            return this;
        }

        public Builder withDescription(String description) {
            this.description = description;
            return this;
        }

        public Builder withOutputSchema(Map<String, Object> schema) {
            this.outputSchema = schema;
            return this;
        }

        public Builder withAnnotations(Map<String, Object> annotations) {
            this.annotations = annotations;
            return this;
        }

        public OperationSchema build() {
            return new OperationSchema(name, inputSchema, title, description, outputSchema, annotations);
        }
    }

    public static Builder builder(@NotNull String name, @NotNull Map<String, Object> inputSchema) {
        return new Builder(name, inputSchema);
    }
}

