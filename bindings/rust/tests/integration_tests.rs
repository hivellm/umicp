/*!
# Integration Tests for UMICP Rust Bindings

Comprehensive integration tests for the UMICP protocol implementation.
*/

use umicp_core::*;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_envelope_roundtrip() {
        // Create envelope with valid UUID
        let valid_uuid = uuid::Uuid::new_v4().to_string();
        let envelope = Envelope::builder()
            .from("test-sender")
            .to("test-receiver")
            .operation(OperationType::Data)
            .message_id(&valid_uuid)
            .capability("test", "value")
            .build()
            .unwrap();

        // Serialize
        let serialized = envelope.serialize().unwrap();

        // Deserialize
        let deserialized = Envelope::deserialize(&serialized).unwrap();

        // Verify
        assert_eq!(deserialized.from(), envelope.from());
        assert_eq!(deserialized.to(), envelope.to());
        assert_eq!(deserialized.operation(), envelope.operation());
        assert_eq!(deserialized.message_id(), envelope.message_id());
        assert_eq!(deserialized.capabilities(), envelope.capabilities());
    }

    #[test]
    fn test_matrix_operations() {
        let matrix = Matrix::new();

        // Test vector addition
        let a = vec![1.0, 2.0, 3.0];
        let b = vec![4.0, 5.0, 6.0];
        let mut result = vec![0.0; 3];

        matrix.vector_add(&a, &b, &mut result).unwrap();
        assert_eq!(result, vec![5.0, 7.0, 9.0]);

        // Test dot product
        let dot_result = matrix.dot_product(&a, &b).unwrap();
        assert_eq!(dot_result.result.unwrap(), 32.0);

        // Test matrix multiplication
        let matrix_a = vec![1.0, 2.0, 3.0, 4.0]; // 2x2
        let matrix_b = vec![5.0, 6.0, 7.0, 8.0]; // 2x2
        let mut matrix_result = vec![0.0; 4];

        matrix.multiply(&matrix_a, &matrix_b, &mut matrix_result, 2, 2, 2).unwrap();
        assert_eq!(matrix_result, vec![19.0, 22.0, 43.0, 50.0]);
    }

    #[test]
    fn test_envelope_validation() {
        // Valid envelope
        let valid = Envelope::builder()
            .from("sender")
            .to("receiver")
            .build();
        assert!(valid.is_ok());

        // Invalid - empty from
        let invalid = Envelope::builder()
            .from("")
            .to("receiver")
            .build();
        assert!(invalid.is_err());

        // Invalid - empty to
        let invalid = Envelope::builder()
            .from("sender")
            .to("")
            .build();
        assert!(invalid.is_err());

        // Invalid - malformed UUID
        let invalid = Envelope::builder()
            .from("sender")
            .to("receiver")
            .message_id("invalid-uuid")
            .build();
        assert!(invalid.is_err());
    }

    #[test]
    fn test_matrix_validation() {
        let matrix = Matrix::new();

        // Test dimension mismatch
        let a = vec![1.0, 2.0];
        let b = vec![1.0, 2.0, 3.0];
        let mut result = vec![0.0; 2];

        let matrix_result = matrix.vector_add(&a, &b, &mut result);
        assert!(matrix_result.is_err());
    }

    #[test]
    fn test_envelope_capabilities() {
        let envelope = Envelope::builder()
            .from("sender")
            .to("receiver")
            .capability("priority", "high")
            .capability("content-type", "application/json")
            .build()
            .unwrap();

        let capabilities = envelope.capabilities().unwrap();
        assert_eq!(capabilities.get("priority").unwrap(), "high");
        assert_eq!(capabilities.get("content-type").unwrap(), "application/json");
        assert_eq!(capabilities.len(), 2);
    }

    #[test]
    fn test_cosine_similarity() {
        let matrix = Matrix::new();

        // Identical vectors
        let a = vec![1.0, 2.0, 3.0];
        let b = vec![1.0, 2.0, 3.0];

        let result = matrix.cosine_similarity(&a, &b).unwrap();
        assert!((result.similarity.unwrap() - 1.0).abs() < 1e-6);

        // Orthogonal vectors
        let a = vec![1.0, 0.0];
        let b = vec![0.0, 1.0];

        let result = matrix.cosine_similarity(&a, &b).unwrap();
        assert!((result.similarity.unwrap() - 0.0).abs() < 1e-6);

        // Opposite vectors
        let a = vec![1.0, 2.0];
        let b = vec![-1.0, -2.0];

        let result = matrix.cosine_similarity(&a, &b).unwrap();
        assert!((result.similarity.unwrap() - (-1.0)).abs() < 1e-6);
    }

    #[test]
    fn test_envelope_hash() {
        // Create envelopes with same content (including timestamp by using same envelope data)
        let envelope1 = Envelope::builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType::Data)
            .capability("test", "value")
            .build()
            .unwrap();

        // Create envelope with same data by serializing and deserializing
        let serialized = envelope1.serialize().unwrap();
        let envelope2 = Envelope::deserialize(&serialized).unwrap();

        // Same content should produce same hash
        let hash1 = envelope1.hash().unwrap();
        let hash2 = envelope2.hash().unwrap();
        assert_eq!(hash1, hash2);

        // Different content should produce different hash
        let envelope3 = Envelope::builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType::Data)
            .capability("test", "different")
            .build()
            .unwrap();

        let hash3 = envelope3.hash().unwrap();
        assert_ne!(hash1, hash3);
    }

    #[test]
    fn test_matrix_determinant() {
        let matrix = Matrix::new();

        // 2x2 matrix: det([1, 2; 3, 4]) = 1*4 - 2*3 = -2
        let mat = vec![1.0, 2.0, 3.0, 4.0];
        let result = matrix.determinant(&mat, 2).unwrap();
        assert_eq!(result.result.unwrap(), -2.0);

        // Identity matrix: det([1, 0; 0, 1]) = 1
        let identity = vec![1.0, 0.0, 0.0, 1.0];
        let result = matrix.determinant(&identity, 2).unwrap();
        assert_eq!(result.result.unwrap(), 1.0);
    }

    #[test]
    fn test_envelope_serialization_edge_cases() {
        // Test with all optional fields and valid UUID
        let valid_uuid = uuid::Uuid::new_v4().to_string();
        let envelope = Envelope::builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType::Data)
            .message_id(&valid_uuid)
            .capability("key1", "value1")
            .capability("key2", "value2")
            .build()
            .unwrap();

        let serialized = envelope.serialize().unwrap();
        let deserialized = Envelope::deserialize(&serialized).unwrap();

        assert_eq!(deserialized.from(), "sender");
        assert_eq!(deserialized.to(), "receiver");
        assert_eq!(deserialized.operation(), OperationType::Data);
        assert_eq!(deserialized.message_id(), valid_uuid);

        let capabilities = deserialized.capabilities().unwrap();
        assert_eq!(capabilities.get("key1").unwrap(), "value1");
        assert_eq!(capabilities.get("key2").unwrap(), "value2");
    }
}
