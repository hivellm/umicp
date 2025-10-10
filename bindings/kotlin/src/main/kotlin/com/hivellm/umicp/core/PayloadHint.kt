package com.hivellm.umicp.core

import com.hivellm.umicp.types.EncodingType
import com.hivellm.umicp.types.PayloadType
import kotlinx.serialization.Serializable

/**
 * Payload Hint - provides metadata about the payload
 *
 * @property type The type of payload data
 * @property encoding The encoding format
 * @property count Number of elements
 * @property shape Shape of the data (for multi-dimensional arrays)
 */
@Serializable
data class PayloadHint(
    val type: PayloadType,
    val encoding: EncodingType? = null,
    val count: Int? = null,
    val shape: List<Int>? = null
) {
    /**
     * Builder for PayloadHint
     */
    class Builder {
        private var type: PayloadType = PayloadType.BINARY
        private var encoding: EncodingType? = null
        private var count: Int? = null
        private var shape: List<Int>? = null

        fun type(type: PayloadType) = apply { this.type = type }
        fun encoding(encoding: EncodingType) = apply { this.encoding = encoding }
        fun count(count: Int) = apply { this.count = count }
        fun shape(vararg dimensions: Int) = apply { this.shape = dimensions.toList() }
        fun shape(dimensions: List<Int>) = apply { this.shape = dimensions }

        fun build() = PayloadHint(type, encoding, count, shape)
    }

    companion object {
        /**
         * Create a builder instance
         */
        fun builder() = Builder()

        /**
         * DSL builder function
         */
        inline fun build(block: Builder.() -> Unit) = Builder().apply(block).build()
    }
}

