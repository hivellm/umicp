package com.hivellm.umicp.core;

import com.hivellm.umicp.types.OperationType;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * Configuration options for creating an Envelope.
 *
 * <p>This class uses the Builder pattern to provide a fluent API for
 * configuring envelope properties.
 *
 * <p>Example usage:
 * <pre>{@code
 * EnvelopeOptions options = EnvelopeOptions.builder()
 *     .from("client-001")
 *     .to("server-001")
 *     .operation(OperationType.DATA)
 *     .messageId(UUID.randomUUID().toString())
 *     .capability("priority", "high")
 *     .capability("content-type", "application/json")
 *     .payloadHint(PayloadHint.builder()
 *         .type(PayloadType.TEXT)
 *         .build())
 *     .build();
 *
 * Envelope envelope = new Envelope(options);
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see Envelope
 */
public class EnvelopeOptions {

    private final String from;
    private final String to;
    private final OperationType operation;
    private final String messageId;
    private final Map<String, Object> capabilities;
    private final PayloadHint payloadHint;

    /**
     * Private constructor. Use {@link Builder} to create instances.
     */
    private EnvelopeOptions(String from, String to, OperationType operation,
                           String messageId, Map<String, Object> capabilities,
                           PayloadHint payloadHint) {
        this.from = from;
        this.to = to;
        this.operation = operation;
        this.messageId = messageId;
        this.capabilities = capabilities != null ? new HashMap<>(capabilities) : new HashMap<>();
        this.payloadHint = payloadHint;
    }

    /**
     * Returns the sender identifier.
     *
     * @return the sender ID, or null if not set
     */
    @Nullable
    public String getFrom() {
        return from;
    }

    /**
     * Returns the receiver identifier.
     *
     * @return the receiver ID, or null if not set
     */
    @Nullable
    public String getTo() {
        return to;
    }

    /**
     * Returns the operation type.
     *
     * @return the operation type, or null if not set
     */
    @Nullable
    public OperationType getOperation() {
        return operation;
    }

    /**
     * Returns the message identifier.
     *
     * @return the message ID, or null if not set
     */
    @Nullable
    public String getMessageId() {
        return messageId;
    }

    /**
     * Returns the capabilities map.
     *
     * @return an unmodifiable view of the capabilities map
     */
    @NotNull
    public Map<String, Object> getCapabilities() {
        return new HashMap<>(capabilities);
    }

    /**
     * Returns the payload hint.
     *
     * @return the payload hint, or null if not set
     */
    @Nullable
    public PayloadHint getPayloadHint() {
        return payloadHint;
    }

    /**
     * Creates a new builder for EnvelopeOptions.
     *
     * @return a new builder instance
     */
    @NotNull
    public static Builder builder() {
        return new Builder();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        EnvelopeOptions that = (EnvelopeOptions) o;
        return Objects.equals(from, that.from) &&
               Objects.equals(to, that.to) &&
               operation == that.operation &&
               Objects.equals(messageId, that.messageId) &&
               Objects.equals(capabilities, that.capabilities) &&
               Objects.equals(payloadHint, that.payloadHint);
    }

    @Override
    public int hashCode() {
        return Objects.hash(from, to, operation, messageId, capabilities, payloadHint);
    }

    @Override
    public String toString() {
        return "EnvelopeOptions{" +
               "from='" + from + '\'' +
               ", to='" + to + '\'' +
               ", operation=" + operation +
               ", messageId='" + messageId + '\'' +
               ", capabilities=" + capabilities +
               ", payloadHint=" + payloadHint +
               '}';
    }

    /**
     * Builder for creating EnvelopeOptions instances.
     */
    public static class Builder {
        private String from;
        private String to;
        private OperationType operation;
        private String messageId;
        private Map<String, Object> capabilities;
        private PayloadHint payloadHint;

        /**
         * Sets the sender identifier.
         *
         * @param from the sender ID
         * @return this builder
         */
        @NotNull
        public Builder from(@Nullable String from) {
            this.from = from;
            return this;
        }

        /**
         * Sets the receiver identifier.
         *
         * @param to the receiver ID
         * @return this builder
         */
        @NotNull
        public Builder to(@Nullable String to) {
            this.to = to;
            return this;
        }

        /**
         * Sets the operation type.
         *
         * @param operation the operation type
         * @return this builder
         */
        @NotNull
        public Builder operation(@Nullable OperationType operation) {
            this.operation = operation;
            return this;
        }

        /**
         * Sets the message identifier.
         *
         * @param messageId the message ID
         * @return this builder
         */
        @NotNull
        public Builder messageId(@Nullable String messageId) {
            this.messageId = messageId;
            return this;
        }

        /**
         * Sets all capabilities at once.
         *
         * @param capabilities a map of capability key-value pairs
         * @return this builder
         */
        @NotNull
        public Builder capabilities(@Nullable Map<String, Object> capabilities) {
            this.capabilities = capabilities != null ? new HashMap<>(capabilities) : null;
            return this;
        }

        /**
         * Adds a single capability with any type.
         *
         * @param key the capability key
         * @param value the capability value
         * @return this builder
         */
        @NotNull
        public Builder capability(@NotNull String key, @NotNull Object value) {
            if (this.capabilities == null) {
                this.capabilities = new HashMap<>();
            }
            this.capabilities.put(key, value);
            return this;
        }
        
        /**
         * Adds a single string capability (helper method).
         *
         * @param key the capability key
         * @param value the capability value
         * @return this builder
         */
        @NotNull
        public Builder capabilityString(@NotNull String key, @NotNull String value) {
            return capability(key, value);
        }

        /**
         * Sets the payload hint.
         *
         * @param payloadHint the payload hint
         * @return this builder
         */
        @NotNull
        public Builder payloadHint(@Nullable PayloadHint payloadHint) {
            this.payloadHint = payloadHint;
            return this;
        }

        /**
         * Builds the EnvelopeOptions instance.
         *
         * @return a new EnvelopeOptions instance
         */
        @NotNull
        public EnvelopeOptions build() {
            return new EnvelopeOptions(from, to, operation, messageId, capabilities, payloadHint);
        }
    }
}

