package com.hivellm.umicp.types

import kotlinx.serialization.Serializable

/**
 * UMICP Encoding Types
 *
 * Defines the encoding format for payload data.
 */
@Serializable
enum class EncodingType {
    /** 32-bit floating point */
    FLOAT32,

    /** 64-bit floating point */
    FLOAT64,

    /** 32-bit signed integer */
    INT32,

    /** 64-bit signed integer */
    INT64,

    /** 8-bit unsigned integer */
    UINT8,

    /** 16-bit unsigned integer */
    UINT16,

    /** 32-bit unsigned integer */
    UINT32,

    /** 64-bit unsigned integer */
    UINT64;

    companion object {
        /**
         * Convert from string value
         */
        fun fromString(value: String): EncodingType? =
            entries.find { it.name.equals(value, ignoreCase = true) }
    }
}

