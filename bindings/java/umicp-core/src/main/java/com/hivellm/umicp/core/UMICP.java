package com.hivellm.umicp.core;

import com.hivellm.umicp.types.SerializationException;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Main UMICP utility class providing static factory methods and utilities.
 *
 * <p>This class serves as the primary entry point for the UMICP API, offering
 * convenient factory methods for creating core objects and performing common
 * operations.
 *
 * <p>Example usage:
 * <pre>{@code
 * // Create envelope
 * Envelope envelope = UMICP.createEnvelope(
 *     EnvelopeOptions.builder()
 *         .from("client")
 *         .to("server")
 *         .operation(OperationType.DATA)
 *         .build()
 * );
 *
 * // Create matrix
 * try (Matrix matrix = UMICP.createMatrix()) {
 *     MatrixResult result = matrix.dotProduct(a, b);
 * }
 *
 * // Serialize/deserialize
 * String json = UMICP.serialize(envelope);
 * Envelope received = UMICP.deserialize(json);
 *
 * // Validate
 * boolean valid = UMICP.validate(envelope);
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public final class UMICP {

    /**
     * UMICP version string.
     */
    public static final String VERSION = "1.0.0";

    /**
     * UMICP protocol version.
     */
    public static final String PROTOCOL_VERSION = "1.0";

    /**
     * Build information.
     */
    public static final String BUILD = "java-pure";

    /**
     * Private constructor to prevent instantiation.
     */
    private UMICP() {
        throw new UnsupportedOperationException("UMICP is a utility class and cannot be instantiated");
    }

    // ==================== Envelope Factory Methods ====================

    /**
     * Creates a new empty envelope.
     *
     * @return a new Envelope instance
     */
    @NotNull
    public static Envelope createEnvelope() {
        return new Envelope();
    }

    /**
     * Creates a new envelope with the given options.
     *
     * @param options the envelope configuration options
     * @return a new Envelope instance
     */
    @NotNull
    public static Envelope createEnvelope(@NotNull EnvelopeOptions options) {
        return new Envelope(options);
    }

    /**
     * Creates a new envelope options builder.
     *
     * @return a new EnvelopeOptions.Builder instance
     */
    @NotNull
    public static EnvelopeOptions.Builder envelopeOptions() {
        return EnvelopeOptions.builder();
    }

    // ==================== Matrix Factory Methods ====================

    /**
     * Creates a new matrix instance.
     *
     * @return a new Matrix instance
     */
    @NotNull
    public static Matrix createMatrix() {
        return new Matrix();
    }

    // ==================== PayloadHint Factory Methods ====================

    /**
     * Creates a new payload hint builder.
     *
     * @return a new PayloadHint.Builder instance
     */
    @NotNull
    public static PayloadHint.Builder payloadHint() {
        return PayloadHint.builder();
    }

    // ==================== Serialization Methods ====================

    /**
     * Serializes an envelope to JSON string.
     *
     * @param envelope the envelope to serialize
     * @return the JSON representation
     * @throws SerializationException if serialization fails
     */
    @NotNull
    public static String serialize(@NotNull Envelope envelope) throws SerializationException {
        return envelope.serialize();
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
        return Envelope.deserialize(json);
    }

    // ==================== Validation Methods ====================

    /**
     * Validates an envelope.
     *
     * @param envelope the envelope to validate
     * @return true if valid, false otherwise
     */
    public static boolean validate(@NotNull Envelope envelope) {
        return envelope.validate();
    }

    /**
     * Computes the hash of an envelope.
     *
     * @param envelope the envelope to hash
     * @return the hex-encoded hash string
     * @throws SerializationException if serialization fails
     */
    @NotNull
    public static String hash(@NotNull Envelope envelope) throws SerializationException {
        return envelope.getHash();
    }

    // ==================== Version Information ====================

    /**
     * Returns the UMICP library version.
     *
     * @return the version string
     */
    @NotNull
    public static String getVersion() {
        return VERSION;
    }

    /**
     * Returns the UMICP protocol version.
     *
     * @return the protocol version string
     */
    @NotNull
    public static String getProtocolVersion() {
        return PROTOCOL_VERSION;
    }

    /**
     * Returns build information.
     *
     * @return the build string
     */
    @NotNull
    public static String getBuild() {
        return BUILD;
    }

    /**
     * Returns version information as a formatted string.
     *
     * @return version information string
     */
    @NotNull
    public static String getVersionInfo() {
        return String.format("UMICP Java Bindings v%s (Protocol: %s, Build: %s)",
                           VERSION, PROTOCOL_VERSION, BUILD);
    }

    // ==================== Utility Methods ====================

    /**
     * Checks if native (JNI) implementation is available.
     *
     * @return true if native implementation is available, false if using pure Java
     */
    public static boolean isNativeAvailable() {
        // Pure Java implementation for now
        return false;
    }

    /**
     * Returns implementation type.
     *
     * @return "native" if JNI is available, "pure-java" otherwise
     */
    @NotNull
    public static String getImplementationType() {
        return isNativeAvailable() ? "native" : "pure-java";
    }

    /**
     * Prints version information to standard output.
     */
    public static void printVersion() {
        System.out.println(getVersionInfo());
        System.out.println("Implementation: " + getImplementationType());
    }
}

