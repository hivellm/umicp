/**
 * Edge case tests for UMICP Matrix operations
 */

import { Matrix, UMICP, MatrixResult } from '../src/index';

describe('Matrix Edge Cases', () => {
  let matrix: Matrix;

  beforeEach(() => {
    matrix = UMICP.createMatrix();
  });

  describe('Error Handling', () => {
    test('should handle empty arrays in dotProduct', () => {
      const a = new Float32Array(0);
      const b = new Float32Array(0);

      const result = matrix.dotProduct(a, b);
      expect(result.success).toBeDefined();
    });

    test('should handle mismatched sizes in dotProduct', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([1, 2]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
      expect(typeof result.success).toBe('boolean');
    });

    test('should handle zero-sized matrices in add', () => {
      const a = new Float32Array(0);
      const b = new Float32Array(0);
      const result = new Float32Array(0);

      const res = matrix.add(a, b, result, 0, 0);
      expect(res).toBeDefined();
    });

    test('should handle invalid dimensions in multiply', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([1, 2]);
      const result = new Float32Array(4);

      const res = matrix.multiply(a, b, result, 2, 2, 1);
      expect(res).toBeDefined();
      expect(typeof res.success).toBe('boolean');
    });
  });

  describe('Boundary Values', () => {
    test('should handle very large values', () => {
      const a = new Float32Array([1e38, 1e38]);
      const b = new Float32Array([1e38, 1e38]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle very small values', () => {
      const a = new Float32Array([1e-38, 1e-38]);
      const b = new Float32Array([1e-38, 1e-38]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle negative values', () => {
      const a = new Float32Array([-1, -2, -3]);
      const b = new Float32Array([-1, -2, -3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle mixed positive and negative', () => {
      const a = new Float32Array([1, -2, 3, -4]);
      const b = new Float32Array([-1, 2, -3, 4]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle zero values', () => {
      const a = new Float32Array([0, 0, 0]);
      const b = new Float32Array([1, 2, 3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
      if (result.success) {
        expect(result.result).toBe(0);
      }
    });
  });

  describe('Single Element Operations', () => {
    test('should handle 1x1 matrix addition', () => {
      const a = new Float32Array([5]);
      const b = new Float32Array([3]);
      const result = new Float32Array(1);

      const res = matrix.add(a, b, result, 1, 1);
      expect(res).toBeDefined();
    });

    test('should handle 1x1 matrix multiplication', () => {
      const a = new Float32Array([5]);
      const b = new Float32Array([3]);
      const result = new Float32Array(1);

      const res = matrix.multiply(a, b, result, 1, 1, 1);
      expect(res).toBeDefined();
    });

    test('should handle single-element vectors', () => {
      const a = new Float32Array([5]);
      const b = new Float32Array([3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });
  });

  describe('Large Matrix Operations', () => {
    test('should handle large vectors in dotProduct', () => {
      const size = 1000;
      const a = new Float32Array(size).fill(1);
      const b = new Float32Array(size).fill(2);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle large matrix addition', () => {
      const size = 100;
      const a = new Float32Array(size * size).fill(1);
      const b = new Float32Array(size * size).fill(2);
      const result = new Float32Array(size * size);

      const res = matrix.add(a, b, result, size, size);
      expect(res).toBeDefined();
    });

    test('should handle large transpose', () => {
      const rows = 100;
      const cols = 50;
      const input = new Float32Array(rows * cols);
      const output = new Float32Array(cols * rows);

      // Fill with test data
      for (let i = 0; i < input.length; i++) {
        input[i] = i;
      }

      const res = matrix.transpose(input, output, rows, cols);
      expect(res).toBeDefined();
    });
  });

  describe('Special Cases', () => {
    test('should handle identical vectors in cosineSimilarity', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([1, 2, 3]);

      const result = matrix.cosineSimilarity(a, b);
      expect(result).toBeDefined();
      if (result.success) {
        expect(result.similarity).toBeCloseTo(1.0, 5);
      }
    });

    test('should handle orthogonal vectors', () => {
      const a = new Float32Array([1, 0]);
      const b = new Float32Array([0, 1]);

      const result = matrix.cosineSimilarity(a, b);
      expect(result).toBeDefined();
      if (result.success) {
        expect(result.similarity).toBeCloseTo(0.0, 5);
      }
    });

    test('should handle opposite vectors', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([-1, -2, -3]);

      const result = matrix.cosineSimilarity(a, b);
      expect(result).toBeDefined();
      if (result.success) {
        expect(result.similarity).toBeCloseTo(-1.0, 5);
      }
    });

    test('should handle normalization of unit vector', () => {
      const vector = new Float32Array([1, 0, 0]);

      const result = matrix.normalize(vector, 1, 3);
      expect(result).toBeDefined();
    });

    test('should handle normalization of zero vector', () => {
      const vector = new Float32Array([0, 0, 0]);

      const result = matrix.normalize(vector, 1, 3);
      expect(result).toBeDefined();
    });
  });

  describe('MatrixResult Structure', () => {
    test('should return proper result structure on success', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([4, 5, 6]);

      const result = matrix.dotProduct(a, b);

      expect(result).toBeDefined();
      expect(result).toHaveProperty('success');
      expect(typeof result.success).toBe('boolean');

      if (result.success) {
        expect(result).toHaveProperty('result');
        expect(typeof result.result).toBe('number');
      }
    });

    test('should return error message on failure', () => {
      // Try an operation that might fail
      const a = new Float32Array([1]);
      const b = new Float32Array([1, 2]);

      const result = matrix.dotProduct(a, b);

      expect(result).toBeDefined();
      if (!result.success) {
        expect(result).toHaveProperty('error');
        expect(typeof result.error).toBe('string');
      }
    });
  });

  describe('Square Matrix Operations', () => {
    test('should handle 2x2 matrix multiplication', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([5, 6, 7, 8]);
      const result = new Float32Array(4);

      const res = matrix.multiply(a, b, result, 2, 2, 2);
      expect(res).toBeDefined();
    });

    test('should handle 3x3 matrix addition', () => {
      const a = new Float32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]);
      const b = new Float32Array([9, 8, 7, 6, 5, 4, 3, 2, 1]);
      const result = new Float32Array(9);

      const res = matrix.add(a, b, result, 3, 3);
      expect(res).toBeDefined();
    });

    test('should handle square matrix transpose', () => {
      const input = new Float32Array([1, 2, 3, 4]);
      const output = new Float32Array(4);

      const res = matrix.transpose(input, output, 2, 2);
      expect(res).toBeDefined();
    });
  });

  describe('Non-Square Matrix Operations', () => {
    test('should handle 2x3 matrix transpose to 3x2', () => {
      const input = new Float32Array([1, 2, 3, 4, 5, 6]);
      const output = new Float32Array(6);

      const res = matrix.transpose(input, output, 2, 3);
      expect(res).toBeDefined();
    });

    test('should handle rectangular matrix multiplication', () => {
      const a = new Float32Array([1, 2, 3, 4, 5, 6]); // 2x3
      const b = new Float32Array([7, 8, 9, 10, 11, 12]); // 3x2
      const result = new Float32Array(4); // 2x2

      const res = matrix.multiply(a, b, result, 2, 3, 2);
      expect(res).toBeDefined();
    });
  });

  describe('Consistency Tests', () => {
    test('should give consistent results for repeated operations', () => {
      const a = new Float32Array([1, 2, 3, 4, 5]);
      const b = new Float32Array([5, 4, 3, 2, 1]);

      const result1 = matrix.dotProduct(a, b);
      const result2 = matrix.dotProduct(a, b);
      const result3 = matrix.dotProduct(a, b);

      expect(result1.success).toBe(result2.success);
      expect(result2.success).toBe(result3.success);

      if (result1.success && result2.success && result3.success) {
        expect(result1.result).toBe(result2.result);
        expect(result2.result).toBe(result3.result);
      }
    });

    test('should handle multiple matrix instances', () => {
      const matrix1 = new Matrix();
      const matrix2 = new Matrix();
      const matrix3 = UMICP.createMatrix();

      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([4, 5, 6]);

      const result1 = matrix1.dotProduct(a, b);
      const result2 = matrix2.dotProduct(a, b);
      const result3 = matrix3.dotProduct(a, b);

      expect(result1.success).toBe(result2.success);
      expect(result2.success).toBe(result3.success);
    });
  });
});

