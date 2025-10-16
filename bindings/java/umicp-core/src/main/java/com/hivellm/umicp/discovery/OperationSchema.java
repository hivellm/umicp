package com.hivellm.umicp.discovery;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.Map;
import java.util.Objects;

/**
 * Operation schema compatible with MCP JSON Schema.
 * Defines the structure and metadata for an operation.
 *
 * @author HiveLLM Team
 * @version 0.2.0
 * @since 0.2.0
 */
@JsonInclude(JsonInclude.Include.NON_NULL)
public class OperationSchema {

    @JsonProperty("name")
    private final String name;

    @JsonProperty("input_schema")
    private final Map<String, Object> inputSchema;

    @JsonProperty("title")
    private final String title;

    @JsonProperty("description")
    private final String description;

    @JsonProperty("output_schema")
    private final Map<String, Object> outputSchema;

    @JsonProperty("annotations")
    private final Map<String, Object> annotations;

    /**
     * Constructs an OperationSchema with all fields.
     *
     * @param name Operation name
     * @param inputSchema JSON Schema for input parameters
     * @param title Human-readable operation title
     * @param description Operation description
     * @param outputSchema JSON Schema for output/response
     * @param annotations Additional metadata annotations
     */
    public OperationSchema(
            String name,
            Map<String, Object> inputSchema,
            String title,
            String description,
            Map<String, Object> outputSchema,
            Map<String, Object> annotations
    ) {
        if (name == null || name.isEmpty()) {
            throw new IllegalArgumentException("Operation name cannot be null or empty");
        }
        if (inputSchema == null) {
            throw new IllegalArgumentException("Input schema cannot be null");
        }

        this.name = name;
        this.inputSchema = inputSchema;
        this.title = title;
        this.description = description;
        this.outputSchema = outputSchema;
        this.annotations = annotations;
    }

    /**
     * Constructs a minimal OperationSchema.
     *
     * @param name Operation name
     * @param inputSchema JSON Schema for input parameters
     */
    public OperationSchema(String name, Map<String, Object> inputSchema) {
        this(name, inputSchema, null, null, null, null);
    }

    // Getters

    public String getName() {
        return name;
    }

    public Map<String, Object> getInputSchema() {
        return inputSchema;
    }

    public String getTitle() {
        return title;
    }

    public String getDescription() {
        return description;
    }

    public Map<String, Object> getOutputSchema() {
        return outputSchema;
    }

    public Map<String, Object> getAnnotations() {
        return annotations;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        OperationSchema that = (OperationSchema) o;
        return Objects.equals(name, that.name) &&
                Objects.equals(inputSchema, that.inputSchema) &&
                Objects.equals(title, that.title) &&
                Objects.equals(description, that.description) &&
                Objects.equals(outputSchema, that.outputSchema) &&
                Objects.equals(annotations, that.annotations);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name, inputSchema, title, description, outputSchema, annotations);
    }

    @Override
    public String toString() {
        return "OperationSchema{" +
                "name='" + name + '\'' +
                ", title='" + title + '\'' +
                ", description='" + description + '\'' +
                '}';
    }

    /**
     * Builder for OperationSchema.
     */
    public static class Builder {
        private final String name;
        private final Map<String, Object> inputSchema;
        private String title;
        private String description;
        private Map<String, Object> outputSchema;
        private Map<String, Object> annotations;

        /**
         * Creates a new builder.
         *
         * @param name Operation name
         * @param inputSchema JSON Schema for input
         */
        public Builder(String name, Map<String, Object> inputSchema) {
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

        public Builder withOutputSchema(Map<String, Object> outputSchema) {
            this.outputSchema = outputSchema;
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
}
