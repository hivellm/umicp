/**
 * UMICP StreamableHTTP Server Implementation
 */

#include "http_server.h"
#include "serialization.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <sstream>
#include <algorithm>

namespace umicp {

// ============================================================================
// HTTPResponseBuilder Implementation
// ============================================================================

HTTPResponseBuilder& HTTPResponseBuilder::status(int code) {
    status_code_ = code;
    return *this;
}

HTTPResponseBuilder& HTTPResponseBuilder::header(const std::string& key, const std::string& value) {
    headers_[key] = value;
    return *this;
}

HTTPResponseBuilder& HTTPResponseBuilder::body(const std::vector<uint8_t>& data) {
    body_ = data;
    return *this;
}

HTTPResponseBuilder& HTTPResponseBuilder::body(const std::string& data) {
    body_.assign(data.begin(), data.end());
    return *this;
}

HTTPResponseBuilder& HTTPResponseBuilder::json(const std::string& json_str) {
    headers_["Content-Type"] = "application/json";
    return body(json_str);
}

std::vector<uint8_t> HTTPResponseBuilder::build() const {
    std::ostringstream response;

    // Status line
    response << "HTTP/1.1 " << status_code_ << " " << get_status_message(status_code_) << "\r\n";

    // Headers
    auto headers = headers_;

    // Add Content-Length if not present
    if (headers.find("Content-Length") == headers.end()) {
        headers["Content-Length"] = std::to_string(body_.size());
    }

    // Add Server header
    if (headers.find("Server") == headers.end()) {
        headers["Server"] = "UMICP/1.0";
    }

    // Add Date header
    if (headers.find("Date") == headers.end()) {
        time_t now = time(nullptr);
        char date_buf[128];
        strftime(date_buf, sizeof(date_buf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
        headers["Date"] = date_buf;
    }

    for (const auto& [key, value] : headers) {
        response << key << ": " << value << "\r\n";
    }

    response << "\r\n";

    // Build response vector
    std::string header_str = response.str();
    std::vector<uint8_t> result(header_str.begin(), header_str.end());
    result.insert(result.end(), body_.begin(), body_.end());

    return result;
}

std::string HTTPResponseBuilder::get_status_message(int code) const {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

// ============================================================================
// StreamableHTTPServer Implementation
// ============================================================================

StreamableHTTPServer::StreamableHTTPServer(const HTTPServerConfig& config)
    : config_(config),
      server_socket_(-1),
      running_(false),
      should_stop_(false),
      stats_{} {
}

StreamableHTTPServer::~StreamableHTTPServer() {
    stop();
}

Result<void> StreamableHTTPServer::start() {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    if (running_) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Server already running");
    }

    // Create server socket
    auto socket_result = create_server_socket();
    if (!socket_result.is_ok()) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Failed to create socket: " + socket_result.error());
    }

    server_socket_ = *socket_result.value;
    running_ = true;
    should_stop_ = false;

    // Initialize statistics
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.started_at = std::chrono::system_clock::now();
        stats_.connections_active = 0;
    }

    // Start accept thread
    accept_thread_ = std::thread([this]() { accept_loop(); });

    // Start worker threads
    for (size_t i = 0; i < config_.worker_threads; ++i) {
        worker_threads_.emplace_back([this]() { worker_loop(); });
    }

    emit("start");

    return Result<void>::ok();
}

void StreamableHTTPServer::stop() {
    if (!running_) {
        return;
    }

    should_stop_ = true;
    running_ = false;

    // Close server socket to break accept loop
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }

    // Wait for accept thread
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    // Wait for worker threads
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();

    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [id, client] : clients_) {
            if (client.socket_fd >= 0) {
                close(client.socket_fd);
            }
        }
        clients_.clear();
    }

    emit("stop");
}

bool StreamableHTTPServer::is_running() const {
    return running_;
}

void StreamableHTTPServer::on_message(MessageHandler handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    message_handler_ = handler;
}

void StreamableHTTPServer::on_request(const std::string& path, HTTPRequestHandler handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    request_handlers_[path] = handler;
}

std::vector<HTTPClientInfo> StreamableHTTPServer::get_clients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    std::vector<HTTPClientInfo> result;
    result.reserve(clients_.size());

    for (const auto& [id, client] : clients_) {
        result.push_back(client);
    }

    return result;
}

Result<HTTPClientInfo> StreamableHTTPServer::get_client(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        return Result<HTTPClientInfo>::error(ErrorCode::PROTOCOL_ERROR, "Client not found");
    }

    return Result<HTTPClientInfo>::ok(it->second);
}

Result<void> StreamableHTTPServer::disconnect_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Client not found");
    }

    close_client(client_id);
    return Result<void>::ok();
}

StreamableHTTPServer::Statistics StreamableHTTPServer::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    Statistics stats = stats_;
    auto now = std::chrono::system_clock::now();
    stats.uptime = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - stats_.started_at);

    return stats;
}

void StreamableHTTPServer::reset_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    auto started_at = stats_.started_at;
    size_t active = stats_.connections_active;

    stats_ = Statistics{};
    stats_.started_at = started_at;
    stats_.connections_active = active;
}

void StreamableHTTPServer::accept_loop() {
    while (!should_stop_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_socket_,
                              (struct sockaddr*)&client_addr,
                              &client_len);

        if (client_fd < 0) {
            if (should_stop_) {
                break;
            }
            continue;
        }

        // Check connection limit
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            if (clients_.size() >= config_.max_connections) {
                close(client_fd);
                continue;
            }
        }

        // Get client info
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        uint16_t client_port = ntohs(client_addr.sin_port);

        std::string client_id = generate_client_id(client_fd, client_ip, client_port);

        // Add client
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            HTTPClientInfo info(client_id, client_fd);
            info.remote_address = client_ip;
            info.remote_port = client_port;
            clients_[client_id] = info;

            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.connections_accepted++;
            stats_.connections_active++;
        }

        emit("connection");

        // Handle client in separate thread (worker pool handles this)
        std::thread([this, client_fd]() {
            handle_client(client_fd);
        }).detach();
    }
}

void StreamableHTTPServer::worker_loop() {
    // Worker threads are created but actual work is done in handle_client
    // This is a placeholder for future thread pool implementation
}

void StreamableHTTPServer::handle_client(int client_fd) {
    while (!should_stop_) {
        // Parse request
        auto request_result = parse_request(client_fd);
        if (!request_result.is_ok()) {
            // Connection closed or error
            break;
        }

        if (!request_result.value.has_value()) continue;
        HTTPRequest request = *request_result.value;

        // Update client activity
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (auto& [id, client] : clients_) {
                if (client.socket_fd == client_fd) {
                    client.last_activity = std::chrono::system_clock::now();
                    client.requests_handled++;
                    client.bytes_received += request.body.size();
                    break;
                }
            }
        }

        emit("request");

        // Handle request
        HTTPResponseBuilder response = handle_request(request);

        // Send response
        auto send_result = send_response(client_fd, response);
        if (!send_result.is_ok()) {
            break;
        }

        // Check if connection should be kept alive
        auto conn_header = request.headers.find("Connection");
        bool keep_alive = (conn_header != request.headers.end() &&
                          conn_header->second == "keep-alive");

        if (!keep_alive) {
            break;
        }
    }

    // Close connection
    std::string client_id_to_close;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [id, client] : clients_) {
            if (client.socket_fd == client_fd) {
                client_id_to_close = id;
                break;
            }
        }
    }

    if (!client_id_to_close.empty()) {
        close_client(client_id_to_close);
    }
}

Result<HTTPRequest> StreamableHTTPServer::parse_request(int client_fd) {
    HTTPRequest request;

    // Read request headers
    std::vector<char> buffer(config_.buffer_size);
    std::string request_str;

    ssize_t bytes_read;
    bool headers_complete = false;

    while (!headers_complete && !should_stop_) {
        bytes_read = recv(client_fd, buffer.data(), buffer.size(), 0);

        if (bytes_read <= 0) {
            return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, "Connection closed");
        }

        request_str.append(buffer.data(), bytes_read);

        // Check if headers are complete
        size_t header_end = request_str.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            headers_complete = true;
        }

        if (request_str.size() > config_.max_header_size) {
            return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, "Headers too large");
        }
    }

    // Split into lines
    std::vector<std::string> lines;
    std::istringstream stream(request_str);
    std::string line;

    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    if (lines.empty()) {
        return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, "Invalid request");
    }

    // Parse request line
    auto result = parse_request_line(lines[0], request);
    if (!result.is_ok()) {
        return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, result.error());
    }

    // Parse headers
    result = parse_headers(std::vector<std::string>(lines.begin() + 1, lines.end()), request);
    if (!result.is_ok()) {
        return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, result.error());
    }

    // Read body if present
    auto content_length_it = request.headers.find("Content-Length");
    if (content_length_it != request.headers.end()) {
        size_t content_length = std::stoull(content_length_it->second);

        if (content_length > config_.max_request_size) {
            return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, "Request too large");
        }

        result = read_body(client_fd, request, content_length);
        if (!result.is_ok()) {
            return Result<HTTPRequest>::error(ErrorCode::PROTOCOL_ERROR, result.error());
        }
    }

    request.received_at = std::chrono::system_clock::now();

    return Result<HTTPRequest>::ok(request);
}

Result<void> StreamableHTTPServer::parse_request_line(const std::string& line, HTTPRequest& req) {
    std::istringstream stream(line);
    std::string method_str, path, version;

    stream >> method_str >> path >> version;

    if (method_str.empty() || path.empty()) {
        return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Invalid request line");
    }

    // Parse method
    if (method_str == "GET") req.method = HTTPMethod::GET;
    else if (method_str == "POST") req.method = HTTPMethod::POST;
    else if (method_str == "PUT") req.method = HTTPMethod::PUT;
    else if (method_str == "DELETE") req.method = HTTPMethod::DELETE;
    else if (method_str == "HEAD") req.method = HTTPMethod::HEAD;
    else if (method_str == "OPTIONS") req.method = HTTPMethod::OPTIONS;
    else if (method_str == "PATCH") req.method = HTTPMethod::PATCH;
    else req.method = HTTPMethod::UNKNOWN;

    // Parse path and query string
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        req.path = path.substr(0, query_pos);
        req.query_string = path.substr(query_pos + 1);

        // Parse query parameters
        std::istringstream query_stream(req.query_string);
        std::string param;
        while (std::getline(query_stream, param, '&')) {
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = param.substr(0, eq_pos);
                std::string value = param.substr(eq_pos + 1);
                req.query_params[key] = value;
            }
        }
    } else {
        req.path = path;
    }

    return Result<void>::ok();
}

Result<void> StreamableHTTPServer::parse_headers(const std::vector<std::string>& lines, HTTPRequest& req) {
    for (const auto& line : lines) {
        if (line.empty()) break;

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        req.headers[key] = value;
    }

    return Result<void>::ok();
}

Result<void> StreamableHTTPServer::read_body(int client_fd, HTTPRequest& req, size_t content_length) {
    req.body.resize(content_length);
    size_t total_read = 0;

    while (total_read < content_length && !should_stop_) {
        ssize_t bytes_read = recv(client_fd,
                                  req.body.data() + total_read,
                                  content_length - total_read,
                                  0);

        if (bytes_read <= 0) {
            return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Connection closed while reading body");
        }

        total_read += bytes_read;
    }

    return Result<void>::ok();
}

HTTPResponseBuilder StreamableHTTPServer::handle_request(const HTTPRequest& req) {
    // Check for custom handler
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        auto it = request_handlers_.find(req.path);
        if (it != request_handlers_.end()) {
            return it->second(req);
        }
    }

    // Handle health check
    if (req.path == "/health") {
        return handle_health_check(req);
    }

    // Handle UMICP message
    if (req.path == config_.path) {
        return handle_umicp_message(req);
    }

    // Not found
    HTTPResponseBuilder builder;
    builder.status(404)
           .json("{\"error\": \"Not found\"}");
    return builder;
}

HTTPResponseBuilder StreamableHTTPServer::handle_health_check(const HTTPRequest& req) {
    HTTPResponseBuilder builder;
    builder.status(200)
           .json("{\"status\": \"ok\", \"service\": \"UMICP\"}");
    return builder;
}

HTTPResponseBuilder StreamableHTTPServer::handle_umicp_message(const HTTPRequest& req) {
    HTTPResponseBuilder builder;

    // Check if message handler is set
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        if (!message_handler_) {
            builder.status(501)
                   .json("{\"error\": \"No message handler configured\"}");
            return builder;
        }
    }

    // Deserialize envelope from JSON
    std::string json_str(req.body.begin(), req.body.end());
    auto envelope_result = EnvelopeProcessor::deserialize(json_str);
    if (!envelope_result.is_ok()) {
        builder.status(400)
               .json("{\"error\": \"Invalid envelope\"}");
        return builder;
    }

    auto envelope = *envelope_result.value;

    // Call message handler
    Result<Envelope> response_result;
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        response_result = message_handler_(envelope, req);
    }

    if (!response_result.is_ok()) {
        builder.status(500)
               .json("{\"error\": \"" + response_result.error() + "\"}");
        return builder;
    }

    auto response_envelope = *response_result.value;

    // Serialize response to JSON
    auto json_result = EnvelopeProcessor::serialize(response_envelope);
    if (!json_result.is_ok()) {
        builder.status(500)
               .json("{\"error\": \"Failed to serialize response\"}");
        return builder;
    }

    std::string response_json = *json_result.value;
    std::vector<uint8_t> serialized(response_json.begin(), response_json.end());

    builder.status(200)
           .header("Content-Type", "application/json")
           .body(serialized);

    return builder;
}

Result<void> StreamableHTTPServer::send_response(int client_fd, const HTTPResponseBuilder& builder) {
    auto response_data = builder.build();

    size_t total_sent = 0;
    while (total_sent < response_data.size() && !should_stop_) {
        ssize_t bytes_sent = send(client_fd,
                                  response_data.data() + total_sent,
                                  response_data.size() - total_sent,
                                  0);

        if (bytes_sent < 0) {
            return Result<void>::error(ErrorCode::PROTOCOL_ERROR, "Failed to send response");
        }

        total_sent += bytes_sent;
    }

    // Update statistics
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto& [id, client] : clients_) {
            if (client.socket_fd == client_fd) {
                client.bytes_sent += total_sent;
                break;
            }
        }
    }

    update_stats(0, total_sent, false);

    return Result<void>::ok();
}

Result<int> StreamableHTTPServer::create_server_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return Result<int>::error(ErrorCode::INITIALIZATION_FAILED, "Failed to create socket");
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(sockfd);
        return Result<int>::error(ErrorCode::INITIALIZATION_FAILED, "Failed to set SO_REUSEADDR");
    }

    // Bind to address
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config_.port);

    if (config_.host == "0.0.0.0") {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, config_.host.c_str(), &server_addr.sin_addr) <= 0) {
            close(sockfd);
            return Result<int>::error(ErrorCode::INITIALIZATION_FAILED, "Invalid host address");
        }
    }

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return Result<int>::error(ErrorCode::INITIALIZATION_FAILED, "Failed to bind to port " + std::to_string(config_.port));
    }

    // Listen
    if (listen(sockfd, config_.max_connections) < 0) {
        close(sockfd);
        return Result<int>::error(ErrorCode::INITIALIZATION_FAILED, "Failed to listen on socket");
    }

    return Result<int>::ok(sockfd);
}

void StreamableHTTPServer::close_client(const std::string& client_id) {
    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        return;
    }

    if (it->second.socket_fd >= 0) {
        close(it->second.socket_fd);
    }

    clients_.erase(it);

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        if (stats_.connections_active > 0) {
            stats_.connections_active--;
        }
    }

    emit("disconnection");
}

void StreamableHTTPServer::update_stats(size_t bytes_received, size_t bytes_sent, bool error) {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    stats_.requests_handled++;
    stats_.bytes_received += bytes_received;
    stats_.bytes_sent += bytes_sent;

    if (error) {
        stats_.errors++;
    }
}

std::string StreamableHTTPServer::generate_client_id(int fd, const std::string& remote_addr,
                                                     uint16_t remote_port) {
    return remote_addr + ":" + std::to_string(remote_port) + "#" + std::to_string(fd);
}

} // namespace umicp

