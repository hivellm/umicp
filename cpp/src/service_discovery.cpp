/**
 * @file service_discovery.cpp
 * @brief Implementation of Service Discovery for UMICP
 */

#include "service_discovery.h"
#include <algorithm>

namespace umicp {

// ============================================================================
// ServiceInfo Implementation
// ============================================================================

ServiceInfo::ServiceInfo(const std::string& service_id,
                         const std::string& name,
                         const std::string& address)
    : service_id_(service_id)
    , name_(name)
    , address_(address)
    , last_seen_(std::chrono::system_clock::now())
    , version_("1.0.0")
{
}

void ServiceInfo::addCapability(const std::string& capability) {
    // Only add if not already present
    if (std::find(capabilities_.begin(), capabilities_.end(), capability) == capabilities_.end()) {
        capabilities_.push_back(capability);
    }
}

void ServiceInfo::addMetadata(const std::string& key, const std::string& value) {
    metadata_[key] = value;
}

bool ServiceInfo::hasCapability(const std::string& capability) const {
    return std::find(capabilities_.begin(), capabilities_.end(), capability) != capabilities_.end();
}

std::string ServiceInfo::getMetadataValue(const std::string& key) const {
    auto it = metadata_.find(key);
    if (it != metadata_.end()) {
        return it->second;
    }
    return "";
}

void ServiceInfo::updateLastSeen() {
    last_seen_ = std::chrono::system_clock::now();
}

bool ServiceInfo::isStale(int timeout_seconds) const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_seen_);
    return elapsed.count() > timeout_seconds;
}

// ============================================================================
// ServiceDiscovery Implementation
// ============================================================================

ServiceDiscovery::ServiceDiscovery()
    : timeout_seconds_(60)
{
}

ServiceDiscovery::ServiceDiscovery(int timeout_seconds)
    : timeout_seconds_(timeout_seconds)
{
}

void ServiceDiscovery::registerLocal(const ServiceInfo& service) {
    std::lock_guard<std::mutex> lock(mutex_);
    local_service_ = std::make_shared<ServiceInfo>(service);
    services_[service.getServiceId()] = local_service_;
}

void ServiceDiscovery::registerService(const ServiceInfo& service) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = services_.find(service.getServiceId());

    if (it != services_.end()) {
        // Update existing service
        it->second = std::make_shared<ServiceInfo>(service);
        it->second->updateLastSeen();
    } else {
        // Add new service
        services_[service.getServiceId()] = std::make_shared<ServiceInfo>(service);
    }
}

bool ServiceDiscovery::unregisterService(const std::string& service_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = services_.find(service_id);

    if (it != services_.end()) {
        services_.erase(it);
        return true;
    }

    return false;
}

std::shared_ptr<ServiceInfo> ServiceDiscovery::getService(const std::string& service_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = services_.find(service_id);

    if (it != services_.end()) {
        return it->second;
    }

    return nullptr;
}

std::vector<std::shared_ptr<ServiceInfo>> ServiceDiscovery::getAllServices() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<ServiceInfo>> result;
    result.reserve(services_.size());

    for (const auto& pair : services_) {
        result.push_back(pair.second);
    }

    return result;
}

std::vector<std::shared_ptr<ServiceInfo>> ServiceDiscovery::findByCapability(
    const std::string& capability) const {

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<ServiceInfo>> result;

    for (const auto& pair : services_) {
        if (pair.second->hasCapability(capability)) {
            result.push_back(pair.second);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ServiceInfo>> ServiceDiscovery::findByMetadata(
    const std::string& key, const std::string& value) const {

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<ServiceInfo>> result;

    for (const auto& pair : services_) {
        if (pair.second->getMetadataValue(key) == value) {
            result.push_back(pair.second);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ServiceInfo>> ServiceDiscovery::findByName(
    const std::string& name_pattern) const {

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<ServiceInfo>> result;

    for (const auto& pair : services_) {
        if (pair.second->getName().find(name_pattern) != std::string::npos) {
            result.push_back(pair.second);
        }
    }

    return result;
}

int ServiceDiscovery::cleanupStaleServices() {
    std::lock_guard<std::mutex> lock(mutex_);
    int removed_count = 0;

    auto it = services_.begin();
    while (it != services_.end()) {
        if (it->second->isStale(timeout_seconds_)) {
            // Don't remove local service
            if (local_service_ && it->second->getServiceId() == local_service_->getServiceId()) {
                ++it;
                continue;
            }

            it = services_.erase(it);
            removed_count++;
        } else {
            ++it;
        }
    }

    return removed_count;
}

size_t ServiceDiscovery::getServiceCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return services_.size();
}

std::shared_ptr<ServiceInfo> ServiceDiscovery::getLocalService() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return local_service_;
}

void ServiceDiscovery::setTimeout(int timeout_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    timeout_seconds_ = timeout_seconds;
}

bool ServiceDiscovery::announceService(const ServiceInfo& service) {
    // Placeholder for network announcement
    // In a full implementation, this would broadcast the service
    // information to the network (e.g., via multicast or central registry)

    // For now, just register it locally
    registerService(service);
    return true;
}

} // namespace umicp

