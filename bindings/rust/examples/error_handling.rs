/*!
# Error Handling Example

This example demonstrates comprehensive error handling patterns and recovery
strategies using the UMICP Rust bindings with proper error propagation.
*/

use umicp_core::{Envelope, Matrix, OperationType, UmicpError};

/// Comprehensive error handling demonstration
fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("🛡️  UMICP Rust Bindings - Error Handling Example");
    println!("===============================================");

    // Example 1: Basic Error Handling
    println!("\n1. Basic Error Handling");
    println!("-----------------------");

    demonstrate_basic_errors()?;

    // Example 2: Error Recovery Strategies
    println!("\n2. Error Recovery Strategies");
    println!("----------------------------");

    demonstrate_error_recovery()?;

    // Example 3: Custom Error Types
    println!("\n3. Custom Error Types and Context");
    println!("----------------------------------");

    demonstrate_custom_errors()?;

    // Example 4: Validation and Sanitization
    println!("\n4. Input Validation and Sanitization");
    println!("------------------------------------");

    demonstrate_validation()?;

    // Example 5: Graceful Degradation
    println!("\n5. Graceful Degradation");
    println!("-----------------------");

    demonstrate_graceful_degradation();

    // Example 6: Error Aggregation
    println!("\n6. Error Aggregation and Reporting");
    println!("-----------------------------------");

    demonstrate_error_aggregation()?;

    println!("\n🎉 Error Handling Example Completed!");
    println!("=====================================");
    println!();
    println!("💡 Error Handling Best Practices:");
    println!("   • Always use Result types for operations that can fail");
    println!("   • Provide meaningful error messages with context");
    println!("   • Implement proper error recovery strategies");
    println!("   • Use custom error types for domain-specific errors");
    println!("   • Validate input data before processing");
    println!("   • Log errors appropriately for debugging");

    Ok(())
}

/// Demonstrate basic error handling patterns
fn demonstrate_basic_errors() -> Result<(), Box<dyn std::error::Error>> {
    let matrix = Matrix::new();

    // Test with invalid matrix dimensions
    println!("📊 Testing matrix operations with invalid inputs...");

    let invalid_result = matrix.multiply(
        &vec![1.0, 2.0],
        &vec![3.0, 4.0],
        &mut vec![0.0],
        2, 3, 2  // Invalid dimensions: 2x3 * 3x2 = 2x2, but we provided 2x2 result
    );

    match invalid_result {
        Ok(_) => println!("❌ Unexpected success with invalid dimensions"),
        Err(UmicpError::Matrix { message: msg }) => {
            println!("✅ Matrix dimension error caught: {}", msg);
        }
        Err(e) => println!("✅ Other error caught: {:?}", e),
    }

    // Test vector operations with mismatched sizes
    let vec_mismatch = matrix.vector_add(
        &vec![1.0, 2.0, 3.0],
        &vec![4.0, 5.0],
        &mut vec![0.0, 0.0, 0.0]
    );

    match vec_mismatch {
        Ok(_) => println!("❌ Unexpected success with mismatched vector sizes"),
        Err(UmicpError::Matrix { message: msg }) => {
            println!("✅ Vector size mismatch error caught: {}", msg);
        }
        Err(e) => println!("✅ Other error caught: {:?}", e),
    }

    Ok(())
}

/// Demonstrate error recovery strategies
fn demonstrate_error_recovery() -> Result<(), Box<dyn std::error::Error>> {
    let matrix = Matrix::new();

    println!("🔄 Testing error recovery strategies...");

    // Strategy 1: Retry with exponential backoff
    let mut attempt = 0;
    let max_attempts = 3;

    loop {
        attempt += 1;
        println!("   Attempt {} of {}", attempt, max_attempts);

        // Simulate an operation that might fail
        let result = perform_unreliable_operation(&matrix, attempt);

        match result {
            Ok(value) => {
                println!("✅ Operation succeeded on attempt {}: {}", attempt, value);
                break;
            }
            Err(e) if attempt < max_attempts => {
                println!("⚠️  Attempt {} failed: {:?}", attempt, e);
                println!("   Retrying in {}ms...", attempt * 100);

                // Exponential backoff
                std::thread::sleep(std::time::Duration::from_millis((attempt * 100) as u64));
            }
            Err(e) => {
                println!("❌ Operation failed after {} attempts: {:?}", max_attempts, e);
                return Err(e.into());
            }
        }
    }

    // Strategy 2: Fallback to simpler operation
    println!("\n🔄 Testing fallback strategy...");

    let complex_result = perform_complex_operation(&matrix);
    match complex_result {
        Ok(value) => println!("✅ Complex operation succeeded: {}", value),
        Err(_) => {
            println!("⚠️  Complex operation failed, falling back to simple operation...");
            let simple_result = perform_simple_operation(&matrix)?;
            println!("✅ Fallback operation succeeded: {}", simple_result);
        }
    }

    Ok(())
}

/// Demonstrate custom error types and context
fn demonstrate_custom_errors() -> Result<(), Box<dyn std::error::Error>> {
    println!("🏷️  Testing custom error types...");

    // Test envelope creation with invalid data
    let invalid_envelope = create_envelope_with_invalid_data();
    match invalid_envelope {
        Ok(_) => println!("❌ Unexpected success with invalid data"),
        Err(e) => {
            println!("✅ Custom validation error caught: {:?}", e);
            println!("   Error context: This helps with debugging and user feedback");
        }
    }

    // Test with corrupted data
    let corrupted_result = process_corrupted_data();
    match corrupted_result {
        Ok(_) => println!("❌ Unexpected success with corrupted data"),
        Err(CorruptionError::DataIntegrity(msg)) => {
            println!("✅ Data integrity error caught: {}", msg);
        }
        Err(CorruptionError::FormatError(msg)) => {
            println!("✅ Format error caught: {}", msg);
        }
        Err(e) => println!("✅ Other error caught: {:?}", e),
    }

    Ok(())
}

/// Demonstrate input validation and sanitization
fn demonstrate_validation() -> Result<(), Box<dyn std::error::Error>> {
    println!("🔍 Testing input validation and sanitization...");

    // Test envelope validation
    let test_cases = vec![
        ("", "receiver", "Invalid sender"),
        ("sender", "", "Invalid receiver"),
        ("sender", "receiver", "Valid data"),
    ];

    for (from, to, expected) in test_cases {
        let envelope = Envelope::builder()
            .from(from)
            .to(to)
            .operation(OperationType::Data)
            .message_id(&format!("test-{}", chrono::Utc::now().timestamp()))
            .build();

        match envelope {
            Ok(env) => {
                let is_valid = validate_envelope(&env)?;
                println!("✅ {}: Envelope created and validated", expected);
            }
            Err(e) => {
                println!("✅ {}: Error caught - {:?}", expected, e);
            }
        }
    }

    // Test data sanitization
    let unsafe_data = vec![
        "safe_data",
        "data_with_null\x00character",
        "data_with_newline\ncharacter",
        "normal_data"
    ];

    println!("\n🧹 Testing data sanitization...");
    for data in unsafe_data {
        let sanitized = sanitize_input(data);
        println!("   '{}' -> '{}'", data, sanitized);
    }

    Ok(())
}

/// Demonstrate graceful degradation
fn demonstrate_graceful_degradation() {
    println!("📉 Testing graceful degradation...");

    let matrix = Matrix::new();

    // Test with increasingly large matrices
    let sizes = vec![10, 50, 100, 500, 1000];

    for size in sizes {
        let start_time = std::time::Instant::now();

        // Create test data
        let mut matrix_data = vec![0.0f32; size * size];
        for i in 0..matrix_data.len() {
            matrix_data[i] = (i % 100) as f32 * 0.01;
        }

        // Try matrix multiplication
        let mut result = vec![0.0f32; size * size];
        let multiply_result = matrix.multiply(&matrix_data, &matrix_data, &mut result, size, size, size);

        let elapsed = start_time.elapsed();

        match multiply_result {
            Ok(_) => {
                println!("✅ Matrix {}x{}: {:.3}ms", size, size, elapsed.as_secs_f64() * 1000.0);
            }
            Err(UmicpError::Matrix { message: msg }) if msg.contains("too large") => {
                println!("⚠️  Matrix {}x{} too large, skipping: {}", size, size, msg);
                continue;
            }
            Err(e) => {
                println!("❌ Matrix {}x{} failed: {:?}", size, size, e);
                break;
            }
        }
    }

    println!("📊 Graceful degradation maintained system stability");
}

/// Demonstrate error aggregation and reporting
fn demonstrate_error_aggregation() -> Result<(), Box<dyn std::error::Error>> {
    println!("📊 Testing error aggregation and reporting...");

    let operations: Vec<(&str, fn() -> Result<(), UmicpError>)> = vec![
        ("matrix_multiply", perform_matrix_operation as fn() -> Result<(), UmicpError>),
        ("vector_add", perform_vector_operation as fn() -> Result<(), UmicpError>),
        ("envelope_create", perform_envelope_operation as fn() -> Result<(), UmicpError>),
        ("dot_product", perform_dot_product_operation as fn() -> Result<(), UmicpError>),
    ];

    let mut errors = Vec::new();
    let mut successes = 0;

    for &(name, operation) in &operations {
        match operation() {
            Ok(_) => {
                successes += 1;
                println!("✅ {}: Success", name);
            }
            Err(e) => {
                let error_msg = format!("{:?}", e);
                errors.push((name.to_string(), e));
                println!("❌ {}: Failed - {}", name, error_msg);
            }
        }
    }

    // Generate error report
    if !errors.is_empty() {
        println!("\n📋 Error Report:");
        println!("===============");
        println!("Total operations: {}", operations.len());
        println!("Successful: {}", successes);
        println!("Failed: {}", errors.len());
        println!("Success rate: {:.1}%", (successes as f32 / operations.len() as f32) * 100.0);

        println!("\nError Details:");
        for (name, error) in &errors {
            println!("• {}: {:?}", name, error);
        }

        // In a real application, this might be sent to monitoring/logging system
        println!("\n🚨 Error report would be sent to monitoring system");
    }

    Ok(())
}

// Helper functions for error simulation

fn perform_unreliable_operation(matrix: &Matrix, attempt: usize) -> Result<f32, UmicpError> {
    // Simulate random failures
    if attempt < 3 && rand::random::<f32>() < 0.7 {
        return Err(UmicpError::generic(format!("Simulated failure on attempt {}", attempt)));
    }

    // Perform actual operation
    let a = vec![1.0, 2.0, 3.0];
    let b = vec![4.0, 5.0, 6.0];
    let result = matrix.dot_product(&a, &b)?;

    result.result.ok_or_else(|| UmicpError::generic("No result from dot product".to_string())).map(|v| v as f32)
}

fn perform_complex_operation(matrix: &Matrix) -> Result<f32, UmicpError> {
    // Simulate a complex operation that might fail
    if rand::random::<f32>() < 0.6 {
        return Err(UmicpError::generic("Complex operation failed".to_string()));
    }

    let size = 100;
    let mut a = vec![0.0f32; size];
    let mut b = vec![0.0f32; size];
    let mut result = vec![0.0f32; size];

    for i in 0..size {
        a[i] = (i as f32).sin();
        b[i] = (i as f32).cos();
    }

    matrix.vector_add(&a, &b, &mut result)?;
    Ok(result.iter().sum())
}

fn perform_simple_operation(matrix: &Matrix) -> Result<f32, UmicpError> {
    let a = vec![1.0, 2.0, 3.0];
    let b = vec![4.0, 5.0, 6.0];
    let result = matrix.dot_product(&a, &b)?;

    result.result.ok_or_else(|| UmicpError::generic("No result from simple operation".to_string())).map(|v| v as f32)
}

fn create_envelope_with_invalid_data() -> Result<Envelope, UmicpError> {
    Envelope::builder()
        .from("")  // Invalid: empty sender
        .to("receiver")
        .operation(OperationType::Data)
        .message_id("invalid-envelope")
        .build()
}

#[derive(Debug)]
enum CorruptionError {
    DataIntegrity(String),
    FormatError(String),
}

fn process_corrupted_data() -> Result<(), CorruptionError> {
    // Simulate corrupted data detection
    let corrupted = rand::random::<f32>() < 0.5;

    if corrupted {
        if rand::random::<bool>() {
            Err(CorruptionError::DataIntegrity("Data integrity check failed".to_string()))
        } else {
            Err(CorruptionError::FormatError("Invalid data format detected".to_string()))
        }
    } else {
        Ok(())
    }
}

fn validate_envelope(envelope: &Envelope) -> Result<bool, UmicpError> {
    // Custom validation logic
    if envelope.from().is_empty() || envelope.to().is_empty() {
        return Err(UmicpError::validation("Envelope missing required fields".to_string()));
    }

    // Additional validation could be added here
    Ok(true)
}

fn sanitize_input(input: &str) -> String {
    input.chars()
        .filter(|c| c.is_alphanumeric() || c.is_whitespace() || *c == '-' || *c == '_')
        .collect()
}

fn perform_matrix_operation() -> Result<(), UmicpError> {
    let matrix = Matrix::new();
    let a = vec![1.0, 2.0, 3.0, 4.0];
    let b = vec![5.0, 6.0, 7.0, 8.0];
    let mut result = vec![0.0; 4];

    matrix.multiply(&a, &b, &mut result, 2, 2, 2)?;
    Ok(())
}

fn perform_vector_operation() -> Result<(), UmicpError> {
    let matrix = Matrix::new();
    let a = vec![1.0, 2.0, 3.0];
    let b = vec![4.0, 5.0, 6.0];
    let mut result = vec![0.0; 3];

    matrix.vector_add(&a, &b, &mut result)?;
    Ok(())
}

fn perform_envelope_operation() -> Result<(), UmicpError> {
    Envelope::builder()
        .from("test-sender")
        .to("test-receiver")
        .operation(OperationType::Data)
        .message_id("test-envelope")
        .build()?;
    Ok(())
}

fn perform_dot_product_operation() -> Result<(), UmicpError> {
    let matrix = Matrix::new();
    let a = vec![1.0, 2.0, 3.0];
    let b = vec![4.0, 5.0, 6.0];

    matrix.dot_product(&a, &b)?;
    Ok(())
}
