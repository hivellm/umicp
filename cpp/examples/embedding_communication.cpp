/**
 * UMICP C++ Bindings - Embedding Communication Demo
 *
 * This example demonstrates AI model communication using UMICP envelopes
 * for embeddings, transformer states, and distributed model coordination.
 */

#include <umicp.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <algorithm>

/// AI Model embedding data structure
struct EmbeddingData {
    std::string model_name;
    size_t layer_index;
    size_t sequence_length;
    size_t embedding_dim;
    std::vector<float> embeddings;
    std::vector<uint8_t> attention_mask;
    std::vector<int32_t> token_ids;
    std::unordered_map<std::string, std::string> metadata;

    EmbeddingData(const std::string& name, size_t layer, size_t seq_len, size_t emb_dim)
        : model_name(name), layer_index(layer), sequence_length(seq_len), embedding_dim(emb_dim) {
        embeddings.resize(sequence_length * embedding_dim);
        attention_mask.resize(sequence_length, 1);
        token_ids.resize(sequence_length, 0);
    }
};

/// Transformer layer state
struct LayerState {
    std::vector<float> attention_weights;
    std::vector<float> key_cache;
    std::vector<float> value_cache;
    std::vector<float> feed_forward_output;
};

/// Complete transformer state
struct TransformerState {
    std::vector<LayerState> layer_states;
    std::vector<float> final_embeddings;
    std::vector<float> classification_head;
};

/// Communication protocol types
enum class CommunicationType {
    EMBEDDING_TRANSFER,
    ATTENTION_SHARING,
    GRADIENT_SYNC,
    KV_CACHE_SYNC
};

/// High-performance embedding serializer
class EmbeddingSerializer {
private:
    std::mt19937 gen_;
    std::normal_distribution<float> normal_dist_;

public:
    EmbeddingSerializer()
        : gen_(std::random_device{}()), normal_dist_(0.0f, 0.1f) {}

    /// Create BERT-style embedding data
    EmbeddingData createBertEmbedding(size_t sequence_length, size_t embedding_dim) {
        EmbeddingData data("bert-base-uncased", 11, sequence_length, embedding_dim);

        // Generate realistic embedding patterns
        for (size_t i = 0; i < data.embeddings.size(); ++i) {
            size_t seq_pos = i / embedding_dim;
            size_t dim_pos = i % embedding_dim;
            data.embeddings[i] = std::sin(seq_pos * 0.1f) * std::cos(dim_pos * 0.01f) * 0.1f;
        }

        // Set attention mask (first half are real tokens)
        std::fill(data.attention_mask.begin(), data.attention_mask.begin() + sequence_length / 2, 1);
        std::fill(data.attention_mask.begin() + sequence_length / 2, data.attention_mask.end(), 0);

        // Generate token IDs
        for (size_t i = 0; i < sequence_length; ++i) {
            data.token_ids[i] = static_cast<int32_t>(i % 30522); // BERT vocab size
        }

        // Set metadata
        data.metadata["batch_size"] = "1";
        data.metadata["max_seq_length"] = "512";
        data.metadata["vocab_size"] = "30522";
        data.metadata["hidden_size"] = std::to_string(embedding_dim);
        data.metadata["num_layers"] = "12";
        data.metadata["num_heads"] = "12";
        data.metadata["intermediate_size"] = "3072";

        return data;
    }

    /// Create GPT-style attention weights
    std::vector<float> createGptAttention(size_t sequence_length, size_t num_heads) {
        size_t attention_size = sequence_length * sequence_length * num_heads;
        std::vector<float> attention_weights(attention_size, 0.0f);

        // Generate attention patterns
        for (size_t head = 0; head < num_heads; ++head) {
            for (size_t i = 0; i < sequence_length; ++i) {
                for (size_t j = 0; j < sequence_length; ++j) {
                    size_t idx = head * sequence_length * sequence_length + i * sequence_length + j;
                    // Causal attention pattern
                    if (j <= i) {
                        float distance = static_cast<float>(i - j);
                        attention_weights[idx] = std::exp(-distance * 0.1f) * std::sin(head * 0.1f);
                    }
                }
            }
        }

        return attention_weights;
    }

    /// Create T5-style encoder embeddings
    EmbeddingData createT5EncoderOutput(size_t sequence_length, size_t embedding_dim) {
        EmbeddingData data("t5-encoder", 11, sequence_length, embedding_dim);

        // Generate encoder-style patterns
        for (size_t i = 0; i < data.embeddings.size(); ++i) {
            size_t seq_pos = i / embedding_dim;
            size_t dim_pos = i % embedding_dim;
            data.embeddings[i] = std::cos(seq_pos * 0.05f) * std::sin(dim_pos * 0.02f) * 0.15f;
        }

        // Set metadata
        data.metadata["batch_size"] = "1";
        data.metadata["max_seq_length"] = "512";
        data.metadata["vocab_size"] = "32100";
        data.metadata["hidden_size"] = std::to_string(embedding_dim);
        data.metadata["num_layers"] = "12";
        data.metadata["num_heads"] = "8";
        data.metadata["d_kv"] = "64";
        data.metadata["d_ff"] = "2048";

        return data;
    }
};

/// Transformer communication handler
class TransformerCommunication {
private:
    EmbeddingSerializer serializer_;

public:
    TransformerCommunication() : serializer_() {}

    /// Create embedding transfer envelope
    umicp_envelope_t* createEmbeddingEnvelope(const std::string& source, const std::string& target, const EmbeddingData& data) {
        umicp_envelope_t* envelope = umicp_create_envelope();

        umicp_set_from(envelope, source.c_str());
        umicp_set_to(envelope, target.c_str());
        umicp_set_operation(envelope, UMICP_OPERATION_DATA);
        umicp_set_message_id(envelope, ("embedding-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count())).c_str());

        // Add embedding metadata
        umicp_set_capability(envelope, "message_type", "embedding_transfer");
        umicp_set_capability(envelope, "model_name", data.model_name.c_str());
        umicp_set_capability(envelope, "layer_index", std::to_string(data.layer_index).c_str());
        umicp_set_capability(envelope, "sequence_length", std::to_string(data.sequence_length).c_str());
        umicp_set_capability(envelope, "embedding_dim", std::to_string(data.embedding_dim).c_str());
        umicp_set_capability(envelope, "data_size", std::to_string(data.embeddings.size() * sizeof(float)).c_str());
        umicp_set_capability(envelope, "compression", "none");
        umicp_set_capability(envelope, "format", "binary");

        // Add metadata
        for (const auto& [key, value] : data.metadata) {
            umicp_set_capability(envelope, ("meta_" + key).c_str(), value.c_str());
        }

        return envelope;
    }

    /// Create attention sharing envelope
    umicp_envelope_t* createAttentionEnvelope(const std::string& source, const std::string& target,
                                            const std::vector<float>& attention_weights,
                                            size_t sequence_length, size_t num_heads) {
        umicp_envelope_t* envelope = umicp_create_envelope();

        umicp_set_from(envelope, source.c_str());
        umicp_set_to(envelope, target.c_str());
        umicp_set_operation(envelope, UMICP_OPERATION_DATA);
        umicp_set_message_id(envelope, ("attention-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count())).c_str());

        umicp_set_capability(envelope, "message_type", "attention_sharing");
        umicp_set_capability(envelope, "sequence_length", std::to_string(sequence_length).c_str());
        umicp_set_capability(envelope, "num_heads", std::to_string(num_heads).c_str());
        umicp_set_capability(envelope, "attention_size", std::to_string(attention_weights.size()).c_str());
        umicp_set_capability(envelope, "attention_shape",
                           (std::to_string(sequence_length) + "x" + std::to_string(sequence_length) + "x" + std::to_string(num_heads)).c_str());

        return envelope;
    }

    /// Create KV cache synchronization envelope
    umicp_envelope_t* createKVCacheEnvelope(const std::string& source, const std::string& target,
                                          const std::vector<float>& key_cache,
                                          const std::vector<float>& value_cache,
                                          size_t layer_index, size_t sequence_pos) {
        umicp_envelope_t* envelope = umicp_create_envelope();

        umicp_set_from(envelope, source.c_str());
        umicp_set_to(envelope, target.c_str());
        umicp_set_operation(envelope, UMICP_OPERATION_DATA);
        umicp_set_message_id(envelope, ("kv-cache-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count())).c_str());

        umicp_set_capability(envelope, "message_type", "kv_cache_sync");
        umicp_set_capability(envelope, "layer_index", std::to_string(layer_index).c_str());
        umicp_set_capability(envelope, "sequence_position", std::to_string(sequence_pos).c_str());
        umicp_set_capability(envelope, "key_cache_size", std::to_string(key_cache.size()).c_str());
        umicp_set_capability(envelope, "value_cache_size", std::to_string(value_cache.size()).c_str());
        umicp_set_capability(envelope, "cache_format", "float32");

        return envelope;
    }

    /// Process embedding envelope
    EmbeddingData processEmbeddingEnvelope(umicp_envelope_t* envelope) {
        const char* message_type = umicp_get_capability(envelope, "message_type");
        if (strcmp(message_type, "embedding_transfer") != 0) {
            throw std::runtime_error("Not an embedding transfer envelope");
        }

        // Extract metadata from envelope
        const char* model_name = umicp_get_capability(envelope, "model_name");
        const char* layer_index_str = umicp_get_capability(envelope, "layer_index");
        const char* seq_length_str = umicp_get_capability(envelope, "sequence_length");
        const char* emb_dim_str = umicp_get_capability(envelope, "embedding_dim");

        size_t layer_index = std::stoul(layer_index_str);
        size_t sequence_length = std::stoul(seq_length_str);
        size_t embedding_dim = std::stoul(emb_dim_str);

        // Create embedding data (in real implementation, extract from envelope payload)
        EmbeddingData data(std::string(model_name), layer_index, sequence_length, embedding_dim);

        // Generate mock data for demonstration
        for (size_t i = 0; i < data.embeddings.size(); ++i) {
            data.embeddings[i] = std::sin(i * 0.001f) * 0.1f;
        }

        return data;
    }

    /// Find similar embeddings using cosine similarity
    std::vector<std::pair<size_t, float>> findSimilarEmbeddings(
        const std::vector<float>& query,
        const std::vector<std::vector<float>>& candidates,
        size_t top_k = 5) {

        std::vector<std::pair<size_t, float>> similarities;

        for (size_t i = 0; i < candidates.size(); ++i) {
            float similarity = calculateCosineSimilarity(query, candidates[i]);
            similarities.emplace_back(i, similarity);
        }

        // Sort by similarity (descending)
        std::sort(similarities.begin(), similarities.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });

        if (similarities.size() > top_k) {
            similarities.resize(top_k);
        }

        return similarities;
    }

    /// Aggregate embeddings from multiple sources
    std::vector<float> aggregateEmbeddings(const std::vector<std::vector<float>>& embeddings) {
        if (embeddings.empty()) {
            throw std::runtime_error("No embeddings to aggregate");
        }

        size_t embedding_dim = embeddings[0].size();
        std::vector<float> aggregated(embedding_dim, 0.0f);

        // Verify all embeddings have the same dimension
        for (const auto& embedding : embeddings) {
            if (embedding.size() != embedding_dim) {
                throw std::runtime_error("All embeddings must have the same dimension");
            }
        }

        // Average the embeddings
        for (const auto& embedding : embeddings) {
            for (size_t i = 0; i < embedding_dim; ++i) {
                aggregated[i] += embedding[i];
            }
        }

        for (size_t i = 0; i < embedding_dim; ++i) {
            aggregated[i] /= static_cast<float>(embeddings.size());
        }

        return aggregated;
    }

private:
    float calculateCosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
        if (a.size() != b.size()) {
            return 0.0f;
        }

        float dot_product = 0.0f;
        float norm_a = 0.0f;
        float norm_b = 0.0f;

        for (size_t i = 0; i < a.size(); ++i) {
            dot_product += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }

        norm_a = std::sqrt(norm_a);
        norm_b = std::sqrt(norm_b);

        if (norm_a == 0.0f || norm_b == 0.0f) {
            return 0.0f;
        }

        return dot_product / (norm_a * norm_b);
    }
};

int main() {
    std::cout << "🧠 UMICP C++ Embedding Communication Demo" << std::endl;
    std::cout << "=========================================" << std::endl;

    TransformerCommunication communication;
    EmbeddingSerializer serializer;

    // Example 1: BERT-style embedding transfer
    std::cout << "\n1. BERT-Style Embedding Transfer" << std::endl;
    std::cout << "-------------------------------" << std::endl;

    EmbeddingData bert_embedding = serializer.createBertEmbedding(128, 768);
    umicp_envelope_t* bert_envelope = communication.createEmbeddingEnvelope(
        "bert-encoder", "classification-head", bert_embedding);

    std::cout << "✅ BERT embedding envelope created" << std::endl;
    std::cout << "   Model: " << bert_embedding.model_name << std::endl;
    std::cout << "   Layer: " << bert_embedding.layer_index << std::endl;
    std::cout << "   Sequence length: " << bert_embedding.sequence_length << std::endl;
    std::cout << "   Embedding dim: " << bert_embedding.embedding_dim << std::endl;
    std::cout << "   Data size: " << (bert_embedding.embeddings.size() * sizeof(float) / 1024) << " KB" << std::endl;

    umicp_free_envelope(bert_envelope);

    // Example 2: GPT-style attention sharing
    std::cout << "\n2. GPT-Style Attention Sharing" << std::endl;
    std::cout << "----------------------------" << std::endl;

    size_t sequence_length = 512;
    size_t num_heads = 12;
    std::vector<float> attention_weights = serializer.createGptAttention(sequence_length, num_heads);

    umicp_envelope_t* attention_envelope = communication.createAttentionEnvelope(
        "gpt-decoder", "attention-aggregator", attention_weights, sequence_length, num_heads);

    std::cout << "✅ GPT attention envelope created" << std::endl;
    std::cout << "   Sequence length: " << sequence_length << std::endl;
    std::cout << "   Number of heads: " << num_heads << std::endl;
    std::cout << "   Attention matrix size: " << attention_weights.size()
              << " (" << (attention_weights.size() * sizeof(float) / (1024.0 * 1024.0)) << " MB)" << std::endl;

    umicp_free_envelope(attention_envelope);

    // Example 3: T5-style encoder-decoder communication
    std::cout << "\n3. T5-Style Encoder-Decoder Communication" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    EmbeddingData t5_embedding = serializer.createT5EncoderOutput(256, 512);
    umicp_envelope_t* t5_envelope = communication.createEmbeddingEnvelope(
        "t5-encoder", "t5-decoder", t5_embedding);

    std::cout << "✅ T5 encoder-decoder envelope created" << std::endl;
    std::cout << "   Encoder output size: " << (t5_embedding.embeddings.size() * sizeof(float) / 1024) << " KB" << std::endl;

    umicp_free_envelope(t5_envelope);

    // Example 4: Embedding similarity search
    std::cout << "\n4. Embedding Similarity Search" << std::endl;
    std::cout << "------------------------------" << std::endl;

    // Create embedding database
    const size_t database_size = 1000;
    const size_t embedding_dim = 384;
    std::vector<std::vector<float>> embedding_database;

    for (size_t i = 0; i < database_size; ++i) {
        std::vector<float> embedding(embedding_dim);
        for (size_t j = 0; j < embedding_dim; ++j) {
            embedding[j] = std::sin(i * 0.1f) + std::cos(j * 0.01f) * 0.1f;
        }
        embedding_database.push_back(embedding);
    }

    // Create query embedding
    std::vector<float> query_embedding(embedding_dim);
    for (size_t j = 0; j < embedding_dim; ++j) {
        query_embedding[j] = std::sin(0.05f) + std::cos(j * 0.01f) * 0.1f;
    }

    auto search_start = std::chrono::high_resolution_clock::now();
    auto similar_results = communication.findSimilarEmbeddings(query_embedding, embedding_database, 5);
    auto search_end = std::chrono::high_resolution_clock::now();

    auto search_duration = std::chrono::duration_cast<std::chrono::milliseconds>(search_end - search_start);

    std::cout << "✅ Similarity search completed" << std::endl;
    std::cout << "   Database size: " << database_size << " embeddings" << std::endl;
    std::cout << "   Search time: " << search_duration.count() << " ms" << std::endl;
    std::cout << "   Results per second: " << (database_size / (search_duration.count() / 1000.0)) << std::endl;
    std::cout << "   Top 5 similar embeddings:" << std::endl;

    for (size_t i = 0; i < similar_results.size(); ++i) {
        std::cout << "     " << (i + 1) << ". Index " << similar_results[i].first
                  << ": similarity " << similar_results[i].second << std::endl;
    }

    // Example 5: Federated learning embedding aggregation
    std::cout << "\n5. Federated Learning Embedding Aggregation" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

    const size_t num_clients = 5;
    const size_t shared_embedding_dim = 512;
    std::vector<std::vector<float>> client_embeddings;

    for (size_t client = 0; client < num_clients; ++client) {
        std::vector<float> client_embedding(shared_embedding_dim);
        for (size_t i = 0; i < shared_embedding_dim; ++i) {
            client_embedding[i] = std::sin(i * 0.01f) + client * 0.01f;
        }
        client_embeddings.push_back(client_embedding);
    }

    std::vector<float> aggregated_embedding = communication.aggregateEmbeddings(client_embeddings);

    float magnitude = 0.0f;
    for (float val : aggregated_embedding) {
        magnitude += val * val;
    }
    magnitude = std::sqrt(magnitude);

    std::cout << "✅ Federated embedding aggregation completed" << std::endl;
    std::cout << "   Clients: " << num_clients << std::endl;
    std::cout << "   Embedding dimension: " << shared_embedding_dim << std::endl;
    std::cout << "   Aggregated embedding magnitude: " << magnitude << std::endl;

    std::cout << "\n🎉 Embedding Communication Demo Completed!" << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "💡 Key Benefits for AI Model Communication:" << std::endl;
    std::cout << "   • Efficient binary serialization of embeddings" << std::endl;
    std::cout << "   • Type-safe transformer state transfer" << std::endl;
    std::cout << "   • Optimized attention weight sharing" << std::endl;
    std::cout << "   • Real-time embedding similarity search" << std::endl;
    std::cout << "   • Federated learning support" << std::endl;
    std::cout << "   • Memory-efficient large model handling" << std::endl;

    return 0;
}
