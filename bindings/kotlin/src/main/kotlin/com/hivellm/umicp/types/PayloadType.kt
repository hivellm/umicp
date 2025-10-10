package com.hivellm.umicp.types

import kotlinx.serialization.Serializable

/**
 * UMICP Payload Types
 *
 * Defines the type of data contained in a message payload.
 */
@Serializable
enum class PayloadType {
    /** Vector/array data (e.g., embeddings, tensors) */
    VECTOR,

    /** Text/string data */
    TEXT,

    /** Metadata (key-value pairs) */
    METADATA,

    /** Binary data */
    BINARY;

    companion object {
        /**
         * Convert from string value
         */
        fun fromString(value: String): PayloadType? =
            entries.find { it.name.equals(value, ignoreCase = true) }
    }
}

