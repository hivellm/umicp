/**
 * UMICP Compression Manager Header
 * Message compression and decompression support
 */

#ifndef UMICP_COMPRESSION_H
#define UMICP_COMPRESSION_H

#include "umicp_types.h"
#include <memory>

namespace umicp {

// Compression manager class
class CompressionManager {
public:
    explicit CompressionManager(CompressionAlgorithm algorithm = CompressionAlgorithm::ZLIB);
    ~CompressionManager();

    // Compression operations
    Result<ByteBuffer> compress(const ByteBuffer& data, int level = -1);
    Result<ByteBuffer> decompress(const ByteBuffer& compressed_data);

    // Configuration
    Result<void> set_algorithm(CompressionAlgorithm algorithm);
    CompressionAlgorithm get_algorithm() const;

    // Utility functions
    static Result<size_t> estimate_compressed_size(const ByteBuffer& data,
                                                   CompressionAlgorithm algorithm);
    static bool should_compress(const ByteBuffer& data, size_t threshold, CompressionAlgorithm algorithm);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace umicp

#endif // UMICP_COMPRESSION_H
