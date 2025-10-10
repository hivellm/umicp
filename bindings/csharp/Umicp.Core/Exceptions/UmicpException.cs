namespace Umicp.Core.Exceptions;

/// <summary>
/// Base exception for UMICP errors
/// </summary>
public class UmicpException : Exception
{
    /// <summary>
    /// Error context information
    /// </summary>
    public Dictionary<string, object>? Context { get; set; }

    public UmicpException() : base() { }

    public UmicpException(string message) : base(message) { }

    public UmicpException(string message, Exception innerException)
        : base(message, innerException) { }

    public UmicpException(string message, Dictionary<string, object>? context)
        : base(message)
    {
        Context = context;
    }
}

/// <summary>
/// Exception thrown when validation fails
/// </summary>
public class ValidationException : UmicpException
{
    public ValidationException() : base() { }
    public ValidationException(string message) : base(message) { }
    public ValidationException(string message, Exception innerException)
        : base(message, innerException) { }
}

/// <summary>
/// Exception thrown when serialization/deserialization fails
/// </summary>
public class SerializationException : UmicpException
{
    public SerializationException() : base() { }
    public SerializationException(string message) : base(message) { }
    public SerializationException(string message, Exception innerException)
        : base(message, innerException) { }
}

/// <summary>
/// Exception thrown when transport operations fail
/// </summary>
public class TransportException : UmicpException
{
    public TransportException() : base() { }
    public TransportException(string message) : base(message) { }
    public TransportException(string message, Exception innerException)
        : base(message, innerException) { }
}

/// <summary>
/// Exception thrown when matrix operations fail
/// </summary>
public class MatrixOperationException : UmicpException
{
    public MatrixOperationException() : base() { }
    public MatrixOperationException(string message) : base(message) { }
    public MatrixOperationException(string message, Exception innerException)
        : base(message, innerException) { }
}

