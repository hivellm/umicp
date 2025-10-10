package com.hivellm.umicp.types

/**
 * Base exception for UMICP errors
 */
open class UMICPException(message: String, cause: Throwable? = null) : Exception(message, cause)

/**
 * Exception thrown during serialization/deserialization
 */
class SerializationException(message: String, cause: Throwable? = null) : UMICPException(message, cause)

/**
 * Exception thrown during validation
 */
class ValidationException(message: String, cause: Throwable? = null) : UMICPException(message, cause)

/**
 * Exception thrown during transport operations
 */
class TransportException(message: String, cause: Throwable? = null) : UMICPException(message, cause)

/**
 * Exception thrown during connection operations
 */
class ConnectionException(message: String, cause: Throwable? = null) : UMICPException(message, cause)

/**
 * Exception thrown during matrix operations
 */
class MatrixException(message: String, cause: Throwable? = null) : UMICPException(message, cause)

