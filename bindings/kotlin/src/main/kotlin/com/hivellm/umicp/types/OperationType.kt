package com.hivellm.umicp.types

import kotlinx.serialization.Serializable

/**
 * UMICP Operation Types
 *
 * Defines the type of operation being performed in a message envelope.
 */
@Serializable
enum class OperationType {
    /** Control messages for protocol management */
    CONTROL,

    /** Data transfer messages */
    DATA,

    /** Acknowledgment messages */
    ACK,

    /** Error messages */
    ERROR,

    /** Request messages */
    REQUEST,

    /** Response messages */
    RESPONSE;

    companion object {
        /**
         * Convert from string value
         */
        fun fromString(value: String): OperationType? =
            entries.find { it.name.equals(value, ignoreCase = true) }
    }
}

