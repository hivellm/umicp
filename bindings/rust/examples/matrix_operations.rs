/*!
# Matrix Operations Example

This example demonstrates the high-performance matrix operations available
in the UMICP Rust bindings, including basic arithmetic, linear algebra,
and similarity calculations.
*/

use umicp_core::Matrix;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("UMICP Rust Bindings - Matrix Operations Example");
    println!("===============================================\n");

    let matrix = Matrix::new();

    // Vector addition
    println!("1. Vector Addition");
    println!("-----------------");
    let vector1 = vec![1.0, 2.0, 3.0, 4.0];
    let vector2 = vec![5.0, 6.0, 7.0, 8.0];
    let mut result = vec![0.0; 4];

    println!("   Vector 1: {:?}", vector1);
    println!("   Vector 2: {:?}", vector2);

    matrix.vector_add(&vector1, &vector2, &mut result)?;
    println!("   Sum: {:?}", result);
    println!();

    // Dot product
    println!("2. Dot Product");
    println!("-------------");
    let dot_result = matrix.dot_product(&vector1, &vector2)?;
    println!("   Vector 1: {:?}", vector1);
    println!("   Vector 2: {:?}", vector2);
    println!("   Dot product: {}", dot_result.result.unwrap());
    println!();

    // Vector scaling
    println!("3. Vector Scaling");
    println!("-----------------");
    let vector = vec![1.0, 2.0, 3.0, 4.0];
    let mut scaled_result = vec![0.0; 4];
    let scalar = 3.0;

    println!("   Original vector: {:?}", vector);
    println!("   Scalar: {}", scalar);

    matrix.vector_scale(&vector, scalar, &mut scaled_result)?;
    println!("   Scaled vector: {:?}", scaled_result);
    println!();

    // Matrix multiplication
    println!("4. Matrix Multiplication");
    println!("-----------------------");
    // 2x3 matrix * 3x2 matrix = 2x2 matrix
    let matrix_a = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0]; // 2x3
    let matrix_b = vec![7.0, 8.0, 9.0, 10.0, 11.0, 12.0]; // 3x2
    let mut matrix_result = vec![0.0; 4]; // 2x2

    println!("   Matrix A (2x3):");
    print_matrix(&matrix_a, 2, 3);
    println!("   Matrix B (3x2):");
    print_matrix(&matrix_b, 3, 2);

    matrix.multiply(&matrix_a, &matrix_b, &mut matrix_result, 2, 3, 2)?;
    println!("   Result (2x2):");
    print_matrix(&matrix_result, 2, 2);
    println!();

    // Cosine similarity
    println!("5. Cosine Similarity");
    println!("-------------------");
    let vec_a = vec![1.0, 2.0, 3.0];
    let vec_b = vec![1.0, 2.0, 3.0]; // identical
    let vec_c = vec![1.0, 0.0, 0.0]; // orthogonal to vec_a

    println!("   Vector A: {:?}", vec_a);
    println!("   Vector B (identical): {:?}", vec_b);

    let similarity_ab = matrix.cosine_similarity(&vec_a, &vec_b)?;
    println!("   Similarity A·B: {:.6}", similarity_ab.similarity.unwrap());

    println!("   Vector A: {:?}", vec_a);
    println!("   Vector C (orthogonal): {:?}", vec_c);

    let similarity_ac = matrix.cosine_similarity(&vec_a, &vec_c)?;
    println!("   Similarity A·C: {:.6}", similarity_ac.similarity.unwrap());
    println!();

    // Matrix transpose
    println!("6. Matrix Transpose");
    println!("------------------");
    let original = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0]; // 2x3
    let mut transposed = vec![0.0; 6]; // 3x2

    println!("   Original matrix (2x3):");
    print_matrix(&original, 2, 3);

    matrix.transpose(&original, &mut transposed, 2, 3)?;
    println!("   Transposed matrix (3x2):");
    print_matrix(&transposed, 3, 2);
    println!();

    // Determinant
    println!("7. Matrix Determinant");
    println!("--------------------");
    let square_matrix = vec![1.0, 2.0, 3.0, 4.0]; // 2x2 matrix

    println!("   Matrix (2x2):");
    print_matrix(&square_matrix, 2, 2);

    let det_result = matrix.determinant(&square_matrix, 2)?;
    println!("   Determinant: {}", det_result.result.unwrap());
    println!();

    // Vector normalization
    println!("8. Vector Normalization (L2)");
    println!("----------------------------");
    let unnormalized = vec![3.0, 4.0]; // Should normalize to [0.6, 0.8]
    let mut normalized = vec![0.0; 2];

    println!("   Original vector: {:?}", unnormalized);
    println!("   Magnitude: {}", (9.0_f32 + 16.0_f32).sqrt());

    matrix.vector_scale(&unnormalized, 1.0_f32 / (9.0_f32 + 16.0_f32).sqrt(), &mut normalized)?;
    println!("   Normalized vector: {:?}", normalized);
    println!("   Verification - magnitude: {:.6}", (normalized[0]*normalized[0] + normalized[1]*normalized[1]).sqrt() as f64);

    println!("\n✓ Matrix operations example completed successfully!");

    Ok(())
}

fn print_matrix(matrix: &[f32], rows: usize, cols: usize) {
    for i in 0..rows {
        print!("     ");
        for j in 0..cols {
            print!("{:6.2} ", matrix[i * cols + j]);
        }
        println!();
    }
}
