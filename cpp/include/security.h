/**
 * UMICP Security Manager Header
 * Authentication and encryption support
 */

#ifndef UMICP_SECURITY_H
#define UMICP_SECURITY_H

#include "umicp_types.h"
#include <string>
#include <memory>

namespace umicp {

// Security manager class
class SecurityManager {
public:
    explicit SecurityManager(const std::string& local_id);
    ~SecurityManager();

    // Authentication
    bool authenticated = false;
    std::string local_id;
    std::optional<std::string> peer_id;

    // Key management
    Result<void> generate_keypair();
    Result<void> load_private_key(const ByteBuffer& key_data);
    Result<void> set_peer_public_key(const ByteBuffer& public_key);

    // Signing and verification
    Result<ByteBuffer> sign_data(const ByteBuffer& data);
    Result<bool> verify_signature(const ByteBuffer& data, const ByteBuffer& signature);

    // Encryption/Decryption
    Result<ByteBuffer> encrypt_data(const ByteBuffer& plaintext);
    Result<ByteBuffer> decrypt_data(const ByteBuffer& ciphertext);

    // Session management
    Result<void> establish_session(const std::string& peer_id);
    void close_session();
    bool has_session() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace umicp

#endif // UMICP_SECURITY_H
