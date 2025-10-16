package com.hivellm.umicp.core

import com.google.gson.Gson
import com.google.gson.JsonSyntaxException
import com.hivellm.umicp.types.OperationType
import com.hivellm.umicp.types.SerializationException
import com.hivellm.umicp.types.ValidationException
import java.security.MessageDigest
import java.util.*

/**
 * UMICP Envelope - message container
 *
 * @property from Source identifier
 * @property to Destination identifier
 * @property operation Operation type
 * @property messageId Unique message identifier
 * @property timestamp Message timestamp (Unix epoch milliseconds)
 * @property payloadHint Optional payload metadata
 * @property capabilities Optional key-value capabilities (supports native JSON types)
 */
data class Envelope(
    val from: String,
    val to: String,
    val operation: OperationType,
    val messageId: String = UUID.randomUUID().toString(),
    val timestamp: Long = System.currentTimeMillis(),
    val payloadHint: PayloadHint? = null,
    val capabilities: Map<String, Any?> = emptyMap()
) {
    /**
     * Validate the envelope
     *
     * @return true if valid
     */
    fun validate(): Boolean {
        return try {
            validateOrThrow()
            true
        } catch (e: ValidationException) {
            false
        }
    }

    /**
     * Validate the envelope, throw exception if invalid
     *
     * @throws ValidationException if validation fails
     */
    fun validateOrThrow() {
        if (from.isBlank()) {
            throw ValidationException("'from' field cannot be blank")
        }
        if (to.isBlank()) {
            throw ValidationException("'to' field cannot be blank")
        }
        if (messageId.isBlank()) {
            throw ValidationException("'messageId' field cannot be blank")
        }
        if (timestamp <= 0) {
            throw ValidationException("'timestamp' must be positive")
        }
    }

    /**
     * Serialize to JSON string
     *
     * @return JSON string representation
     * @throws SerializationException if serialization fails
     */
    fun serialize(): String {
        return try {
            gson.toJson(this)
        } catch (e: Exception) {
            throw SerializationException("Failed to serialize envelope", e)
        }
    }

    /**
     * Compute SHA-256 hash of the envelope
     *
     * @return Hex-encoded hash
     */
    fun hash(): String {
        val json = serialize()
        val digest = MessageDigest.getInstance("SHA-256")
        val hashBytes = digest.digest(json.toByteArray())
        return hashBytes.joinToString("") { "%02x".format(it) }
    }

    /**
     * Create a copy with modified fields
     */
    fun toBuilder() = Builder().apply {
        from(this@Envelope.from)
        to(this@Envelope.to)
        operation(this@Envelope.operation)
        messageId(this@Envelope.messageId)
        timestamp(this@Envelope.timestamp)
        payloadHint?.let { payloadHint(it) }
        capabilities.forEach { (k, v) -> capability(k, v) }
    }

    /**
     * Builder for Envelope
     */
    class Builder {
        private var from: String = ""
        private var to: String = ""
        private var operation: OperationType = OperationType.DATA
        private var messageId: String = UUID.randomUUID().toString()
        private var timestamp: Long = System.currentTimeMillis()
        private var payloadHint: PayloadHint? = null
        private val capabilities: MutableMap<String, String> = mutableMapOf()

        fun from(from: String) = apply { this.from = from }
        fun to(to: String) = apply { this.to = to }
        fun operation(operation: OperationType) = apply { this.operation = operation }
        fun messageId(messageId: String) = apply { this.messageId = messageId }
        fun timestamp(timestamp: Long) = apply { this.timestamp = timestamp }
        fun payloadHint(hint: PayloadHint) = apply { this.payloadHint = hint }
        fun capability(key: String, value: String) = apply { this.capabilities[key] = value }
        fun capabilities(caps: Map<String, String>) = apply { this.capabilities.putAll(caps) }

        fun build() = Envelope(
            from = from,
            to = to,
            operation = operation,
            messageId = messageId,
            timestamp = timestamp,
            payloadHint = payloadHint,
            capabilities = capabilities.toMap()
        )
    }

    companion object {
        private val gson = Gson()

        /**
         * Create a builder instance
         */
        fun builder() = Builder()

        /**
         * DSL builder function
         */
        inline fun build(block: Builder.() -> Unit) = Builder().apply(block).build()

        /**
         * Deserialize from JSON string
         *
         * @param json JSON string
         * @return Deserialized envelope
         * @throws SerializationException if deserialization fails
         */
        fun deserialize(json: String): Envelope {
            return try {
                gson.fromJson(json, Envelope::class.java)
            } catch (e: JsonSyntaxException) {
                throw SerializationException("Failed to deserialize envelope", e)
            }
        }
    }
}

