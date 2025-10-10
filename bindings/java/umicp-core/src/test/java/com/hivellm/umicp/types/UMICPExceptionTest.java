package com.hivellm.umicp.types;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Test class for UMICPException and its hierarchy.
 */
public class UMICPExceptionTest {

    @Test
    public void testUMICPException_NoArgs() {
        UMICPException exception = new UMICPException();
        assertNotNull(exception);
        assertNull(exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void testUMICPException_WithMessage() {
        String message = "Test error message";
        UMICPException exception = new UMICPException(message);

        assertNotNull(exception);
        assertEquals(message, exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void testUMICPException_WithMessageAndCause() {
        String message = "Test error";
        Throwable cause = new RuntimeException("Original cause");
        UMICPException exception = new UMICPException(message, cause);

        assertNotNull(exception);
        assertEquals(message, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testUMICPException_WithCause() {
        Throwable cause = new IllegalArgumentException("Invalid argument");
        UMICPException exception = new UMICPException(cause);

        assertNotNull(exception);
        assertEquals(cause, exception.getCause());
        assertTrue(exception.getMessage().contains("IllegalArgumentException"));
    }

    @Test
    public void testValidationException_NoArgs() {
        ValidationException exception = new ValidationException();
        assertNotNull(exception);
        assertNull(exception.getMessage());
        assertTrue(exception instanceof UMICPException);
    }

    @Test
    public void testValidationException_WithMessage() {
        String message = "Validation failed";
        ValidationException exception = new ValidationException(message);

        assertEquals(message, exception.getMessage());
        assertTrue(exception instanceof UMICPException);
    }

    @Test
    public void testValidationException_WithMessageAndCause() {
        String message = "Field validation error";
        Throwable cause = new IllegalStateException("Invalid state");
        ValidationException exception = new ValidationException(message, cause);

        assertEquals(message, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testValidationException_WithCause() {
        Throwable cause = new NullPointerException("Required field is null");
        ValidationException exception = new ValidationException(cause);

        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testSerializationException_NoArgs() {
        SerializationException exception = new SerializationException();
        assertNotNull(exception);
        assertTrue(exception instanceof UMICPException);
    }

    @Test
    public void testSerializationException_WithMessage() {
        String message = "Failed to serialize envelope";
        SerializationException exception = new SerializationException(message);

        assertEquals(message, exception.getMessage());
    }

    @Test
    public void testSerializationException_WithMessageAndCause() {
        String message = "JSON serialization failed";
        Throwable cause = new RuntimeException("Jackson error");
        SerializationException exception = new SerializationException(message, cause);

        assertEquals(message, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testSerializationException_WithCause() {
        Throwable cause = new IllegalArgumentException("Invalid JSON");
        SerializationException exception = new SerializationException(cause);

        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testTransportException_NoArgs() {
        TransportException exception = new TransportException();
        assertNotNull(exception);
        assertTrue(exception instanceof UMICPException);
    }

    @Test
    public void testTransportException_WithMessage() {
        String message = "Transport layer error";
        TransportException exception = new TransportException(message);

        assertEquals(message, exception.getMessage());
    }

    @Test
    public void testTransportException_WithMessageAndCause() {
        String message = "Connection failed";
        Throwable cause = new java.io.IOException("Network error");
        TransportException exception = new TransportException(message, cause);

        assertEquals(message, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testTransportException_WithCause() {
        Throwable cause = new java.net.SocketException("Connection reset");
        TransportException exception = new TransportException(cause);

        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testConnectionException_NoArgs() {
        ConnectionException exception = new ConnectionException();
        assertNotNull(exception);
        assertTrue(exception instanceof TransportException);
        assertTrue(exception instanceof UMICPException);
    }

    @Test
    public void testConnectionException_WithMessage() {
        String message = "Failed to connect to peer";
        ConnectionException exception = new ConnectionException(message);

        assertEquals(message, exception.getMessage());
    }

    @Test
    public void testConnectionException_WithMessageAndCause() {
        String message = "Connection timeout";
        Throwable cause = new java.net.SocketTimeoutException("Timeout after 30s");
        ConnectionException exception = new ConnectionException(message, cause);

        assertEquals(message, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testConnectionException_WithCause() {
        Throwable cause = new java.net.ConnectException("Connection refused");
        ConnectionException exception = new ConnectionException(cause);

        assertEquals(cause, exception.getCause());
    }

    @Test
    public void testExceptionHierarchy() {
        // Test inheritance chain
        ConnectionException connEx = new ConnectionException("test");
        assertTrue(connEx instanceof TransportException);
        assertTrue(connEx instanceof UMICPException);
        assertTrue(connEx instanceof Exception);

        ValidationException valEx = new ValidationException("test");
        assertTrue(valEx instanceof UMICPException);
        assertTrue(valEx instanceof Exception);

        SerializationException serEx = new SerializationException("test");
        assertTrue(serEx instanceof UMICPException);
        assertTrue(serEx instanceof Exception);
    }

    @Test
    public void testExceptionCatching() {
        // Test that we can catch all UMICP exceptions with base class
        try {
            throw new ValidationException("Validation error");
        } catch (UMICPException e) {
            assertEquals("Validation error", e.getMessage());
            assertTrue(e instanceof ValidationException);
        }

        try {
            throw new ConnectionException("Connection error");
        } catch (UMICPException e) {
            assertEquals("Connection error", e.getMessage());
            assertTrue(e instanceof ConnectionException);
        }
    }

    @Test
    public void testStackTrace() {
        UMICPException exception = new UMICPException("Test error");
        StackTraceElement[] stackTrace = exception.getStackTrace();

        assertNotNull(stackTrace);
        assertTrue(stackTrace.length > 0);
        assertTrue(stackTrace[0].getClassName().contains("UMICPExceptionTest"));
    }

    @Test
    public void testExceptionSerialization() {
        // Test that exceptions can be properly serialized for logging
        ValidationException exception = new ValidationException(
            "Field 'from' is required",
            new NullPointerException("from field is null")
        );

        String message = exception.toString();
        assertNotNull(message);
        assertTrue(message.contains("ValidationException"));
    }

    @Test
    public void testNestedExceptions() {
        // Test nested exception chain
        NullPointerException root = new NullPointerException("Root cause");
        IllegalArgumentException middle = new IllegalArgumentException("Middle", root);
        UMICPException top = new UMICPException("Top level", middle);

        assertEquals("Top level", top.getMessage());
        assertEquals(middle, top.getCause());
        assertEquals(root, top.getCause().getCause());
    }
}

