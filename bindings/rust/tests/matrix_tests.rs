/*!
# Tests for UMICP Matrix operations
*/

use umicp_core::Matrix;

#[test]
fn test_create_matrix_instance() {
    let matrix = Matrix::new();
    // Just verify it creates successfully
    drop(matrix);
}

#[test]
fn test_vector_addition() {
    let matrix = Matrix::new();
    let size = 100;

    let mut a = vec![0.0f32; size];
    let mut b = vec![0.0f32; size];
    let mut result = vec![0.0f32; size];

    // Initialize test data
    for i in 0..size {
        a[i] = i as f32 * 1.0;
        b[i] = i as f32 * 2.0;
    }

    let operation_result = matrix.vector_add(&a, &b, &mut result)
        .expect("Addition failed");

    assert!(operation_result.success);

    // Verify results
    for i in 0..size {
        assert!((result[i] - (a[i] + b[i])).abs() < 0.00001);
    }
}

#[test]
fn test_matrix_multiplication() {
    let matrix = Matrix::new();
    let m = 4;
    let n = 6;
    let p = 8;

    let mut a = vec![0.0f32; m * n];
    let mut b = vec![0.0f32; n * p];
    let mut result = vec![0.0f32; m * p];

    // Initialize with simple values for predictable results
    for i in 0..a.len() {
        a[i] = (i + 1) as f32;
    }
    for i in 0..b.len() {
        b[i] = (i + 1) as f32;
    }

    let operation_result = matrix.multiply(&a, &b, &mut result, m, n, p)
        .expect("Multiplication failed");

    assert!(operation_result.success);
    assert_eq!(result.len(), m * p);
    assert!(result.iter().all(|&val| !val.is_nan()));
}

#[test]
fn test_matrix_transpose() {
    let matrix = Matrix::new();
    let rows = 3;
    let cols = 4;

    let mut input = vec![0.0f32; rows * cols];
    let mut output = vec![0.0f32; cols * rows];

    // Initialize with sequential values
    for i in 0..input.len() {
        input[i] = (i + 1) as f32;
    }

    let operation_result = matrix.transpose(&input, &mut output, rows, cols)
        .expect("Transpose failed");

    assert!(operation_result.success);

    // Verify transpose
    for i in 0..rows {
        for j in 0..cols {
            assert_eq!(output[j * rows + i], input[i * cols + j]);
        }
    }
}

#[test]
fn test_dot_product() {
    let matrix = Matrix::new();
    let size = 10;

    let mut a = vec![0.0f32; size];
    let mut b = vec![0.0f32; size];

    // Initialize with known values
    for i in 0..size {
        a[i] = (i + 1) as f32; // [1, 2, 3, ..., 10]
        b[i] = (i + 1) as f32; // [1, 2, 3, ..., 10]
    }

    let operation_result = matrix.dot_product(&a, &b)
        .expect("Dot product failed");

    assert!(operation_result.success);
    assert!(operation_result.result.is_some());

    // Expected result: sum of squares from 1 to 10 = (10 * 11 * 21) / 6 = 385
    let expected = (size * (size + 1) * (2 * size + 1)) as f64 / 6.0;
    let result = operation_result.result.unwrap();
    assert!((result - expected).abs() < 0.01);
}

#[test]
fn test_normalize_vector() {
    let matrix = Matrix::new();
    let size = 4;

    let mut vector = vec![3.0f32, 4.0, 0.0, 0.0];

    let operation_result = matrix.normalize(&mut vector, 1, size)
        .expect("Normalization failed");

    assert!(operation_result.success);

    // Calculate magnitude
    let magnitude: f32 = vector.iter().map(|&x| x * x).sum::<f32>().sqrt();

    // Should be unit length (magnitude = 1.0)
    assert!((magnitude - 1.0).abs() < 0.00001);
}

#[test]
fn test_cosine_similarity() {
    let matrix = Matrix::new();

    // Identical vectors should have similarity = 1.0
    let a = vec![1.0f32, 2.0, 3.0];
    let b = vec![1.0f32, 2.0, 3.0];

    let operation_result = matrix.cosine_similarity(&a, &b)
        .expect("Cosine similarity failed");

    assert!(operation_result.success);
    assert!(operation_result.similarity.is_some());

    let similarity = operation_result.similarity.unwrap();
    assert!((similarity - 1.0).abs() < 0.00001);
}

#[test]
fn test_cosine_similarity_orthogonal() {
    let matrix = Matrix::new();

    // Orthogonal vectors should have similarity = 0.0
    let a = vec![1.0f32, 0.0, 0.0];
    let b = vec![0.0f32, 1.0, 0.0];

    let operation_result = matrix.cosine_similarity(&a, &b)
        .expect("Cosine similarity failed");

    assert!(operation_result.success);
    assert!(operation_result.similarity.is_some());

    let similarity = operation_result.similarity.unwrap();
    assert!(similarity.abs() < 0.00001);
}

#[test]
fn test_vector_operations() {
    let matrix = Matrix::new();
    let size = 10;

    let mut a = vec![0.0f32; size];
    let mut b = vec![0.0f32; size];
    let mut result = vec![0.0f32; size];

    for i in 0..size {
        a[i] = (i + 1) as f32;
        b[i] = (i + 2) as f32;
    }

    // Vector addition
    let add_result = matrix.vector_add(&a, &b, &mut result)
        .expect("Vector addition failed");
    assert!(add_result.success);

    for i in 0..size {
        assert!((result[i] - (a[i] + b[i])).abs() < 0.00001);
    }

    // Vector subtraction
    result.fill(0.0);
    let sub_result = matrix.vector_subtract(&a, &b, &mut result)
        .expect("Vector subtraction failed");
    assert!(sub_result.success);

    for i in 0..size {
        assert!((result[i] - (a[i] - b[i])).abs() < 0.00001);
    }

    // Vector multiplication (element-wise)
    result.fill(0.0);
    let mul_result = matrix.vector_multiply(&a, &b, &mut result)
        .expect("Vector multiplication failed");
    assert!(mul_result.success);

    for i in 0..size {
        assert!((result[i] - (a[i] * b[i])).abs() < 0.00001);
    }

    // Vector scale
    result.fill(0.0);
    let scale_result = matrix.vector_scale(&a, 2.5, &mut result)
        .expect("Vector scale failed");
    assert!(scale_result.success);

    for i in 0..size {
        assert!((result[i] - (a[i] * 2.5)).abs() < 0.00001);
    }
}

#[test]
fn test_matrix_determinant_2x2() {
    let matrix = Matrix::new();

    // 2x2 matrix: [1, 2]
    //             [3, 4]
    // Det = 1*4 - 2*3 = -2
    let matrix_2x2 = vec![1.0f32, 2.0, 3.0, 4.0];

    let det_result = matrix.determinant(&matrix_2x2, 2)
        .expect("Determinant failed");

    assert!(det_result.success);
    assert!(det_result.result.is_some());

    let det = det_result.result.unwrap();
    assert!((det - (-2.0)).abs() < 0.00001);
}

#[test]
fn test_large_matrix_operations() {
    let matrix = Matrix::new();
    let size = 1000;

    let mut a = vec![0.0f32; size];
    let mut b = vec![0.0f32; size];
    let mut result = vec![0.0f32; size];

    for i in 0..size {
        a[i] = (i % 100) as f32 * 0.01;
        b[i] = ((i + 50) % 100) as f32 * 0.01;
    }

    // Should handle large vectors efficiently
    let dot_result = matrix.dot_product(&a, &b)
        .expect("Dot product failed for large vectors");

    assert!(dot_result.success);
    assert!(dot_result.result.is_some());
}

#[test]
fn test_matrix_dimension_validation() {
    let matrix = Matrix::new();

    let a = vec![1.0f32, 2.0, 3.0, 4.0]; // 2x2
    let b = vec![1.0f32, 2.0, 3.0]; // Wrong size

    let mut result_vec = vec![0.0f32; 4];

    // Should fail due to dimension mismatch
    let multiply_result = matrix.multiply(&a, &b, &mut result_vec, 2, 2, 2);

    assert!(multiply_result.is_err());
}

#[test]
fn test_zero_vectors() {
    let matrix = Matrix::new();

    let a = vec![0.0f32; 10];
    let b = vec![0.0f32; 10];

    let dot_result = matrix.dot_product(&a, &b)
        .expect("Dot product failed");

    assert!(dot_result.success);
    assert_eq!(dot_result.result, Some(0.0));
}

#[test]
fn test_parallel_processing_threshold() {
    let matrix = Matrix::new();

    // Test with size that triggers parallel processing (>1000 elements)
    let size = 2000;
    let mut a = vec![0.0f32; size];
    let mut b = vec![0.0f32; size];
    let mut result = vec![0.0f32; size];

    for i in 0..size {
        a[i] = i as f32;
        b[i] = (i + 1) as f32;
    }

    let add_result = matrix.vector_add(&a, &b, &mut result)
        .expect("Addition failed for large matrix");

    assert!(add_result.success);

    // Verify results
    for i in 0..size {
        assert!((result[i] - (a[i] + b[i])).abs() < 0.00001);
    }
}

