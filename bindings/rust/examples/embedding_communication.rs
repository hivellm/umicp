/*!
# Embedding Communication Example

This example demonstrates efficient AI model communication using UMICP
for embeddings, transformer states, and distributed model coordination.
*/

use umicp_core::{Envelope, Matrix, OperationType};
use serde::{Serialize, Deserialize};
use std::collections::HashMap;

/// AI Model embedding data structure
#[derive(Clone, Debug, Serialize, Deserialize)]
struct EmbeddingData {
    model_name: String,
    layer_index: usize,
    sequence_length: usize,
    embedding_dim: usize,
    embeddings: Vec<f32>,
    attention_mask: Option<Vec<u8>>,
    token_ids: Option<Vec<i32>>,
    metadata: ModelMetadata,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
struct ModelMetadata {
    batch_size: usize,
    max_seq_length: usize,
    vocab_size: usize,
    model_config: HashMap<String, serde_json::Value>,
}

/// Transformer layer state for model synchronization
#[derive(Clone, Debug, Serialize, Deserialize)]
struct TransformerState {
    layer_states: Vec<LayerState>,
    final_embeddings: Vec<f32>,
    classification_head: Option<Vec<f32>>,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
struct LayerState {
    attention_weights: Vec<f32>,
    key_cache: Vec<f32>,
    value_cache: Vec<f32>,
    feed_forward_output: Vec<f32>,
}

/// Communication protocol for model coordination
#[derive(Clone, Debug, Serialize, Deserialize)]
struct ModelCommunication {
    source_model: String,
    target_model: String,
    communication_type: CommunicationType,
    data: CommunicationData,
    timestamp: i64,
    sequence_id: String,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
enum CommunicationType {
    EmbeddingTransfer,
    AttentionSharing,
    GradientSync,
    KVCacheSync,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
enum CommunicationData {
    Embedding(EmbeddingData),
    Transformer(TransformerState),
}

/// High-performance embedding serializer
struct EmbeddingSerializer {
    matrix: Matrix,
}

impl EmbeddingSerializer {
    fn new() -> Self {
        Self {
            matrix: Matrix::new(),
        }
    }

    /// Create BERT-style embedding data
    fn create_bert_embedding(&self, sequence_length: usize, embedding_dim: usize) -> EmbeddingData {
        let mut embeddings = vec![0.0f32; sequence_length * embedding_dim];
        let mut attention_mask = vec![0u8; sequence_length];
        let mut token_ids = vec![0i32; sequence_length];

        // Generate realistic embedding patterns
        for i in 0..embeddings.len() {
            let seq_pos = i / embedding_dim;
            let dim_pos = i % embedding_dim;
            embeddings[i] = (seq_pos as f32 * 0.1).sin() * (dim_pos as f32 * 0.01).cos() * 0.1;
        }

        // Set attention mask (first part is real tokens)
        for i in 0..(sequence_length / 2) {
            attention_mask[i] = 1;
        }

        // Generate token IDs
        for i in 0..sequence_length {
            token_ids[i] = (i % 30522) as i32; // BERT vocab size
        }

        EmbeddingData {
            model_name: "bert-base-uncased".to_string(),
            layer_index: 11,
            sequence_length,
            embedding_dim,
            embeddings,
            attention_mask: Some(attention_mask),
            token_ids: Some(token_ids),
            metadata: ModelMetadata {
                batch_size: 1,
                max_seq_length: 512,
                vocab_size: 30522,
                model_config: {
                    let mut config = HashMap::new();
                    config.insert("hidden_size".to_string(), 768.into());
                    config.insert("num_layers".to_string(), 12.into());
                    config.insert("num_heads".to_string(), 12.into());
                    config.insert("intermediate_size".to_string(), 3072.into());
                    config
                },
            },
        }
    }

    /// Create GPT-style attention weights
    fn create_gpt_attention(&self, sequence_length: usize, num_heads: usize) -> Vec<f32> {
        let attention_size = sequence_length * sequence_length * num_heads;
        let mut attention_weights = vec![0.0f32; attention_size];

        // Generate attention patterns
        for head in 0..num_heads {
            for i in 0..sequence_length {
                for j in 0..sequence_length {
                    let idx = head * sequence_length * sequence_length + i * sequence_length + j;
                    // Causal attention pattern
                    if j <= i {
                        let distance = (i - j) as f32;
                        attention_weights[idx] = (-distance * 0.1).exp() * (head as f32 * 0.1).sin();
                    }
                }
            }
        }

        attention_weights
    }

    /// Create T5-style encoder embeddings
    fn create_t5_encoder_output(&self, sequence_length: usize, embedding_dim: usize) -> EmbeddingData {
        let mut embeddings = vec![0.0f32; sequence_length * embedding_dim];

        // Generate encoder-style patterns
        for i in 0..embeddings.len() {
            let seq_pos = i / embedding_dim;
            let dim_pos = i % embedding_dim;
            embeddings[i] = (seq_pos as f32 * 0.05).cos() * (dim_pos as f32 * 0.02).sin() * 0.15;
        }

        EmbeddingData {
            model_name: "t5-encoder".to_string(),
            layer_index: 11,
            sequence_length,
            embedding_dim,
            embeddings,
            attention_mask: None,
            token_ids: None,
            metadata: ModelMetadata {
                batch_size: 1,
                max_seq_length: 512,
                vocab_size: 32100,
                model_config: {
                    let mut config = HashMap::new();
                    config.insert("hidden_size".to_string(), embedding_dim.into());
                    config.insert("num_layers".to_string(), 12.into());
                    config.insert("num_heads".to_string(), 8.into());
                    config.insert("d_kv".to_string(), 64.into());
                    config.insert("d_ff".to_string(), 2048.into());
                    config
                },
            },
        }
    }
}

/// Transformer communication handler
struct TransformerCommunication {
    serializer: EmbeddingSerializer,
    matrix: Matrix,
}

impl TransformerCommunication {
    fn new() -> Self {
        Self {
            serializer: EmbeddingSerializer::new(),
            matrix: Matrix::new(),
        }
    }

    /// Create embedding transfer envelope
    fn create_embedding_envelope(&self, source: &str, target: &str, data: &EmbeddingData) -> Result<Envelope, Box<dyn std::error::Error>> {
        let json_data = serde_json::to_string(data)?;

        Ok(Envelope::builder()
            .from(source)
            .to(target)
            .operation(OperationType::Data)
            .message_id(&uuid::Uuid::new_v4().to_string())
            .capability("message_type", "embedding_transfer")
            .capability("model_name", &data.model_name)
            .capability("layer_index", &data.layer_index.to_string())
            .capability("sequence_length", &data.sequence_length.to_string())
            .capability("embedding_dim", &data.embedding_dim.to_string())
            .capability("data_size", &json_data.len().to_string())
            .capability("compression", "none")
            .capability("format", "json")
            .build()?)
    }

    /// Create attention sharing envelope
    fn create_attention_envelope(&self, source: &str, target: &str, attention_weights: &[f32], sequence_length: usize, num_heads: usize) -> Result<Envelope, Box<dyn std::error::Error>> {
        Ok(Envelope::builder()
            .from(source)
            .to(target)
            .operation(OperationType::Data)
            .message_id(&uuid::Uuid::new_v4().to_string())
            .capability("message_type", "attention_sharing")
            .capability("sequence_length", &sequence_length.to_string())
            .capability("num_heads", &num_heads.to_string())
            .capability("attention_size", &attention_weights.len().to_string())
            .capability("attention_shape", &format!("{}x{}x{}", sequence_length, sequence_length, num_heads))
            .build()?)
    }

    /// Process embedding envelope
    fn process_embedding_envelope(&self, envelope: &Envelope) -> Result<EmbeddingData, Box<dyn std::error::Error>> {
        let capabilities = envelope.capabilities().ok_or("No capabilities found")?;

        if capabilities.get("message_type").map(|s| s.as_str()) != Some("embedding_transfer") {
            return Err("Not an embedding transfer envelope".into());
        }

        // In a real implementation, you'd extract the binary data
        // For this example, we'll create mock data based on capabilities
        let model_name = capabilities.get("model_name").ok_or("Missing model_name")?.clone();
        let layer_index: usize = capabilities.get("layer_index").ok_or("Missing layer_index")?.parse()?;
        let sequence_length: usize = capabilities.get("sequence_length").ok_or("Missing sequence_length")?.parse()?;
        let embedding_dim: usize = capabilities.get("embedding_dim").ok_or("Missing embedding_dim")?.parse()?;

        // Create mock embedding data
        let mut embeddings = vec![0.0f32; sequence_length * embedding_dim];
        for i in 0..embeddings.len() {
            embeddings[i] = (i as f32 * 0.001).sin() * 0.1;
        }

        let metadata = ModelMetadata {
            batch_size: 1,
            max_seq_length: sequence_length,
            vocab_size: 30000,
            model_config: HashMap::new(),
        };

        Ok(EmbeddingData {
            model_name,
            layer_index,
            sequence_length,
            embedding_dim,
            embeddings,
            attention_mask: None,
            token_ids: None,
            metadata,
        })
    }

    /// Find similar embeddings using cosine similarity
    fn find_similar_embeddings(&self, query: &[f32], candidates: &[Vec<f32>], top_k: usize) -> Result<Vec<(usize, f32)>, Box<dyn std::error::Error>> {
        let mut similarities: Vec<(usize, f32)> = Vec::new();

        for (i, candidate) in candidates.iter().enumerate() {
            let similarity_result = self.matrix.cosine_similarity(query, candidate)?;
            if let Some(similarity) = similarity_result.similarity {
                similarities.push((i, similarity as f32));
            }
        }

        // Sort by similarity (descending)
        similarities.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));

        Ok(similarities.into_iter().take(top_k).collect())
    }

    /// Aggregate embeddings from multiple sources
    fn aggregate_embeddings(&self, embeddings: &[Vec<f32>]) -> Result<Vec<f32>, Box<dyn std::error::Error>> {
        if embeddings.is_empty() {
            return Err("No embeddings to aggregate".into());
        }

        let embedding_dim = embeddings[0].len();
        let mut aggregated = vec![0.0f32; embedding_dim];

        // Verify all embeddings have the same dimension
        for embedding in embeddings {
            if embedding.len() != embedding_dim {
                return Err("All embeddings must have the same dimension".into());
            }
        }

        // Average the embeddings
        for embedding in embeddings {
            for i in 0..embedding_dim {
                aggregated[i] += embedding[i];
            }
        }

        for i in 0..embedding_dim {
            aggregated[i] /= embeddings.len() as f32;
        }

        Ok(aggregated)
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("🧠 UMICP Rust Embedding Communication Example");
    println!("=============================================");

    let communication = TransformerCommunication::new();
    let serializer = EmbeddingSerializer::new();

    // Example 1: BERT-style embedding transfer
    println!("\n1. BERT-Style Embedding Transfer");
    println!("-------------------------------");

    let bert_embedding = serializer.create_bert_embedding(128, 768);
    let bert_envelope = communication.create_embedding_envelope(
        "bert-encoder",
        "classification-head",
        &bert_embedding
    )?;

    println!("✅ BERT embedding envelope created");
    println!("   Model: {}", bert_embedding.model_name);
    println!("   Layer: {}", bert_embedding.layer_index);
    println!("   Sequence length: {}", bert_embedding.sequence_length);
    println!("   Embedding dim: {}", bert_embedding.embedding_dim);
    println!("   Data size: {} KB", (bert_embedding.embeddings.len() * 4) / 1024);

    // Example 2: GPT-style attention sharing
    println!("\n2. GPT-Style Attention Sharing");
    println!("----------------------------");

    let sequence_length = 512;
    let num_heads = 12;
    let attention_weights = serializer.create_gpt_attention(sequence_length, num_heads);

    let attention_envelope = communication.create_attention_envelope(
        "gpt-decoder",
        "attention-aggregator",
        &attention_weights,
        sequence_length,
        num_heads
    )?;

    println!("✅ GPT attention envelope created");
    println!("   Sequence length: {}", sequence_length);
    println!("   Number of heads: {}", num_heads);
    println!("   Attention matrix size: {} ({:.2} MB)",
             attention_weights.len(),
             (attention_weights.len() * 4) as f64 / (1024.0 * 1024.0));

    // Example 3: T5-style encoder-decoder communication
    println!("\n3. T5-Style Encoder-Decoder Communication");
    println!("-----------------------------------------");

    let t5_embedding = serializer.create_t5_encoder_output(256, 512);
    let t5_envelope = communication.create_embedding_envelope(
        "t5-encoder",
        "t5-decoder",
        &t5_embedding
    )?;

    println!("✅ T5 encoder-decoder envelope created");
    println!("   Encoder output size: {} KB", (t5_embedding.embeddings.len() * 4) / 1024);

    // Example 4: Embedding similarity search
    println!("\n4. Embedding Similarity Search");
    println!("------------------------------");

    // Create embedding database
    let database_size = 1000;
    let embedding_dim = 384;
    let mut embedding_database: Vec<Vec<f32>> = Vec::new();

    for i in 0..database_size {
        let mut embedding = vec![0.0f32; embedding_dim];
        for j in 0..embedding_dim {
            embedding[j] = ((i as f32) * 0.1).sin() + ((j as f32) * 0.01).cos() * 0.1;
        }
        embedding_database.push(embedding);
    }

    // Create query embedding
    let mut query_embedding = vec![0.0f32; embedding_dim];
    for j in 0..embedding_dim {
        query_embedding[j] = (0.05_f32).sin() + ((j as f32) * 0.01_f32).cos() * 0.1_f32;
    }

    let start_time = std::time::Instant::now();
    let similar_results = communication.find_similar_embeddings(
        &query_embedding,
        &embedding_database,
        5
    )?;
    let search_time = start_time.elapsed();

    println!("✅ Similarity search completed");
    println!("   Database size: {} embeddings", database_size);
    println!("   Search time: {:.3} ms", search_time.as_secs_f64() * 1000.0);
    println!("   Results per second: {:.0}", database_size as f64 / search_time.as_secs_f64());
    println!("   Top 5 similar embeddings:");

    for (i, (index, similarity)) in similar_results.iter().enumerate() {
        println!("     {}. Index {}: similarity {:.4}", i + 1, index, similarity);
    }

    // Example 5: Federated learning embedding aggregation
    println!("\n5. Federated Learning Embedding Aggregation");
    println!("-------------------------------------------");

    let num_clients = 5;
    let shared_embedding_dim = 512;
    let mut client_embeddings: Vec<Vec<f32>> = Vec::new();

    for client in 0..num_clients {
        let mut client_embedding = vec![0.0f32; shared_embedding_dim];
        for i in 0..shared_embedding_dim {
            client_embedding[i] = ((i as f32) * 0.01).sin() + (client as f32 * 0.01);
        }
        client_embeddings.push(client_embedding);
    }

    let aggregated_embedding = communication.aggregate_embeddings(&client_embeddings)?;

    let magnitude = aggregated_embedding.iter()
        .fold(0.0f32, |sum, &val| sum + val * val)
        .sqrt();

    println!("✅ Federated embedding aggregation completed");
    println!("   Clients: {}", num_clients);
    println!("   Embedding dimension: {}", shared_embedding_dim);
    println!("   Aggregated embedding magnitude: {:.4}", magnitude);

    // Example 6: Model state serialization
    println!("\n6. Model State Serialization");
    println!("----------------------------");

    let transformer_state = TransformerState {
        layer_states: vec![
            LayerState {
                attention_weights: vec![0.1, 0.2, 0.3],
                key_cache: vec![0.4, 0.5, 0.6],
                value_cache: vec![0.7, 0.8, 0.9],
                feed_forward_output: vec![1.0, 1.1, 1.2],
            }
        ],
        final_embeddings: vec![2.0, 2.1, 2.2],
        classification_head: Some(vec![0.0, 0.0]),
    };

    let model_comm = ModelCommunication {
        source_model: "gpt-2".to_string(),
        target_model: "model-server".to_string(),
        communication_type: CommunicationType::KVCacheSync,
        data: CommunicationData::Transformer(transformer_state),
        timestamp: chrono::Utc::now().timestamp(),
        sequence_id: uuid::Uuid::new_v4().to_string(),
    };

    let serialized_model = serde_json::to_string(&model_comm)?;
    println!("✅ Model state serialized");
    println!("   JSON size: {} bytes", serialized_model.len());

    // Deserialize
    let deserialized_model: ModelCommunication = serde_json::from_str(&serialized_model)?;
    println!("✅ Model state deserialized successfully");

    println!("\n🎉 Embedding Communication Example Completed!");
    println!("=============================================");
    println!();
    println!("💡 Key Benefits for AI Model Communication:");
    println!("   • Efficient binary serialization of embeddings");
    println!("   • Type-safe transformer state transfer");
    println!("   • Optimized attention weight sharing");
    println!("   • Real-time embedding similarity search");
    println!("   • Federated learning support");
    println!("   • Memory-efficient large model handling");

    Ok(())
}
