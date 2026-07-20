package com.hivellm.umicp.core;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.SerializationException;
import com.hivellm.umicp.types.ValidationException;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * UMICP message envelope for protocol communication.
 *
 * <p>An Envelope is the fundamental unit of communication in UMICP, containing
 * metadata about the message (from, to, operation type) and optional capabilities
 * for protocol negotiation and payload hints.
 *
 * <p>Example usage:
 * <pre>{@code
 * // Create envelope with builder pattern
 * Envelope envelope = new Envelope(
 *     EnvelopeOptions.builder()
 *         .from("client-001")
 *         .to("server-001")
 *         .operation(OperationType.DATA)
 *         .messageId(UUID.randomUUID().toString())
 *         .capability("priority", "high")
 *         .build()
 * );
 *
 * // Serialize to JSON
 * String json = envelope.serialize();
 *
 * // Deserialize from JSON
 * Envelope received = Envelope.deserialize(json);
 *
 * // Validate
 * if (received.validate()) {
 *     // Process message
 * }
 * }</pre>
 *
 * <p>This class implements {@link AutoCloseable} for consistency with JNI-based
 * implementations, though the pure Java version doesn't require cleanup.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see EnvelopeOptions
 * @see OperationType
 */
public class Envelope implements AutoCloseable {

    private static final ObjectMapper MAPPER = new ObjectMapper();

    private String from;
    private String to;
    private OperationType operation;
    private String messageId;
    private Map<String, Object> capabilities;
    private PayloadHint payloadHint;

    // For tracking changes to compute hash
    private transient String cachedHash;
    private transient boolean dirty = true;

    /**
     * Creates an empty envelope.
     *
     * <p>Fields must be set using setter methods or initialized via
     * {@link EnvelopeOptions}.
     */
    public Envelope() {
        this.capabilities = new HashMap<>();
    }

    /**
     * Creates an envelope with the given options.
     *
     * @param options the envelope configuration options
     */
    public Envelope(@NotNull EnvelopeOptions options) {
        this.from = options.getFrom();
        this.to = options.getTo();
        this.operation = options.getOperation();
        this.messageId = options.getMessageId();
        this.capabilities = new HashMap<>(options.getCapabilities());
        this.payloadHint = options.getPayloadHint();
    }

    /**
     * JSON constructor for deserialization.
     */
    @JsonCreator
    private Envelope(
            @JsonProperty("from") String from,
            @JsonProperty("to") String to,
            @JsonProperty("operation") Integer operation,
            @JsonProperty("messageId") String messageId,
            @JsonProperty("capabilities") Map<String, String> capabilities,
            @JsonProperty("payloadHint") PayloadHint payloadHint) {
        this.from = from;
        this.to = to;
        this.operation = operation != null ? OperationType.fromValue(operation) : null;
        this.messageId = messageId;
        this.capabilities = capabilities != null ? new HashMap<>(capabilities) : new HashMap<>();
        this.payloadHint = payloadHint;
    }

    /**
     * Sets the sender identifier.
     *
     * @param from the sender ID
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope setFrom(@Nullable String from) {
        if (!Objects.equals(this.from, from)) {
            this.from = from;
            this.dirty = true;
        }
        return this;
    }

    /**
     * Returns the sender identifier.
     *
     * @return the sender ID, or null if not set
     */
    @Nullable
    @JsonProperty("from")
    public String getFrom() {
        return from;
    }

    /**
     * Sets the receiver identifier.
     *
     * @param to the receiver ID
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope setTo(@Nullable String to) {
        if (!Objects.equals(this.to, to)) {
            this.to = to;
            this.dirty = true;
        }
        return this;
    }

    /**
     * Returns the receiver identifier.
     *
     * @return the receiver ID, or null if not set
     */
    @Nullable
    @JsonProperty("to")
    public String getTo() {
        return to;
    }

    /**
     * Sets the operation type.
     *
     * @param operation the operation type
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope setOperation(@Nullable OperationType operation) {
        if (this.operation != operation) {
            this.operation = operation;
            this.dirty = true;
        }
        return this;
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
     * Returns the operation type as integer for JSON serialization.
     */
    @JsonProperty("operation")
    private Integer getOperationValue() {
        return operation != null ? operation.getValue() : null;
    }

    /**
     * Sets the message identifier.
     *
     * @param messageId the message ID
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope setMessageId(@Nullable String messageId) {
        if (!Objects.equals(this.messageId, messageId)) {
            this.messageId = messageId;
            this.dirty = true;
        }
        return this;
    }

    /**
     * Returns the message identifier.
     *
     * @return the message ID, or null if not set
     */
    @Nullable
    @JsonProperty("messageId")
    public String getMessageId() {
        return messageId;
    }

    /**
     * Sets all capabilities at once.
     *
     * @param capabilities a map of capability key-value pairs
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope setCapabilities(@Nullable Map<String, Object> capabilities) {
        if (!Objects.equals(this.capabilities, capabilities)) {
            this.capabilities = capabilities != null ? new HashMap<>(capabilities) : new HashMap<>();
            this.dirty = true;
        }
        return this;
    }

    /**
     * Returns the capabilities map.
     *
     * @return a copy of the capabilities map
     */
    @NotNull
    @JsonProperty("capabilities")
    public Map<String, Object> getCapabilities() {
        return new HashMap<>(capabilities);
    }

    /**
     * Adds or updates a single capability.
     *
     * @param key the capability key
     * @param value the capability value
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope addCapability(@NotNull String key, @NotNull Object value) {
        if (!value.equals(capabilities.get(key))) {
            capabilities.put(key, value);
            this.dirty = true;
        }
        return this;
    }

    /**
     * Adds or updates a single string capability (helper method).
     *
     * @param key the capability key
     * @param value the capability value
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope addCapabilityString(@NotNull String key, @NotNull String value) {
        return addCapability(key, value);
    }

    /**
     * Removes a capability.
     *
     * @param key the capability key to remove
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope removeCapability(@NotNull String key) {
        if (capabilities.remove(key) != null) {
            this.dirty = true;
        }
        return this;
    }

    /**
     * Sets the payload hint.
     *
     * @param payloadHint the payload hint
     * @return this envelope for method chaining
     */
    @NotNull
    public Envelope setPayloadHint(@Nullable PayloadHint payloadHint) {
        if (!Objects.equals(this.payloadHint, payloadHint)) {
            this.payloadHint = payloadHint;
            this.dirty = true;
        }
        return this;
    }

    /**
     * Returns the payload hint.
     *
     * @return the payload hint, or null if not set
     */
    @Nullable
    @JsonProperty("payloadHint")
    public PayloadHint getPayloadHint() {
        return payloadHint;
    }

    /**
     * Serializes this envelope to JSON string.
     *
     * @return the JSON representation
     * @throws SerializationException if serialization fails
     */
    @NotNull
    public String serialize() throws SerializationException {
        try {
            return MAPPER.writeValueAsString(this);
        } catch (JsonProcessingException e) {
            throw new SerializationException("Failed to serialize envelope", e);
        }
    }

    /**
     * Deserializes an envelope from JSON string.
     *
     * @param json the JSON string
     * @return the deserialized envelope
     * @throws SerializationException if deserialization fails
     */
    @NotNull
    public static Envelope deserialize(@NotNull String json) throws SerializationException {
        try {
            return MAPPER.readValue(json, Envelope.class);
        } catch (JsonProcessingException e) {
            throw new SerializationException("Failed to deserialize envelope", e);
        }
    }

    /** Alias for {@link #serialize()} kept for the transport layer. */
    @NotNull
    public String toJson() throws SerializationException {
        return serialize();
    }

    /** Alias for {@link #deserialize(String)} kept for the transport layer. */
    @NotNull
    public static Envelope fromJson(@NotNull String json) throws SerializationException {
        return deserialize(json);
    }

    /**
     * Validates this envelope.
     *
     * <p>A valid envelope must have:
     * <ul>
     *   <li>Non-null and non-empty 'from' field</li>
     *   <li>Non-null and non-empty 'to' field</li>
     *   <li>Non-null 'operation' field</li>
     * </ul>
     *
     * @return true if the envelope is valid, false otherwise
     */
    public boolean validate() {
        // Required fields
        if (from == null || from.trim().isEmpty()) {
            return false;
        }
        if (to == null || to.trim().isEmpty()) {
            return false;
        }
        if (operation == null) {
            return false;
        }

        // Validate payload hint if present
        if (payloadHint != null && !payloadHint.validate()) {
            return false;
        }

        return true;
    }

    /**
     * Validates this envelope and throws an exception if invalid.
     *
     * @throws ValidationException if the envelope is invalid
     */
    public void validateOrThrow() throws ValidationException {
        if (from == null || from.trim().isEmpty()) {
            throw new ValidationException("Envelope 'from' field is required");
        }
        if (to == null || to.trim().isEmpty()) {
            throw new ValidationException("Envelope 'to' field is required");
        }
        if (operation == null) {
            throw new ValidationException("Envelope 'operation' field is required");
        }
        if (payloadHint != null && !payloadHint.validate()) {
            throw new ValidationException("Envelope payload hint is invalid");
        }
    }

    /**
     * Computes the SHA-256 hash of this envelope.
     *
     * <p>The hash is computed from the serialized JSON representation and
     * is cached until the envelope is modified.
     *
     * @return the hex-encoded hash string
     * @throws SerializationException if serialization fails
     */
    @com.fasterxml.jackson.annotation.JsonIgnore
    @NotNull
    public String getHash() throws SerializationException {
        if (cachedHash == null || dirty) {
            String json = serialize();
            try {
                MessageDigest digest = MessageDigest.getInstance("SHA-256");
                byte[] hashBytes = digest.digest(json.getBytes(StandardCharsets.UTF_8));
                cachedHash = bytesToHex(hashBytes);
                dirty = false;
            } catch (NoSuchAlgorithmException e) {
                throw new SerializationException("SHA-256 algorithm not available", e);
            }
        }
        return cachedHash;
    }

    /**
     * Converts byte array to hex string.
     */
    private static String bytesToHex(byte[] bytes) {
        StringBuilder result = new StringBuilder();
        for (byte b : bytes) {
            result.append(String.format("%02x", b));
        }
        return result.toString();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Envelope envelope = (Envelope) o;
        return Objects.equals(from, envelope.from) &&
               Objects.equals(to, envelope.to) &&
               operation == envelope.operation &&
               Objects.equals(messageId, envelope.messageId) &&
               Objects.equals(capabilities, envelope.capabilities) &&
               Objects.equals(payloadHint, envelope.payloadHint);
    }

    @Override
    public int hashCode() {
        return Objects.hash(from, to, operation, messageId, capabilities, payloadHint);
    }

    @Override
    public String toString() {
        return "Envelope{" +
               "from='" + from + '\'' +
               ", to='" + to + '\'' +
               ", operation=" + operation +
               ", messageId='" + messageId + '\'' +
               ", capabilities=" + capabilities +
               ", payloadHint=" + payloadHint +
               '}';
    }

    /**
     * Closes this envelope and releases any resources.
     *
     * <p>In the pure Java implementation, this method does nothing.
     * It's provided for compatibility with JNI-based implementations.
     */
    @Override
    public void close() {
        // No resources to release in pure Java implementation
    }
}

