package com.hivellm.umicp.core;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.hivellm.umicp.types.EncodingType;
import com.hivellm.umicp.types.PayloadType;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.Objects;

/**
 * Hint about the payload structure and encoding.
 *
 * <p>PayloadHint provides metadata about the message payload, helping receivers
 * optimize processing and validate data format. This is particularly useful for
 * vector/matrix data where encoding and dimensions matter.
 *
 * <p>Example usage:
 * <pre>{@code
 * // For embeddings (768-dimensional FLOAT32 vector)
 * PayloadHint hint = PayloadHint.builder()
 *     .type(PayloadType.VECTOR)
 *     .encoding(EncodingType.FLOAT32)
 *     .count(768)
 *     .size(768 * 4)  // 4 bytes per float32
 *     .build();
 *
 * envelope.setPayloadHint(hint);
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see PayloadType
 * @see EncodingType
 */
public class PayloadHint {

    private final PayloadType type;
    private final Integer size;
    private final EncodingType encoding;
    private final Integer count;

    /**
     * Private constructor. Use {@link Builder} to create instances.
     */
    @JsonCreator
    private PayloadHint(
            @JsonProperty("type") PayloadType type,
            @JsonProperty("size") Integer size,
            @JsonProperty("encoding") EncodingType encoding,
            @JsonProperty("count") Integer count) {
        this.type = type;
        this.size = size;
        this.encoding = encoding;
        this.count = count;
    }

    /**
     * Returns the payload type.
     *
     * @return the payload type, or null if not set
     */
    @Nullable
    public PayloadType getType() {
        return type;
    }

    /**
     * Returns the payload size in bytes.
     *
     * @return the size in bytes, or null if not set
     */
    @Nullable
    public Integer getSize() {
        return size;
    }

    /**
     * Returns the encoding type for numerical data.
     *
     * @return the encoding type, or null if not set
     */
    @Nullable
    public EncodingType getEncoding() {
        return encoding;
    }

    /**
     * Returns the count of elements (for vectors/arrays).
     *
     * @return the element count, or null if not set
     */
    @Nullable
    public Integer getCount() {
        return count;
    }

    /**
     * Creates a new builder for PayloadHint.
     *
     * @return a new builder instance
     */
    @NotNull
    public static Builder builder() {
        return new Builder();
    }

    /**
     * Validates this payload hint.
     *
     * @return true if the hint is valid and consistent
     */
    public boolean validate() {
        // If encoding is set, type should be VECTOR
        if (encoding != null && type != PayloadType.VECTOR) {
            return false;
        }

        // If count is set, should be positive
        if (count != null && count <= 0) {
            return false;
        }

        // If size is set, should be positive
        if (size != null && size <= 0) {
            return false;
        }

        // If both encoding and count are set, size should match
        if (encoding != null && count != null && size != null) {
            int expectedSize = encoding.calculateTotalBytes(count);
            if (size != expectedSize) {
                return false;
            }
        }

        return true;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        PayloadHint that = (PayloadHint) o;
        return type == that.type &&
               Objects.equals(size, that.size) &&
               encoding == that.encoding &&
               Objects.equals(count, that.count);
    }

    @Override
    public int hashCode() {
        return Objects.hash(type, size, encoding, count);
    }

    @Override
    public String toString() {
        return "PayloadHint{" +
               "type=" + type +
               ", size=" + size +
               ", encoding=" + encoding +
               ", count=" + count +
               '}';
    }

    /**
     * Builder for creating PayloadHint instances.
     */
    public static class Builder {
        private PayloadType type;
        private Integer size;
        private EncodingType encoding;
        private Integer count;

        /**
         * Sets the payload type.
         *
         * @param type the payload type
         * @return this builder
         */
        @NotNull
        public Builder type(@Nullable PayloadType type) {
            this.type = type;
            return this;
        }

        /**
         * Sets the payload size in bytes.
         *
         * @param size the size in bytes
         * @return this builder
         * @throws IllegalArgumentException if size is negative or zero
         */
        @NotNull
        public Builder size(int size) {
            if (size <= 0) {
                throw new IllegalArgumentException("Size must be positive: " + size);
            }
            this.size = size;
            return this;
        }

        /**
         * Sets the encoding type for numerical data.
         *
         * @param encoding the encoding type
         * @return this builder
         */
        @NotNull
        public Builder encoding(@Nullable EncodingType encoding) {
            this.encoding = encoding;
            return this;
        }

        /**
         * Sets the count of elements.
         *
         * @param count the element count
         * @return this builder
         * @throws IllegalArgumentException if count is negative or zero
         */
        @NotNull
        public Builder count(int count) {
            if (count <= 0) {
                throw new IllegalArgumentException("Count must be positive: " + count);
            }
            this.count = count;
            return this;
        }

        /**
         * Builds the PayloadHint instance.
         *
         * <p>If both encoding and count are set but size is not, the size
         * will be automatically calculated.
         *
         * @return a new PayloadHint instance
         */
        @NotNull
        public PayloadHint build() {
            // Auto-calculate size if encoding and count are set
            if (encoding != null && count != null && size == null) {
                size = encoding.calculateTotalBytes(count);
            }

            return new PayloadHint(type, size, encoding, count);
        }
    }
}

