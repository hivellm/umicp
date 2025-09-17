/**
 * UMICP Transport Layer Implementation
 * WebSocket and HTTP/2 transport implementations
 */

#include "transport.h"
#include "envelope.h"
#include "frame.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
// Transport implementations are now in bindings, not in core

namespace umicp {

// Transport implementations are now in bindings, not in core

// ===============================================
// Transport Factory
// ===============================================

::std::unique_ptr<Transport> TransportFactory::create(TransportType type, const TransportConfig& config) {
    // Transport implementations are now in bindings, not in core
    return nullptr;
}

::std::unique_ptr<Transport> TransportFactory::create(TransportType type, const TransportConfig& config, const UMICPConfig& umicp_config) {
    TransportConfig enhanced_config = apply_umicp_config(config, umicp_config);
    return create(type, enhanced_config);
}

TransportConfig TransportFactory::apply_umicp_config(const TransportConfig& transport_config, const UMICPConfig& umicp_config) {
    TransportConfig enhanced_config = transport_config;

    // Apply SSL/TLS configuration based on UMICP config
    if (umicp_config.validate_certificates) {
        if (!enhanced_config.ssl_config) {
            enhanced_config.ssl_config = SSLConfig();
        }

        // Enable SSL if not explicitly configured
        if (!enhanced_config.ssl_config->enable_ssl) {
            enhanced_config.ssl_config->enable_ssl = true;
        }

        // Apply certificate validation settings
        enhanced_config.ssl_config->verify_peer = true;
        enhanced_config.ssl_config->verify_host = true;
    }

    return enhanced_config;
}

} // namespace umicp
