package com.hivellm.umicp.transport;

import com.hivellm.umicp.types.ValidationException;
import org.jetbrains.annotations.NotNull;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * Security validator for UMICP transport layer.
 *
 * <p>Provides validation for security-critical operations including:
 * <ul>
 *   <li>Message size validation (DoS protection)</li>
 *   <li>URI validation (injection protection)</li>
 *   <li>Input sanitization</li>
 * </ul>
 *
 * @author HiveLLM Team
 * @version 0.1.1
 * @since 0.1.1
 */
public final class SecurityValidator {

    private static final Logger logger = LoggerFactory.getLogger(SecurityValidator.class);

    /**
     * Maximum message size: 1MB.
     * Protects against DoS attacks via oversized messages.
     */
    public static final int MAX_MESSAGE_SIZE = 1024 * 1024; // 1MB

    /**
     * Minimum message size: 1 byte.
     */
    public static final int MIN_MESSAGE_SIZE = 1;

    /**
     * Maximum URI length.
     */
    public static final int MAX_URI_LENGTH = 2048;

    /**
     * Private constructor - utility class.
     */
    private SecurityValidator() {
        throw new UnsupportedOperationException("SecurityValidator is a utility class");
    }

    /**
     * Validates message size to prevent DoS attacks.
     *
     * @param json the JSON message to validate
     * @throws ValidationException if message exceeds size limits
     */
    public static void validateMessageSize(@NotNull String json) throws ValidationException {
        if (json == null) {
            throw new ValidationException("Message cannot be null");
        }

        int size = json.length();

        if (size < MIN_MESSAGE_SIZE) {
            throw new ValidationException(
                String.format("Message too small: %d bytes (min: %d)", size, MIN_MESSAGE_SIZE)
            );
        }

        if (size > MAX_MESSAGE_SIZE) {
            logger.warn("Message exceeds size limit: {} bytes (max: {})", size, MAX_MESSAGE_SIZE);
            throw new ValidationException(
                String.format("Message exceeds maximum size: %d bytes (max: %d)",
                    size, MAX_MESSAGE_SIZE)
            );
        }
    }

    /**
     * Validates WebSocket URI for security.
     *
     * <p>Ensures:
     * <ul>
     *   <li>Valid URI syntax</li>
     *   <li>Correct WebSocket scheme (ws:// or wss://)</li>
     *   <li>No malicious characters</li>
     *   <li>Length within limits</li>
     * </ul>
     *
     * @param uri the URI to validate
     * @throws ValidationException if URI is invalid or potentially malicious
     */
    public static void validateURI(@NotNull String uri) throws ValidationException {
        if (uri == null || uri.isEmpty()) {
            throw new ValidationException("URI cannot be null or empty");
        }

        if (uri.length() > MAX_URI_LENGTH) {
            throw new ValidationException(
                String.format("URI too long: %d characters (max: %d)",
                    uri.length(), MAX_URI_LENGTH)
            );
        }

        // Validate WebSocket scheme
        if (!uri.startsWith("ws://") && !uri.startsWith("wss://")) {
            throw new ValidationException(
                "Invalid WebSocket URI: must start with ws:// or wss://"
            );
        }

        // Parse and validate URI syntax
        try {
            URI parsedUri = new URI(uri);

            // Validate host is present
            if (parsedUri.getHost() == null || parsedUri.getHost().isEmpty()) {
                throw new ValidationException("URI must contain a valid host");
            }

            // Validate port if specified
            if (parsedUri.getPort() != -1) {
                int port = parsedUri.getPort();
                if (port < 1 || port > 65535) {
                    throw new ValidationException(
                        String.format("Invalid port: %d (must be 1-65535)", port)
                    );
                }
            }

        } catch (URISyntaxException e) {
            throw new ValidationException("Invalid URI syntax: " + sanitizeErrorMessage(e.getMessage()), e);
        }
    }

    /**
     * Sanitizes error messages to prevent information disclosure.
     *
     * <p>Removes potentially sensitive information from error messages
     * before exposing them to clients.
     *
     * @param message the original error message
     * @return sanitized error message
     */
    @NotNull
    public static String sanitizeErrorMessage(@NotNull String message) {
        if (message == null) {
            return "An error occurred";
        }

        // Remove file paths
        message = message.replaceAll("[A-Za-z]:\\\\[\\w\\\\]+", "[PATH]");
        message = message.replaceAll("/[\\w/]+\\.java", "[FILE]");

        // Remove IP addresses
        message = message.replaceAll("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}", "[IP]");

        // Remove port numbers in errors
        message = message.replaceAll("port \\d+", "port [PORT]");

        // Truncate if too long
        if (message.length() > 200) {
            message = message.substring(0, 197) + "...";
        }

        return message;
    }

    /**
     * Validates configuration parameter.
     *
     * @param value the value to validate
     * @param min minimum allowed value
     * @param max maximum allowed value
     * @param paramName parameter name for error messages
     * @throws ValidationException if value is out of range
     */
    public static void validateRange(int value, int min, int max, @NotNull String paramName)
            throws ValidationException {
        if (value < min || value > max) {
            throw new ValidationException(
                String.format("%s must be between %d and %d (got: %d)",
                    paramName, min, max, value)
            );
        }
    }

    /**
     * Validates duration parameter.
     *
     * @param milliseconds the duration in milliseconds
     * @param min minimum allowed value
     * @param paramName parameter name for error messages
     * @throws ValidationException if duration is invalid
     */
    public static void validateDuration(long milliseconds, long min, @NotNull String paramName)
            throws ValidationException {
        if (milliseconds < min) {
            throw new ValidationException(
                String.format("%s must be at least %d ms (got: %d ms)",
                    paramName, min, milliseconds)
            );
        }
    }

    /**
     * Sanitizes string input by removing potentially dangerous characters.
     *
     * @param input the input to sanitize
     * @return sanitized string
     */
    @NotNull
    public static String sanitizeInput(@NotNull String input) {
        if (input == null) {
            return "";
        }

        // Remove control characters except newline and tab
        return input.replaceAll("[\\x00-\\x08\\x0B-\\x0C\\x0E-\\x1F\\x7F]", "");
    }
}

