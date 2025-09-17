/**
 * UMICP C++ Bindings - Machine Learning Demo
 *
 * This example demonstrates machine learning workflows using UMICP envelopes
 * for distributed training, model serialization, and federated learning.
 */

#include <umicp.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

class NeuralNetwork {
private:
    std::vector<std::vector<float>> weights_;
    std::vector<float> biases_;
    size_t input_size_;
    size_t hidden_size_;
    size_t output_size_;

public:
    NeuralNetwork(size_t input_size, size_t hidden_size, size_t output_size)
        : input_size_(input_size), hidden_size_(hidden_size), output_size_(output_size) {

        // Initialize weights with random values
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dist(0.0f, 0.1f);

        // Input to hidden weights
        weights_.push_back(std::vector<float>(input_size * hidden_size));
        for (auto& w : weights_[0]) {
            w = dist(gen);
        }

        // Hidden to output weights
        weights_.push_back(std::vector<float>(hidden_size * output_size));
        for (auto& w : weights_[1]) {
            w = dist(gen);
        }

        // Biases
        biases_ = std::vector<float>(hidden_size + output_size, 0.0f);
    }

    std::vector<float> forward(const std::vector<float>& input) {
        // Forward pass
        std::vector<float> hidden(hidden_size_);

        // Input to hidden
        for (size_t i = 0; i < hidden_size_; ++i) {
            float sum = biases_[i];
            for (size_t j = 0; j < input_size_; ++j) {
                sum += input[j] * weights_[0][i * input_size_ + j];
            }
            hidden[i] = std::max(0.0f, sum); // ReLU activation
        }

        // Hidden to output
        std::vector<float> output(output_size_);
        for (size_t i = 0; i < output_size_; ++i) {
            float sum = biases_[hidden_size_ + i];
            for (size_t j = 0; j < hidden_size_; ++j) {
                sum += hidden[j] * weights_[1][i * hidden_size_ + j];
            }
            output[i] = sum; // Linear activation for output
        }

        return output;
    }

    void update_weights(const std::vector<std::vector<float>>& gradients,
                       const std::vector<float>& bias_gradients,
                       float learning_rate) {
        // Update weights using gradients
        for (size_t layer = 0; layer < weights_.size(); ++layer) {
            for (size_t i = 0; i < weights_[layer].size(); ++i) {
                weights_[layer][i] -= learning_rate * gradients[layer][i];
            }
        }

        // Update biases
        for (size_t i = 0; i < biases_.size(); ++i) {
            biases_[i] -= learning_rate * bias_gradients[i];
        }
    }

    // Serialize model to envelope
    umicp_envelope_t* serialize_to_envelope() {
        // Create envelope for model weights
        umicp_envelope_t* envelope = umicp_create_envelope();

        umicp_set_from(envelope, "ml-model");
        umicp_set_to(envelope, "model-registry");
        umicp_set_operation(envelope, UMICP_OPERATION_DATA);
        umicp_set_message_id(envelope, "model-weights-v1.0");

        // Serialize weights and biases
        std::string weights_data;
        for (const auto& layer : weights_) {
            for (float w : layer) {
                weights_data += std::to_string(w) + ",";
            }
            weights_data += ";";
        }

        std::string bias_data;
        for (float b : biases_) {
            bias_data += std::to_string(b) + ",";
        }

        umicp_set_capability(envelope, "model_type", "neural_network");
        umicp_set_capability(envelope, "input_size", std::to_string(input_size_).c_str());
        umicp_set_capability(envelope, "hidden_size", std::to_string(hidden_size_).c_str());
        umicp_set_capability(envelope, "output_size", std::to_string(output_size_).c_str());
        umicp_set_capability(envelope, "weights", weights_data.c_str());
        umicp_set_capability(envelope, "biases", bias_data.c_str());

        return envelope;
    }

    // Deserialize model from envelope
    static NeuralNetwork deserialize_from_envelope(umicp_envelope_t* envelope) {
        const char* input_size_str = umicp_get_capability(envelope, "input_size");
        const char* hidden_size_str = umicp_get_capability(envelope, "hidden_size");
        const char* output_size_str = umicp_get_capability(envelope, "output_size");

        size_t input_size = std::stoul(input_size_str);
        size_t hidden_size = std::stoul(hidden_size_str);
        size_t output_size = std::stoul(output_size_str);

        NeuralNetwork nn(input_size, hidden_size, output_size);

        // Parse weights
        const char* weights_str = umicp_get_capability(envelope, "weights");
        std::string weights_data(weights_str);
        std::vector<std::string> layers = split(weights_data, ';');

        for (size_t i = 0; i < layers.size() - 1; ++i) { // -1 because of trailing ;
            std::vector<std::string> weight_vals = split(layers[i], ',');
            for (size_t j = 0; j < weight_vals.size() - 1; ++j) { // -1 because of trailing ,
                nn.weights_[i][j] = std::stof(weight_vals[j]);
            }
        }

        // Parse biases
        const char* bias_str = umicp_get_capability(envelope, "biases");
        std::vector<std::string> bias_vals = split(std::string(bias_str), ',');
        for (size_t i = 0; i < bias_vals.size() - 1; ++i) {
            nn.biases_[i] = std::stof(bias_vals[i]);
        }

        return nn;
    }

private:
    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

class FederatedLearningCoordinator {
private:
    std::vector<NeuralNetwork> global_models_;
    std::mutex model_mutex_;
    std::atomic<size_t> active_clients_{0};
    size_t current_round_{0};

public:
    FederatedLearningCoordinator() {
        // Initialize with a base model
        global_models_.emplace_back(784, 128, 10); // MNIST-like architecture
    }

    // Handle model updates from clients
    umicp_envelope_t* handle_model_update(umicp_envelope_t* update_envelope) {
        std::lock_guard<std::mutex> lock(model_mutex_);

        std::cout << "📨 Received model update from: " << umicp_get_from(update_envelope) << std::endl;

        // Parse client model from envelope
        NeuralNetwork client_model = NeuralNetwork::deserialize_from_envelope(update_envelope);

        // Simple federated averaging (FedAvg)
        const float learning_rate = 0.1f;

        for (size_t layer = 0; layer < global_models_[0].weights_.size(); ++layer) {
            for (size_t i = 0; i < global_models_[0].weights_[layer].size(); ++i) {
                float diff = client_model.weights_[layer][i] - global_models_[0].weights_[layer][i];
                global_models_[0].weights_[layer][i] += diff * learning_rate;
            }
        }

        // Update biases
        for (size_t i = 0; i < global_models_[0].biases_.size(); ++i) {
            float diff = client_model.biases_[i] - global_models_[0].biases_[i];
            global_models_[0].biases_[i] += diff * learning_rate;
        }

        active_clients_++;

        // Create acknowledgment envelope
        umicp_envelope_t* ack_envelope = umicp_create_envelope();
        umicp_set_from(ack_envelope, "federated-coordinator");
        umicp_set_to(ack_envelope, umicp_get_from(update_envelope));
        umicp_set_operation(ack_envelope, UMICP_OPERATION_ACK);
        umicp_set_message_id(ack_envelope, "model-update-ack");

        umicp_set_capability(ack_envelope, "status", "model_aggregated");
        umicp_set_capability(ack_envelope, "active_clients", std::to_string(active_clients_.load()).c_str());
        umicp_set_capability(ack_envelope, "current_round", std::to_string(current_round_).c_str());

        return ack_envelope;
    }

    // Get current global model
    umicp_envelope_t* get_global_model(const char* client_id) {
        std::lock_guard<std::mutex> lock(model_mutex_);

        umicp_envelope_t* model_envelope = global_models_[0].serialize_to_envelope();
        umicp_set_to(model_envelope, client_id);
        umicp_set_capability(model_envelope, "training_round", std::to_string(current_round_).c_str());

        return model_envelope;
    }

    void advance_round() {
        current_round_++;
        std::cout << "🔄 Advanced to training round " << current_round_ << std::endl;
    }

    size_t get_active_clients() const {
        return active_clients_.load();
    }
};

class FederatedLearningClient {
private:
    NeuralNetwork local_model_;
    std::string client_id_;
    std::vector<std::vector<float>> training_data_;
    std::vector<size_t> training_labels_;

public:
    FederatedLearningClient(const std::string& client_id, size_t input_size, size_t hidden_size, size_t output_size)
        : client_id_(client_id), local_model_(input_size, hidden_size, output_size) {
        generate_training_data();
    }

    void update_model_from_global(umicp_envelope_t* global_model_envelope) {
        local_model_ = NeuralNetwork::deserialize_from_envelope(global_model_envelope);
        std::cout << "📥 " << client_id_ << " updated model from global" << std::endl;
    }

    umicp_envelope_t* perform_local_training() {
        std::cout << "🤖 " << client_id_ << " starting local training..." << std::endl;

        const size_t epochs = 5;
        const float learning_rate = 0.01f;

        for (size_t epoch = 0; epoch < epochs; ++epoch) {
            float epoch_loss = 0.0f;

            for (size_t i = 0; i < training_data_.size(); ++i) {
                const auto& input = training_data_[i];
                size_t target = training_labels_[i];

                // Forward pass
                auto output = local_model_.forward(input);

                // Calculate loss (cross-entropy)
                float max_output = *std::max_element(output.begin(), output.end());
                std::vector<float> exp_outputs;
                float sum_exp = 0.0f;

                for (float val : output) {
                    float exp_val = std::exp(val - max_output);
                    exp_outputs.push_back(exp_val);
                    sum_exp += exp_val;
                }

                std::vector<float> probabilities;
                for (float exp_val : exp_outputs) {
                    probabilities.push_back(exp_val / sum_exp);
                }

                float loss = -std::log(probabilities[target]);
                epoch_loss += loss;

                // Backward pass (simplified)
                std::vector<float> gradients(output.size(), 0.0f);
                for (size_t j = 0; j < output.size(); ++j) {
                    gradients[j] = probabilities[j] - (j == target ? 1.0f : 0.0f);
                }

                // Update weights (simplified gradient descent)
                for (size_t layer = 0; layer < local_model_.weights_.size(); ++layer) {
                    // Simplified weight update - in practice you'd compute proper gradients
                    for (size_t j = 0; j < local_model_.weights_[layer].size(); ++j) {
                        local_model_.weights_[layer][j] -= learning_rate * 0.001f; // Simplified
                    }
                }
            }

            std::cout << "   Epoch " << (epoch + 1) << " - Loss: " << (epoch_loss / training_data_.size()) << std::endl;
        }

        // Create model update envelope
        umicp_envelope_t* update_envelope = local_model_.serialize_to_envelope();
        umicp_set_from(update_envelope, client_id_.c_str());
        umicp_set_to(update_envelope, "federated-coordinator");
        umicp_set_message_id(update_envelope, (client_id_ + "-update").c_str());

        umicp_set_capability(update_envelope, "training_samples", std::to_string(training_data_.size()).c_str());
        umicp_set_capability(update_envelope, "client_id", client_id_.c_str());

        return update_envelope;
    }

private:
    void generate_training_data() {
        // Generate synthetic training data (simplified MNIST-like)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        const size_t samples = 100;

        for (size_t i = 0; i < samples; ++i) {
            std::vector<float> sample(784); // 28x28 flattened
            for (size_t j = 0; j < 784; ++j) {
                sample[j] = dist(gen);
            }
            training_data_.push_back(sample);

            // Random label 0-9
            training_labels_.push_back(i % 10);
        }
    }
};

int main() {
    std::cout << "🧠 UMICP C++ Machine Learning Demo" << std::endl;
    std::cout << "==================================" << std::endl;

    // Initialize coordinator
    FederatedLearningCoordinator coordinator;

    // Initialize clients
    std::vector<FederatedLearningClient> clients;
    const size_t num_clients = 3;

    for (size_t i = 0; i < num_clients; ++i) {
        clients.emplace_back("client-" + std::to_string(i + 1), 784, 128, 10);
    }

    std::cout << "🚀 Starting federated learning with " << num_clients << " clients" << std::endl;
    std::cout << std::endl;

    // Simulate multiple rounds of federated learning
    const size_t rounds = 3;

    for (size_t round = 1; round <= rounds; ++round) {
        std::cout << "🔄 Round " << round << "/" << rounds << std::endl;
        std::cout << "-------------------" << std::endl;

        // Send global model to clients
        std::vector<umicp_envelope_t*> model_envelopes;
        for (auto& client : clients) {
            umicp_envelope_t* model_env = coordinator.get_global_model(client.client_id_.c_str());
            client.update_model_from_global(model_env);
            model_envelopes.push_back(model_env);
        }

        std::cout << "📤 Broadcasted global model to " << clients.size() << " clients" << std::endl;

        // Each client performs local training
        std::vector<umicp_envelope_t*> update_envelopes;
        for (auto& client : clients) {
            umicp_envelope_t* update_env = client.perform_local_training();
            update_envelopes.push_back(update_env);
        }

        // Coordinator aggregates updates
        for (auto update_env : update_envelopes) {
            umicp_envelope_t* ack_env = coordinator.handle_model_update(update_env);
            // In a real system, you'd send this acknowledgment back to the client
            umicp_free_envelope(ack_env);
            umicp_free_envelope(update_env);
        }

        // Advance to next round
        coordinator.advance_round();

        std::cout << "📊 Round " << round << " completed - Active clients: " << coordinator.get_active_clients() << std::endl;
        std::cout << std::endl;

        // Clean up model envelopes
        for (auto env : model_envelopes) {
            umicp_free_envelope(env);
        }
    }

    // Demonstrate model serialization/deserialization
    std::cout << "💾 Model Serialization Test" << std::endl;
    std::cout << "===========================" << std::endl;

    NeuralNetwork test_model(10, 5, 2);
    umicp_envelope_t* serialized_model = test_model.serialize_to_envelope();

    std::cout << "✅ Model serialized to envelope" << std::endl;
    std::cout << "   From: " << umicp_get_from(serialized_model) << std::endl;
    std::cout << "   To: " << umicp_get_to(serialized_model) << std::endl;
    std::cout << "   Message ID: " << umicp_get_message_id(serialized_model) << std::endl;

    // Deserialize model
    NeuralNetwork deserialized_model = NeuralNetwork::deserialize_from_envelope(serialized_model);

    // Test both models with same input
    std::vector<float> test_input(10, 0.5f);
    auto original_output = test_model.forward(test_input);
    auto deserialized_output = deserialized_model.forward(test_input);

    std::cout << "✅ Model deserialized successfully" << std::endl;
    std::cout << "   Original output[0]: " << original_output[0] << std::endl;
    std::cout << "   Deserialized output[0]: " << deserialized_output[0] << std::endl;

    // Clean up
    umicp_free_envelope(serialized_model);

    std::cout << std::endl << "🎉 Machine Learning Demo Completed!" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "💡 Demonstrated:" << std::endl;
    std::cout << "   • Neural network implementation" << std::endl;
    std::cout << "   • Federated learning workflow" << std::endl;
    std::cout << "   • Model serialization in UMICP envelopes" << std::endl;
    std::cout << "   • Distributed training coordination" << std::endl;
    std::cout << "   • Real-time model updates" << std::endl;

    return 0;
}
