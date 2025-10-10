/**
 * UMICP StreamableHTTP Client Implementation
 */

#include "http_client.h"
#include "serialization.h"
#include <curl/curl.h>
#include <sstream>
#include <cstring>
#include <numeric>

namespace umicp {

// CURL write callback
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* buffer = static_cast<std::vector<uint8_t>*>(userp);

    const uint8_t* data = static_cast<const uint8_t*>(contents);
    buffer->insert(buffer->end(), data, data + total_size);

    return total_size;
}

// CURL header callback
static size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total_size = size * nitems;
    auto* headers = static_cast<std::unordered_map<std::string, std::string>*>(userdata);

    std::string header_line(buffer, total_size);

    // Parse header
    size_t colon_pos = header_line.find(':');
    if (colon_pos != std::string::npos) {
        std::string key = header_line.substr(0, colon_pos);
        std::string value = header_line.substr(colon_pos + 1);

        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        (*headers)[key] = value;
    }

    return total_size;
}

StreamableHTTPClient::StreamableHTTPClient(const HTTPClientConfig& config)
    : config_(config), curl_(nullptr), connected_(false), stats_{} {
    curl_global_init(CURL_GLOBAL_ALL);
}

StreamableHTTPClient::~StreamableHTTPClient() {
    disconnect();
    curl_global_cleanup();
}

Result<void> StreamableHTTPClient::connect() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (connected_) {
        return Result<void>::ok();
    }

    auto result = init_curl();
    if (!result.is_ok()) {
        return result;
    }

    // Verify connection with health check
    auto health = health_check();
    if (!health.is_ok()) {
        cleanup_curl();
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Failed to connect: " + health.error());
    }

    connected_ = true;
    return Result<void>::ok();
}

Result<Envelope> StreamableHTTPClient::send(const Envelope& envelope) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!connected_) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, "Not connected");
    }

    // Serialize envelope to JSON
    auto json_result = EnvelopeProcessor::serialize(envelope);
    if (!json_result.is_ok()) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, "Failed to serialize envelope: " + json_result.error());
    }

    std::string json = *json_result.value;
    std::vector<uint8_t> serialized(json.begin(), json.end());

    // Send POST request
    auto response = perform_request("POST", config_.path, serialized);
    if (!response.is_ok()) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, "Failed to send: " + response.error());
    }

    auto http_response = *response.value;
    if (!http_response.success) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, "HTTP error: " + http_response.error);
    }

    if (http_response.status_code != 200) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, "Server returned status " +
                                     std::to_string(http_response.status_code));
    }

    // Deserialize response envelope from JSON
    std::string response_json(http_response.body.begin(), http_response.body.end());
    auto response_envelope = EnvelopeProcessor::deserialize(response_json);
    if (!response_envelope.is_ok()) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, "Failed to deserialize response: " +
                                    response_envelope.error());
    }

    return Result<Envelope>::ok(*response_envelope.value);
}

Result<HTTPResponse> StreamableHTTPClient::post(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!connected_) {
        return Result<HTTPResponse>::error(ErrorCode::NOT_CONNECTED, "Not connected");
    }

    return perform_request("POST", config_.path, data);
}

Result<HTTPResponse> StreamableHTTPClient::get(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!connected_) {
        return Result<HTTPResponse>::error(ErrorCode::NOT_CONNECTED, "Not connected");
    }

    return perform_request("GET", path.empty() ? config_.path : path);
}

Result<void> StreamableHTTPClient::health_check() {
    auto response = get("/health");
    if (!response.is_ok()) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Health check failed: " + response.error());
    }

    auto http_response = *response.value;
    if (http_response.status_code != 200) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Health check returned status " +
                                std::to_string(http_response.status_code));
    }

    return Result<void>::ok();
}

void StreamableHTTPClient::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!connected_) {
        return;
    }

    cleanup_curl();
    connected_ = false;
}

bool StreamableHTTPClient::is_connected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connected_;
}

void StreamableHTTPClient::set_header(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.headers[key] = value;
}

void StreamableHTTPClient::remove_header(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.headers.erase(key);
}

StreamableHTTPClient::Statistics StreamableHTTPClient::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);

    // Calculate average latency
    Statistics stats = stats_;
    if (!latencies_.empty()) {
        auto sum = std::accumulate(latencies_.begin(), latencies_.end(),
                                   std::chrono::milliseconds(0));
        stats.avg_latency = sum / latencies_.size();
    }

    return stats;
}

void StreamableHTTPClient::reset_statistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Statistics{};
    latencies_.clear();
}

Result<void> StreamableHTTPClient::init_curl() {
    curl_ = curl_easy_init();
    if (!curl_) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Failed to initialize CURL");
    }

    // Set timeout
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, config_.timeout.count());
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, config_.connect_timeout.count());

    // SSL options
    if (!config_.verify_ssl) {
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    if (!config_.ssl_ca_path.empty()) {
        curl_easy_setopt(curl_, CURLOPT_CAINFO, config_.ssl_ca_path.c_str());
    }

    if (!config_.ssl_cert_path.empty()) {
        curl_easy_setopt(curl_, CURLOPT_SSLCERT, config_.ssl_cert_path.c_str());
    }

    if (!config_.ssl_key_path.empty()) {
        curl_easy_setopt(curl_, CURLOPT_SSLKEY, config_.ssl_key_path.c_str());
    }

    // Connection options
    if (config_.keep_alive) {
        curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
    }

    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, config_.max_redirects);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);

    // Buffer size
    curl_easy_setopt(curl_, CURLOPT_BUFFERSIZE, config_.buffer_size);

    return Result<void>::ok();
}

void StreamableHTTPClient::cleanup_curl() {
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
}

Result<HTTPResponse> StreamableHTTPClient::perform_request(
    const std::string& method,
    const std::string& path,
    const std::vector<uint8_t>& body) {

    if (!curl_) {
        return Result<HTTPResponse>::error(ErrorCode::NOT_CONNECTED, "CURL not initialized");
    }

    auto start_time = std::chrono::steady_clock::now();

    HTTPResponse response;

    // Build URL
    std::string url = build_url(path);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    // Set method
    if (method == "GET") {
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.size());
    }

    // Set headers
    struct curl_slist* headers = nullptr;
    for (const auto& [key, value] : config_.headers) {
        std::string header = key + ": " + value;
        headers = curl_slist_append(headers, header.c_str());
    }

    if (method == "POST") {
        headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    }

    if (headers) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    }

    // Set callbacks
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &response.headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl_);

    // Clean up headers
    if (headers) {
        curl_slist_free_all(headers);
    }

    auto end_time = std::chrono::steady_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    if (res != CURLE_OK) {
        response.success = false;
        response.error = curl_easy_strerror(res);
        update_stats(body.size(), 0, latency, false);
        return Result<HTTPResponse>::error(ErrorCode::NOT_CONNECTED, response.error);
    }

    // Get response code
    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    response.status_code = static_cast<int>(http_code);
    response.success = (http_code >= 200 && http_code < 300);

    update_stats(body.size(), response.body.size(), latency, !response.success);

    return Result<HTTPResponse>::ok(response);
}

std::string StreamableHTTPClient::build_url(const std::string& path) const {
    std::string url = config_.base_url;

    // Remove trailing slash from base_url
    if (!url.empty() && url.back() == '/') {
        url.pop_back();
    }

    // Add path
    if (path.empty()) {
        return url;
    }

    if (path[0] != '/') {
        url += '/';
    }
    url += path;

    return url;
}

void StreamableHTTPClient::update_stats(size_t bytes_sent, size_t bytes_received,
                                       std::chrono::milliseconds latency, bool success) {
    stats_.requests_sent++;
    stats_.bytes_sent += bytes_sent;
    stats_.bytes_received += bytes_received;
    stats_.last_request = std::chrono::system_clock::now();

    if (success) {
        stats_.responses_received++;
        latencies_.push_back(latency);

        // Keep only last 100 latencies
        if (latencies_.size() > 100) {
            latencies_.erase(latencies_.begin());
        }
    } else {
        stats_.errors++;
    }
}

} // namespace umicp

