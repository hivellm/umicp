/**
 * Core UMICP classes including Envelope, Matrix, and Frame.
 *
 * <p>This package contains the fundamental building blocks of the UMICP protocol:
 * <ul>
 *   <li>{@link com.hivellm.umicp.core.Envelope} - Message envelope for protocol communication</li>
 *   <li>{@link com.hivellm.umicp.core.EnvelopeOptions} - Builder for creating envelopes</li>
 *   <li>{@link com.hivellm.umicp.core.PayloadHint} - Metadata about payload structure</li>
 *   <li>{@link com.hivellm.umicp.core.Matrix} - High-performance matrix operations</li>
 *   <li>{@link com.hivellm.umicp.core.Frame} - Low-level protocol frames</li>
 * </ul>
 *
 * <p>Example usage:
 * <pre>{@code
 * // Create an envelope
 * Envelope envelope = new Envelope(
 *     EnvelopeOptions.builder()
 *         .from("client-001")
 *         .to("server-001")
 *         .operation(OperationType.DATA)
 *         .messageId(UUID.randomUUID().toString())
 *         .build()
 * );
 *
 * // Serialize and send
 * String json = envelope.serialize();
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
package com.hivellm.umicp.core;

