/**
 * UMICP Configuration Header
 * Configuration management and validation
 */

#ifndef UMICP_CONFIG_H
#define UMICP_CONFIG_H

#include "umicp_types.h"
#include <string>

namespace umicp {

// Configuration manager class
class ConfigManager {
public:
    static Result<UMICPConfig> create_default();
    static Result<UMICPConfig> validate(const UMICPConfig& config);
    static Result<UMICPConfig> merge(const UMICPConfig& base, const UMICPConfig& override);

    // Configuration file operations (future extension)
    static Result<UMICPConfig> load_from_file(const std::string& filename);
    static Result<void> save_to_file(const UMICPConfig& config, const std::string& filename);

private:
    static Result<void> validate_version(const std::string& version);
    static Result<void> validate_message_size(size_t size);
    static Result<void> validate_timeout(uint32_t timeout);
};

} // namespace umicp

#endif // UMICP_CONFIG_H
