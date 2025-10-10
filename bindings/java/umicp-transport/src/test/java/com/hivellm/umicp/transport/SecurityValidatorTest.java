package com.hivellm.umicp.transport;

import com.hivellm.umicp.types.ValidationException;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link SecurityValidator}.
 *
 * @author HiveLLM Team
 * @version 0.1.1
 */
class SecurityValidatorTest {

    @Test
    @DisplayName("Should reject null message")
    void testNullMessage() {
        assertThatThrownBy(() -> SecurityValidator.validateMessageSize(null))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("cannot be null");
    }

    @Test
    @DisplayName("Should reject empty message")
    void testEmptyMessage() {
        assertThatThrownBy(() -> SecurityValidator.validateMessageSize(""))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("too small");
    }

    @Test
    @DisplayName("Should accept valid message size")
    void testValidMessageSize() {
        String message = "x".repeat(1000);
        assertThatNoException().isThrownBy(() ->
            SecurityValidator.validateMessageSize(message));
    }

    @Test
    @DisplayName("Should reject oversized message (DoS protection)")
    void testOversizedMessage() {
        // Create message larger than 1MB
        String message = "x".repeat(SecurityValidator.MAX_MESSAGE_SIZE + 1);

        assertThatThrownBy(() -> SecurityValidator.validateMessageSize(message))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("exceeds maximum size");
    }

    @Test
    @DisplayName("Should accept message at maximum size")
    void testMessageAtMaximumSize() {
        String message = "x".repeat(SecurityValidator.MAX_MESSAGE_SIZE);
        assertThatNoException().isThrownBy(() ->
            SecurityValidator.validateMessageSize(message));
    }

    @Test
    @DisplayName("Should validate valid WebSocket URI")
    void testValidWebSocketURI() {
        assertThatNoException().isThrownBy(() ->
            SecurityValidator.validateURI("ws://localhost:8080/path"));

        assertThatNoException().isThrownBy(() ->
            SecurityValidator.validateURI("wss://example.com:443/secure"));
    }

    @Test
    @DisplayName("Should reject null URI")
    void testNullURI() {
        assertThatThrownBy(() -> SecurityValidator.validateURI(null))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("cannot be null");
    }

    @Test
    @DisplayName("Should reject empty URI")
    void testEmptyURI() {
        assertThatThrownBy(() -> SecurityValidator.validateURI(""))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("cannot be null or empty");
    }

    @Test
    @DisplayName("Should reject invalid URI scheme")
    void testInvalidURIScheme() {
        assertThatThrownBy(() -> SecurityValidator.validateURI("http://localhost:8080"))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("must start with ws://");

        assertThatThrownBy(() -> SecurityValidator.validateURI("ftp://localhost:8080"))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("must start with ws://");
    }

    @Test
    @DisplayName("Should reject URI without host")
    void testURIWithoutHost() {
        assertThatThrownBy(() -> SecurityValidator.validateURI("ws://"))
            .isInstanceOf(ValidationException.class);
    }

    @Test
    @DisplayName("Should reject URI with invalid port")
    void testInvalidPort() {
        assertThatThrownBy(() -> SecurityValidator.validateURI("ws://localhost:99999/path"))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("Invalid port");
    }

    @Test
    @DisplayName("Should reject overly long URI")
    void testOversizedURI() {
        String longPath = "x".repeat(SecurityValidator.MAX_URI_LENGTH);
        String uri = "ws://localhost/" + longPath;

        assertThatThrownBy(() -> SecurityValidator.validateURI(uri))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("URI too long");
    }

    @Test
    @DisplayName("Should sanitize error messages")
    void testErrorMessageSanitization() {
        String withPath = "Error at C:\\Users\\Admin\\file.java line 42";
        String sanitized = SecurityValidator.sanitizeErrorMessage(withPath);

        assertThat(sanitized)
            .doesNotContain("C:\\Users")
            .doesNotContain("Admin")
            .contains("[PATH]");
    }

    @Test
    @DisplayName("Should sanitize IP addresses in errors")
    void testIPAddressSanitization() {
        String withIP = "Connection failed to 192.168.1.100";
        String sanitized = SecurityValidator.sanitizeErrorMessage(withIP);

        assertThat(sanitized)
            .doesNotContain("192.168.1.100")
            .contains("[IP]");
    }

    @Test
    @DisplayName("Should validate parameter ranges")
    void testRangeValidation() {
        assertThatNoException().isThrownBy(() ->
            SecurityValidator.validateRange(50, 0, 100, "testParam"));

        assertThatThrownBy(() ->
            SecurityValidator.validateRange(-1, 0, 100, "testParam"))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("must be between");

        assertThatThrownBy(() ->
            SecurityValidator.validateRange(101, 0, 100, "testParam"))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("must be between");
    }

    @Test
    @DisplayName("Should validate durations")
    void testDurationValidation() {
        assertThatNoException().isThrownBy(() ->
            SecurityValidator.validateDuration(1000, 100, "timeout"));

        assertThatThrownBy(() ->
            SecurityValidator.validateDuration(50, 100, "timeout"))
            .isInstanceOf(ValidationException.class)
            .hasMessageContaining("must be at least");
    }

    @Test
    @DisplayName("Should sanitize control characters from input")
    void testInputSanitization() {
        String withControlChars = "Hello\u0000\u0001\u001FWorld";
        String sanitized = SecurityValidator.sanitizeInput(withControlChars);

        assertThat(sanitized)
            .isEqualTo("HelloWorld")
            .doesNotContain("\u0000")
            .doesNotContain("\u0001");
    }

    @Test
    @DisplayName("Should handle null in sanitize input")
    void testSanitizeNullInput() {
        String sanitized = SecurityValidator.sanitizeInput(null);
        assertThat(sanitized).isEmpty();
    }

    @Test
    @DisplayName("Should handle null in sanitize error message")
    void testSanitizeNullErrorMessage() {
        String sanitized = SecurityValidator.sanitizeErrorMessage(null);
        assertThat(sanitized).isEqualTo("An error occurred");
    }

    @Test
    @DisplayName("Should truncate very long error messages")
    void testLongErrorMessageTruncation() {
        String longMessage = "x".repeat(300);
        String sanitized = SecurityValidator.sanitizeErrorMessage(longMessage);

        assertThat(sanitized).hasSizeLessThanOrEqualTo(200);
        assertThat(sanitized).endsWith("...");
    }
}

