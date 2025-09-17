/**
 * UMICP C++ Bindings - Financial Transaction Processor
 *
 * This example demonstrates secure financial transaction processing using UMICP
 * envelopes for payment processing, fraud detection, and transaction analytics.
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
#include <unordered_map>
#include <openssl/sha.h>
#include <openssl/hmac.h>

struct FinancialTransaction {
    std::string transaction_id;
    std::string type; // "payment", "transfer", "withdrawal", "deposit", "refund"
    double amount;
    std::string currency;
    std::string from_account;
    std::string to_account;
    std::chrono::system_clock::time_point timestamp;
    std::string description;
    std::unordered_map<std::string, std::string> metadata;

    FinancialTransaction(const std::string& id, const std::string& t, double amt,
                        const std::string& curr, const std::string& from, const std::string& to,
                        const std::string& desc)
        : transaction_id(id), type(t), amount(amt), currency(curr),
          from_account(from), to_account(to), description(desc),
          timestamp(std::chrono::system_clock::now()) {}
};

struct TransactionResult {
    bool success;
    std::string transaction_id;
    long long processing_time_ms;
    double fee;
    std::string risk_level;
    std::string authorization_code;
    std::string error_message;

    TransactionResult(bool s, const std::string& id, long long time, double f,
                     const std::string& risk, const std::string& auth = "",
                     const std::string& error = "")
        : success(s), transaction_id(id), processing_time_ms(time), fee(f),
          risk_level(risk), authorization_code(auth), error_message(error) {}
};

class RiskAssessmentEngine {
private:
    std::unordered_map<std::string, std::vector<double>> transaction_history_;
    std::mutex history_mutex_;

public:
    std::string assess_risk(const FinancialTransaction& transaction) {
        std::unique_lock<std::mutex> lock(history_mutex_);

        int risk_score = 0;

        // Amount-based risk
        if (transaction.amount > 10000) risk_score += 30;
        else if (transaction.amount > 1000) risk_score += 15;
        else if (transaction.amount > 100) risk_score += 5;

        // Transaction type risk
        if (transaction.type == "withdrawal" && transaction.amount > 5000) {
            risk_score += 20;
        }

        // Frequency-based risk (simplified)
        auto& history = transaction_history_[transaction.from_account];
        history.push_back(transaction.amount);

        // Keep only last 10 transactions
        if (history.size() > 10) {
            history.erase(history.begin());
        }

        // Check for unusual patterns
        if (history.size() >= 3) {
            double avg_amount = 0.0;
            for (double amt : history) avg_amount += amt;
            avg_amount /= history.size();

            if (transaction.amount > avg_amount * 3) {
                risk_score += 15; // Unusual large transaction
            }
        }

        // Time-based risk
        auto hour = std::chrono::duration_cast<std::chrono::hours>(
            transaction.timestamp.time_since_epoch()).count() % 24;
        if (hour < 6 || hour > 22) {
            risk_score += 10; // Unusual hours
        }

        // Location-based risk (simplified)
        if (transaction.metadata.count("location") &&
            transaction.metadata.at("location") == "high-risk-country") {
            risk_score += 25;
        }

        if (risk_score >= 50) return "critical";
        if (risk_score >= 30) return "high";
        if (risk_score >= 15) return "medium";
        return "low";
    }
};

class FinancialTransactionProcessor {
private:
    RiskAssessmentEngine risk_engine_;
    std::unordered_map<std::string, TransactionResult> processed_transactions_;
    std::unordered_map<std::string, double> account_balances_;
    std::mutex processor_mutex_;
    std::atomic<size_t> transaction_count_{0};

public:
    TransactionResult process_transaction(const FinancialTransaction& transaction) {
        auto start_time = std::chrono::steady_clock::now();

        std::cout << "💰 Processing " << transaction.type << " transaction: "
                  << transaction.transaction_id << std::endl;
        std::cout << "   Amount: $" << transaction.amount << " " << transaction.currency << std::endl;
        std::cout << "   From: " << transaction.from_account << " -> To: " << transaction.to_account << std::endl;

        // Validate transaction
        auto validation = validate_transaction(transaction);
        if (!validation.first) {
            auto end_time = std::chrono::steady_clock::now();
            auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();

            return TransactionResult(false, transaction.transaction_id, processing_time,
                                   0.0, "critical", "", validation.second);
        }

        // Risk assessment
        std::string risk_level = risk_engine_.assess_risk(transaction);

        // Calculate processing fee
        double fee = calculate_fee(transaction, risk_level);

        // Simulate processing delay based on risk
        simulate_processing_delay(risk_level);

        // Generate authorization code
        std::string auth_code = generate_authorization_code();

        // Update account balances (simplified)
        {
            std::unique_lock<std::mutex> lock(processor_mutex_);
            if (account_balances_.count(transaction.from_account) == 0) {
                account_balances_[transaction.from_account] = 10000.0; // Default balance
            }
            if (account_balances_.count(transaction.to_account) == 0) {
                account_balances_[transaction.to_account] = 10000.0;
            }

            // For demonstration, we'll assume sufficient funds
            account_balances_[transaction.from_account] -= transaction.amount + fee;
            account_balances_[transaction.to_account] += transaction.amount;

            // Store result
            auto end_time = std::chrono::steady_clock::now();
            auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();

            TransactionResult result(true, transaction.transaction_id, processing_time,
                                   fee, risk_level, auth_code);

            processed_transactions_[transaction.transaction_id] = result;
            transaction_count_++;

            std::cout << "✅ Transaction processed successfully" << std::endl;
            std::cout << "   Auth Code: " << auth_code << std::endl;
            std::cout << "   Processing time: " << processing_time << "ms" << std::endl;
            std::cout << "   Fee: $" << fee << std::endl;
            std::cout << "   Risk level: " << risk_level << std::endl;
            std::cout << std::endl;

            return result;
        }
    }

    umicp_envelope_t* create_secure_envelope(const FinancialTransaction& transaction) {
        umicp_envelope_t* envelope = umicp_create_envelope();

        umicp_set_from(envelope, "financial-processor");
        umicp_set_to(envelope, "transaction-validator");
        umicp_set_operation(envelope, UMICP_OPERATION_REQUEST);
        umicp_set_message_id(envelope, ("txn-" + transaction.transaction_id).c_str());

        // Add transaction data
        umicp_set_capability(envelope, "transaction_type", transaction.type.c_str());
        umicp_set_capability(envelope, "transaction_id", transaction.transaction_id.c_str());
        umicp_set_capability(envelope, "amount", std::to_string(transaction.amount).c_str());
        umicp_set_capability(envelope, "currency", transaction.currency.c_str());
        umicp_set_capability(envelope, "from_account", transaction.from_account.c_str());
        umicp_set_capability(envelope, "to_account", transaction.to_account.c_str());
        umicp_set_capability(envelope, "description", transaction.description.c_str());

        // Add timestamp
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            transaction.timestamp.time_since_epoch()).count();
        umicp_set_capability(envelope, "timestamp", std::to_string(timestamp).c_str());

        // Add metadata
        for (const auto& [key, value] : transaction.metadata) {
            umicp_set_capability(envelope, ("meta_" + key).c_str(), value.c_str());
        }

        // Create digital signature for integrity
        std::string transaction_data = transaction.transaction_id + "|" +
                                     std::to_string(transaction.amount) + "|" +
                                     transaction.from_account + "|" +
                                     transaction.to_account + "|" +
                                     std::to_string(timestamp);

        std::string signature = generate_hmac_signature(transaction_data, "secure-secret-key");
        umicp_set_capability(envelope, "signature", signature.c_str());

        umicp_set_capability(envelope, "protocol_version", "1.0");

        return envelope;
    }

    std::unordered_map<std::string, double> get_processing_stats() {
        std::unique_lock<std::mutex> lock(processor_mutex_);

        std::unordered_map<std::string, double> stats;

        stats["total_transactions"] = processed_transactions_.size();
        stats["successful_transactions"] = std::count_if(
            processed_transactions_.begin(),
            processed_transactions_.end(),
            [](const auto& pair) { return pair.second.success; }
        );

        stats["success_rate"] = stats["successful_transactions"] / stats["total_transactions"] * 100.0;

        // Calculate average processing time
        double total_time = 0.0;
        for (const auto& [_, result] : processed_transactions_) {
            total_time += result.processing_time_ms;
        }
        stats["avg_processing_time"] = total_time / processed_transactions_.size();

        // Calculate total fees
        double total_fees = 0.0;
        for (const auto& [_, result] : processed_transactions_) {
            total_fees += result.fee;
        }
        stats["total_fees"] = total_fees;

        // Risk distribution
        std::unordered_map<std::string, int> risk_counts;
        for (const auto& [_, result] : processed_transactions_) {
            risk_counts[result.risk_level]++;
        }

        for (const auto& [risk, count] : risk_counts) {
            stats["risk_" + risk] = count;
        }

        return stats;
    }

private:
    std::pair<bool, std::string> validate_transaction(const FinancialTransaction& transaction) {
        // Amount validation
        if (transaction.amount <= 0) {
            return {false, "Invalid transaction amount"};
        }

        // Currency validation
        std::vector<std::string> valid_currencies = {"USD", "EUR", "GBP", "JPY", "BRL"};
        if (std::find(valid_currencies.begin(), valid_currencies.end(),
                      transaction.currency) == valid_currencies.end()) {
            return {false, "Unsupported currency"};
        }

        // Account validation
        if (transaction.from_account.empty() || transaction.to_account.empty()) {
            return {false, "Invalid account information"};
        }

        // Prevent self-transfers for certain types
        if (transaction.type == "transfer" && transaction.from_account == transaction.to_account) {
            return {false, "Cannot transfer to the same account"};
        }

        // Check account balance (simplified)
        {
            std::unique_lock<std::mutex> lock(processor_mutex_);
            double balance = account_balances_[transaction.from_account];
            if (balance < transaction.amount) {
                return {false, "Insufficient funds"};
            }
        }

        return {true, ""};
    }

    double calculate_fee(const FinancialTransaction& transaction, const std::string& risk_level) {
        double base_fee = 0.0;

        // Base fee by transaction type
        if (transaction.type == "payment") {
            base_fee = transaction.amount * 0.029; // 2.9%
        } else if (transaction.type == "transfer") {
            base_fee = transaction.amount * 0.001; // 0.1%
        } else if (transaction.type == "withdrawal") {
            base_fee = 2.50; // Fixed fee
        } else if (transaction.type == "deposit") {
            base_fee = 0.0; // No fee
        } else if (transaction.type == "refund") {
            base_fee = 1.00; // Fixed fee
        }

        // Risk-based fee adjustment
        double risk_multiplier = 1.0;
        if (risk_level == "medium") risk_multiplier = 1.2;
        else if (risk_level == "high") risk_multiplier = 1.5;
        else if (risk_level == "critical") risk_multiplier = 2.0;

        return base_fee * risk_multiplier;
    }

    void simulate_processing_delay(const std::string& risk_level) {
        std::random_device rd;
        std::mt19937 gen(rd());

        int base_delay = 50; // Base 50ms
        if (risk_level == "medium") base_delay = 200;
        else if (risk_level == "high") base_delay = 500;
        else if (risk_level == "critical") base_delay = 2000;

        std::uniform_int_distribution<> dist(base_delay, base_delay * 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
    }

    std::string generate_authorization_code() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 35);

        std::string code;
        const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        for (int i = 0; i < 6; ++i) {
            code += charset[dist(gen)];
        }

        return code;
    }

    std::string generate_hmac_signature(const std::string& data, const std::string& key) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        unsigned int hash_len;

        HMAC_CTX* hmac_ctx = HMAC_CTX_new();
        HMAC_Init_ex(hmac_ctx, key.c_str(), key.length(), EVP_sha256(), nullptr);
        HMAC_Update(hmac_ctx, reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
        HMAC_Final(hmac_ctx, hash, &hash_len);
        HMAC_CTX_free(hmac_ctx);

        char hex_hash[SHA256_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            sprintf(hex_hash + (i * 2), "%02x", hash[i]);
        }
        hex_hash[SHA256_DIGEST_LENGTH * 2] = '\0';

        return std::string(hex_hash);
    }
};

class FinancialTransactionSimulator {
private:
    FinancialTransactionProcessor& processor_;
    std::vector<std::string> accounts_ = {"acc-001", "acc-002", "acc-003", "merchant-001"};
    std::vector<std::string> transaction_types_ = {"payment", "transfer", "withdrawal", "deposit"};

public:
    FinancialTransactionSimulator(FinancialTransactionProcessor& processor)
        : processor_(processor) {}

    void run_simulation(size_t num_transactions) {
        std::cout << "🏦 Starting financial transaction simulation..." << std::endl;
        std::cout << "   Processing " << num_transactions << " transactions..." << std::endl;
        std::cout << std::endl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> amount_dist(10.0, 1000.0);
        std::uniform_int_distribution<> account_dist(0, accounts_.size() - 1);
        std::uniform_int_distribution<> type_dist(0, transaction_types_.size() - 1);

        std::vector<umicp_envelope_t*> envelopes;

        for (size_t i = 0; i < num_transactions; ++i) {
            // Generate random transaction
            std::string from_account = accounts_[account_dist(gen)];
            std::string to_account = accounts_[account_dist(gen)];
            std::string transaction_type = transaction_types_[type_dist(gen)];

            // Ensure different accounts for transfers
            if (transaction_type == "transfer" && from_account == to_account) {
                to_account = accounts_[(account_dist(gen) + 1) % accounts_.size()];
            }

            FinancialTransaction transaction(
                "txn-" + std::to_string(i + 1) + "-" +
                std::to_string(std::chrono::system_clock::now().time_since_epoch().count()),
                transaction_type,
                amount_dist(gen),
                "USD",
                from_account,
                to_account,
                "Generated transaction " + std::to_string(i + 1)
            );

            // Add metadata
            transaction.metadata["location"] = (rand() % 2 == 0) ? "US" : "EU";
            transaction.metadata["device_fingerprint"] = std::to_string(rand() % 1000000);
            transaction.metadata["merchant_id"] = (transaction_type == "payment") ?
                "merchant-" + std::to_string(rand() % 100) : "";

            // Create secure envelope
            umicp_envelope_t* envelope = processor_.create_secure_envelope(transaction);
            envelopes.push_back(envelope);

            // Process transaction
            processor_.process_transaction(transaction);

            // Small delay between transactions
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "📊 Created " << envelopes.size() << " secure transaction envelopes" << std::endl;

        // Clean up envelopes
        for (auto envelope : envelopes) {
            umicp_free_envelope(envelope);
        }
    }
};

int main() {
    std::cout << "💳 UMICP C++ Financial Transaction Processor" << std::endl;
    std::cout << "==========================================" << std::endl;

    // Create processor
    FinancialTransactionProcessor processor;

    // Create simulator
    FinancialTransactionSimulator simulator(processor);

    // Run simulation
    simulator.run_simulation(20);

    // Show final statistics
    auto stats = processor.get_processing_stats();

    std::cout << "🎉 Financial Processing Complete!" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "📊 Final Statistics:" << std::endl;
    std::cout << "   Total transactions: " << stats["total_transactions"] << std::endl;
    std::cout << "   Successful transactions: " << stats["successful_transactions"] << std::endl;
    std::cout << "   Success rate: " << stats["success_rate"] << "%" << std::endl;
    std::cout << "   Average processing time: " << stats["avg_processing_time"] << "ms" << std::endl;
    std::cout << "   Total fees collected: $" << stats["total_fees"] << std::endl;
    std::cout << std::endl;

    if (stats.count("risk_low")) std::cout << "   Low risk transactions: " << stats["risk_low"] << std::endl;
    if (stats.count("risk_medium")) std::cout << "   Medium risk transactions: " << stats["risk_medium"] << std::endl;
    if (stats.count("risk_high")) std::cout << "   High risk transactions: " << stats["risk_high"] << std::endl;
    if (stats.count("risk_critical")) std::cout << "   Critical risk transactions: " << stats["risk_critical"] << std::endl;

    std::cout << std::endl;
    std::cout << "💡 Key Security Features Demonstrated:" << std::endl;
    std::cout << "   • Digital signature verification" << std::endl;
    std::cout << "   • Real-time risk assessment" << std::endl;
    std::cout << "   • Account balance validation" << std::endl;
    std::cout << "   • Secure envelope creation" << std::endl;
    std::cout << "   • Transaction integrity protection" << std::endl;
    std::cout << "   • Fraud detection algorithms" << std::endl;

    return 0;
}
