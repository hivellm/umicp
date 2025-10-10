/**
 * @file connection_pool_example.cpp
 * @brief Example demonstrating UMICP Connection Pool
 */

#include "connection_pool.h"
#include "websocket_client.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace umicp;

int main() {
    std::cout << "=== UMICP Connection Pool Example ===\n\n";

    // Create pool configuration
    std::cout << "1. Creating connection pool configuration...\n";
    PoolConfig config;
    config.address = "ws://localhost:8080";
    config.min_size = 2;
    config.max_size = 5;
    config.max_age_seconds = 300;        // 5 minutes
    config.idle_timeout_seconds = 120;   // 2 minutes
    config.acquire_timeout_ms = 5000;    // 5 seconds

    std::cout << "   Configuration:\n";
    std::cout << "   - Address: " << config.address << "\n";
    std::cout << "   - Min size: " << config.min_size << "\n";
    std::cout << "   - Max size: " << config.max_size << "\n";
    std::cout << "   - Max age: " << config.max_age_seconds << " seconds\n";
    std::cout << "   - Idle timeout: " << config.idle_timeout_seconds << " seconds\n";
    std::cout << "   - Acquire timeout: " << config.acquire_timeout_ms << " ms\n\n";

    // Create connection pool
    std::cout << "2. Creating connection pool...\n";
    ConnectionPool pool(config);
    std::cout << "   ✓ Pool created successfully\n";
    std::cout << "   Is closed: " << (pool.isClosed() ? "Yes" : "No") << "\n\n";

    // Show initial statistics
    std::cout << "3. Initial pool statistics:\n";
    auto stats = pool.getStats();
    std::cout << "   - Total connections: " << stats.total_connections << "\n";
    std::cout << "   - Available: " << stats.available_connections << "\n";
    std::cout << "   - In use: " << stats.in_use_connections << "\n";
    std::cout << "   - Total acquires: " << stats.total_acquires << "\n";
    std::cout << "   - Total releases: " << stats.total_releases << "\n";
    std::cout << "   - Total creates: " << stats.total_creates << "\n";
    std::cout << "   - Total closes: " << stats.total_closes << "\n";
    std::cout << "   - Failed acquires: " << stats.failed_acquires << "\n\n";

    // Note: The following operations would require a real WebSocket server
    std::cout << "4. Pool initialization:\n";
    std::cout << "   NOTE: Actual initialization requires a running WebSocket server\n";
    std::cout << "   This example demonstrates the API structure.\n\n";

    // Demonstrate pooled connection lifecycle
    std::cout << "5. Demonstrating pooled connection lifecycle:\n";
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    auto conn = std::make_shared<PooledConnection>("ws://localhost:8080", client);

    std::cout << "   Connection created:\n";
    std::cout << "   - ID: " << conn->getId() << "\n";
    std::cout << "   - Address: " << conn->getAddress() << "\n";
    std::cout << "   - State: Available\n";
    std::cout << "   - Use count: " << conn->getUseCount() << "\n\n";

    std::cout << "   Acquiring connection...\n";
    conn->acquire();
    std::cout << "   - State: InUse\n";
    std::cout << "   - Use count: " << conn->getUseCount() << "\n\n";

    std::cout << "   Simulating work...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "   Releasing connection...\n";
    conn->release();
    std::cout << "   - State: Available\n";
    std::cout << "   - Use count: " << conn->getUseCount() << "\n\n";

    std::cout << "   Acquiring again...\n";
    conn->acquire();
    std::cout << "   - Use count: " << conn->getUseCount() << "\n\n";

    std::cout << "   Closing connection...\n";
    conn->close();
    std::cout << "   - State: Closed\n\n";

    // Demonstrate stale detection
    std::cout << "6. Testing stale connection detection:\n";
    WebSocketClientConfig test_config;
    test_config.url = "ws://localhost:8080";
    auto test_conn = std::make_shared<PooledConnection>("ws://localhost:8080",
                                                        std::make_shared<WebSocketClient>(test_config));

    std::cout << "   Fresh connection:\n";
    std::cout << "   - Is stale (10s timeout): " << (test_conn->isStale(10) ? "Yes" : "No") << "\n";
    std::cout << "   - Is idle (10s timeout): " << (test_conn->isIdle(10) ? "Yes" : "No") << "\n\n";

    std::cout << "   Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "   After 2 seconds:\n";
    std::cout << "   - Is stale (1s timeout): " << (test_conn->isStale(1) ? "Yes" : "No") << "\n";
    std::cout << "   - Is idle (1s timeout): " << (test_conn->isIdle(1) ? "Yes" : "No") << "\n\n";

    // Demonstrate pool cleanup
    std::cout << "7. Testing pool cleanup:\n";
    int removed = pool.cleanup();
    std::cout << "   ✓ Cleaned up " << removed << " connection(s)\n\n";

    // Pool configuration validation
    std::cout << "8. Testing configuration validation:\n";
    PoolConfig bad_config;
    bad_config.address = "ws://test:9000";
    bad_config.min_size = 20;  // Invalid: greater than max
    bad_config.max_size = 10;

    ConnectionPool test_pool(bad_config);
    std::cout << "   Original: min=" << bad_config.min_size << ", max=" << bad_config.max_size << "\n";
    std::cout << "   Adjusted: min=" << test_pool.getConfig().min_size
              << ", max=" << test_pool.getConfig().max_size << "\n";
    std::cout << "   ✓ Configuration automatically validated\n\n";

    // Connection states
    std::cout << "9. Connection state transitions:\n";
    WebSocketClientConfig state_config;
    state_config.url = "ws://localhost:8080";
    auto state_conn = std::make_shared<PooledConnection>("ws://localhost:8080",
                                                         std::make_shared<WebSocketClient>(state_config));

    auto print_state = [](PoolConnectionState state) {
        switch (state) {
            case PoolConnectionState::Available: return "Available";
            case PoolConnectionState::InUse: return "InUse";
            case PoolConnectionState::Validating: return "Validating";
            case PoolConnectionState::Closed: return "Closed";
            default: return "Unknown";
        }
    };

    std::cout << "   Initial state: " << print_state(state_conn->getState()) << "\n";

    state_conn->acquire();
    std::cout << "   After acquire: " << print_state(state_conn->getState()) << "\n";

    state_conn->release();
    std::cout << "   After release: " << print_state(state_conn->getState()) << "\n";

    state_conn->close();
    std::cout << "   After close: " << print_state(state_conn->getState()) << "\n\n";

    // Pool shutdown
    std::cout << "10. Shutting down pool...\n";
    pool.shutdown();
    std::cout << "    ✓ Pool shut down\n";
    std::cout << "    Is closed: " << (pool.isClosed() ? "Yes" : "No") << "\n\n";

    // Final statistics
    stats = pool.getStats();
    std::cout << "11. Final pool statistics:\n";
    std::cout << "    - Total connections: " << stats.total_connections << "\n";
    std::cout << "    - Available: " << stats.available_connections << "\n";
    std::cout << "    - In use: " << stats.in_use_connections << "\n";
    std::cout << "    - Total acquires: " << stats.total_acquires << "\n";
    std::cout << "    - Total releases: " << stats.total_releases << "\n";
    std::cout << "    - Total creates: " << stats.total_creates << "\n";
    std::cout << "    - Total closes: " << stats.total_closes << "\n";
    std::cout << "    - Failed acquires: " << stats.failed_acquires << "\n\n";

    std::cout << "=== Connection Pool Example Complete ===\n";
    std::cout << "\nNOTE: This example demonstrates the API structure.\n";
    std::cout << "For full functionality, connect to a running WebSocket server.\n";

    return 0;
}

