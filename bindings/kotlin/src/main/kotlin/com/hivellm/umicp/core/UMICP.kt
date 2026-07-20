package com.hivellm.umicp.core

/**
 * UMICP - Main facade for the SDK
 *
 * Provides static utility methods and version information.
 */
object UMICP {
    private const val VERSION = "0.1.2"
    private const val PROTOCOL_VERSION = "1.0"
    private const val BUILD_TYPE = "kotlin-pure"

    /**
     * Get SDK version
     */
    fun getVersion(): String = VERSION

    /**
     * Get protocol version
     */
    fun getProtocolVersion(): String = PROTOCOL_VERSION

    /**
     * Get build type
     */
    fun getBuildType(): String = BUILD_TYPE

    /**
     * Print version information
     */
    fun printVersion() {
        println("UMICP Kotlin SDK v$VERSION (Protocol: $PROTOCOL_VERSION, Build: $BUILD_TYPE)")
    }

    /**
     * Create an envelope with builder
     */
    fun createEnvelope(block: Envelope.Builder.() -> Unit = {}): Envelope {
        return Envelope.Builder().apply(block).build()
    }

    /**
     * Create a payload hint with builder
     */
    fun createPayloadHint(block: PayloadHint.Builder.() -> Unit = {}): PayloadHint {
        return PayloadHint.Builder().apply(block).build()
    }

    /**
     * Serialize an envelope to JSON
     */
    fun serialize(envelope: Envelope): String = envelope.serialize()

    /**
     * Deserialize an envelope from JSON
     */
    fun deserialize(json: String): Envelope = Envelope.deserialize(json)

    /**
     * Validate an envelope
     */
    fun validate(envelope: Envelope): Boolean = envelope.validate()

    /**
     * Compute hash of an envelope
     */
    fun hash(envelope: Envelope): String = envelope.hash()
}

