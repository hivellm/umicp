/*!
# Advanced Matrix Operations Example

This example demonstrates advanced matrix operations and linear algebra
computations using the UMICP Rust bindings with high-performance implementations.
*/

use umicp_core::Matrix;
use std::time::Instant;

/// Advanced matrix operations demonstration
fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("🔢 UMICP Rust Bindings - Advanced Matrix Operations");
    println!("==================================================");

    let matrix = Matrix::new();

    // Example 1: Large Matrix Multiplication
    println!("\n1. Large Matrix Multiplication (Performance Test)");
    println!("------------------------------------------------");

    // Create large matrices for performance testing
    let m = 100;
    let n = 150;
    let p = 200;

    println!("📊 Matrix dimensions: ({})x({}) * ({})x({}) = ({})x({})",
             m, n, n, p, m, p);
    println!("💾 Memory usage: {:.2} MB",
             ((m*n + n*p + m*p) * 4) as f64 / (1024.0 * 1024.0));

    // Initialize matrices with test data
    let mut matrix_a = vec![0.0f32; m * n];
    let mut matrix_b = vec![0.0f32; n * p];
    let mut result = vec![0.0f32; m * p];

    // Fill with test data
    for i in 0..matrix_a.len() {
        matrix_a[i] = (i % 100) as f32 * 0.01;
    }
    for i in 0..matrix_b.len() {
        matrix_b[i] = ((i + 50) % 100) as f32 * 0.01;
    }

    println!("⏱️  Performing matrix multiplication...");

    let start_time = Instant::now();
    let multiply_result = matrix.multiply(&matrix_a, &matrix_b, &mut result, m, n, p);
    let elapsed = start_time.elapsed();

    match multiply_result {
        Ok(_) => {
            println!("✅ Matrix multiplication completed in {:?}", elapsed);
            println!("📈 Sample result[0][0]: {:.6}", result[0]);
            println!("🚀 Performance: {:.2} MFLOPS",
                     (2.0 * m as f64 * n as f64 * p as f64) / (elapsed.as_secs_f64() * 1_000_000.0));
        }
        Err(e) => println!("❌ Matrix multiplication failed: {:?}", e),
    }

    // Example 2: Matrix Inversion and Determinant
    println!("\n2. Matrix Inversion and Determinant");
    println!("-----------------------------------");

    // Test with 2x2 matrices (larger matrices not yet supported)
    let test_matrices = vec![
        vec![2.0, 1.0, 1.0, 3.0], // det = 5
        vec![4.0, 2.0, 2.0, 1.0], // det = 0 (singular)
        vec![1.0, 0.0, 0.0, 1.0], // identity matrix
    ];

    for (i, matrix_data) in test_matrices.iter().enumerate() {
        println!("📊 Matrix {}:", i + 1);
        print_matrix(matrix_data, 2, 2);

        // Calculate determinant
        let det_result = matrix.determinant(matrix_data, 2);
        match det_result {
            Ok(result) if result.success => {
                let det = result.result.unwrap_or(0.0);
                println!("🔢 Determinant: {:.6}", det);

                if det != 0.0 {
                    // Attempt matrix inversion
                    let mut inverse = vec![0.0f32; 4];
                    let inv_result = matrix.inverse(matrix_data, &mut inverse, 2);

                    match inv_result {
                        Ok(result) if result.success => {
                            println!("🔄 Matrix inversion successful:");
                            print_matrix(&inverse, 2, 2);

                            // Verify inverse by multiplication
                            let mut verification = vec![0.0f32; 4];
                            matrix.multiply(matrix_data, &inverse, &mut verification, 2, 2, 2)?;

                            println!("✅ Verification (A * A^-1 should be identity):");
                            print_matrix(&verification, 2, 2);
                        }
                        _ => println!("❌ Matrix inversion failed"),
                    }
                } else {
                    println!("🚫 Matrix is singular (determinant = 0), cannot invert");
                }
            }
            _ => println!("❌ Determinant calculation failed"),
        }
        println!();
    }

    // Example 3: Vector Operations Showcase
    println!("3. Advanced Vector Operations");
    println!("----------------------------");

    // Large vectors for SIMD demonstration
    let size = 1024;
    let mut vec1 = vec![0.0f32; size];
    let mut vec2 = vec![0.0f32; size];
    let mut result_vec = vec![0.0f32; size];

    // Initialize with sine/cosine waves
    for i in 0..size {
        let t = i as f32 * 0.01;
        vec1[i] = t.sin();
        vec2[i] = t.cos();
    }

    println!("📊 Vector operations on {} elements", size);

    // Vector addition
    let add_start = Instant::now();
    matrix.vector_add(&vec1, &vec2, &mut result_vec)?;
    let add_time = add_start.elapsed();
    println!("➕ Vector addition: {:.3}ms ({:.0} MOPS)",
             add_time.as_secs_f64() * 1000.0,
             size as f64 / add_time.as_secs_f64() / 1_000_000.0);

    // Dot product (with SIMD optimization)
    let dot_start = Instant::now();
    let dot_result = matrix.dot_product(&vec1, &vec2)?;
    let dot_time = dot_start.elapsed();

    if let Some(dot_value) = dot_result.result {
        println!("🔹 Dot product: {:.6} ({:.3}ms)",
                 dot_value,
                 dot_time.as_secs_f64() * 1000.0);
    }

    // Cosine similarity
    let cos_result = matrix.cosine_similarity(&vec1, &vec2)?;
    if let Some(cos_value) = cos_result.similarity {
        println!("📐 Cosine similarity: {:.6}", cos_value);
    }

    // Vector scaling
    let scale_start = Instant::now();
    let scalar = 2.5;
    matrix.vector_scale(&vec1, scalar, &mut result_vec)?;
    let scale_time = scale_start.elapsed();
    println!("✖️  Vector scaling by {:.1}: {:.3}ms", scalar, scale_time.as_secs_f64() * 1000.0);

    // Example 4: Matrix Transpose Performance
    println!("\n4. Matrix Transpose Performance");
    println!("-------------------------------");

    let transpose_sizes = vec![64, 128, 256];

    for &size in &transpose_sizes {
        let mut matrix_data = vec![0.0f32; size * size];
        let mut transposed = vec![0.0f32; size * size];

        // Initialize with pattern
        for i in 0..size {
            for j in 0..size {
                matrix_data[i * size + j] = (i + j) as f32;
            }
        }

        let transpose_start = Instant::now();
        matrix.transpose(&matrix_data, &mut transposed, size, size)?;
        let transpose_time = transpose_start.elapsed();

        println!("🔄 Matrix transpose {}x{}: {:.3}ms ({:.2} MB/s)",
                 size, size,
                 transpose_time.as_secs_f64() * 1000.0,
                 (size * size * 4 * 2) as f64 / transpose_time.as_secs_f64() / (1024.0 * 1024.0));
    }

    // Example 5: Error Handling Demonstration
    println!("\n5. Error Handling Examples");
    println!("--------------------------");

    // Test with invalid dimensions
    let invalid_result = matrix.multiply(&vec![1.0, 2.0], &vec![3.0, 4.0], &mut vec![0.0], 2, 3, 2);
    match invalid_result {
        Err(e) => println!("✅ Expected error caught: {:?}", e),
        Ok(_) => println!("❌ Expected error was not caught"),
    }

    // Test with mismatched vector sizes
    let mismatch_result = matrix.vector_add(&vec![1.0, 2.0], &vec![3.0], &mut vec![0.0, 0.0]);
    match mismatch_result {
        Err(e) => println!("✅ Vector size mismatch error caught: {:?}", e),
        Ok(_) => println!("❌ Expected vector mismatch error was not caught"),
    }

    // Example 6: Memory Efficiency Demonstration
    println!("\n6. Memory Efficiency Analysis");
    println!("-----------------------------");

    // Demonstrate zero-copy operations where possible
    let data_size = 1000000; // 1M elements
    let mut large_vector = vec![1.0f32; data_size];

    let mem_start = Instant::now();
    let normalized = matrix.normalize(&mut large_vector, 1, data_size)?;
    let mem_time = mem_start.elapsed();

    if let Some(normalized_data) = normalized.data {
        println!("📊 Normalized {} elements in {:.3}ms", data_size, mem_time.as_secs_f64() * 1000.0);
        println!("💾 Memory efficient: No unnecessary copies");
        println!("📏 First normalized value: {:.6}", normalized_data[0]);
        println!("📏 Last normalized value: {:.6}", normalized_data[normalized_data.len() - 1]);
    }

    println!("\n🎉 Advanced Matrix Operations Example Completed!");
    println!("==================================================");
    println!();
    println!("💡 Performance Highlights:");
    println!("   • SIMD-accelerated vector operations");
    println!("   • Efficient matrix multiplication algorithms");
    println!("   • Memory-optimized data structures");
    println!("   • Comprehensive error handling");
    println!("   • Zero-copy operations where possible");

    Ok(())
}

/// Helper function to print matrices in a readable format
fn print_matrix(matrix: &[f32], rows: usize, cols: usize) {
    for i in 0..rows {
        print!("     ");
        for j in 0..cols {
            print!("{:8.3} ", matrix[i * cols + j]);
        }
        println!();
    }
}
