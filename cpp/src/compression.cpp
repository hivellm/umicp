/**
 * UMICP Compression Manager Implementation
 * Zlib and LZ4-based compression and decompression
 */

#include "compression.h"
#include <zlib.h>
#include <cstring>
#include <iostream>

// LZ4 headers (if available)
#ifdef HAVE_LZ4
#include <lz4.h>
#include <lz4hc.h>
#endif

namespace umicp {

class CompressionManager::Impl {
public:
    explicit Impl(CompressionAlgorithm algorithm) : algorithm_(algorithm) {}

    Result<ByteBuffer> compress_zlib(const ByteBuffer& data, int level) {
        if (data.empty()) {
            return Result<ByteBuffer>(ByteBuffer{});
        }

        z_stream strm;
        memset(&strm, 0, sizeof(strm));

        if (deflateInit(&strm, level) != Z_OK) {
            return Result<ByteBuffer>(ErrorCode::COMPRESSION_FAILED, "Failed to initialize zlib");
        }

        // Estimate output size (add some overhead for compression)
        size_t estimated_size = data.size() + (data.size() / 10) + 12;
        ByteBuffer compressed_data(estimated_size);

        strm.next_in = const_cast<Bytef*>(&data[0]);
        strm.avail_in = data.size();
        strm.next_out = &compressed_data[0];
        strm.avail_out = compressed_data.size();

        int ret = deflate(&strm, Z_FINISH);
        deflateEnd(&strm);

        if (ret != Z_STREAM_END) {
            return Result<ByteBuffer>(ErrorCode::COMPRESSION_FAILED, "Compression failed");
        }

        // Resize to actual compressed size
        compressed_data.resize(strm.total_out);
        return Result<ByteBuffer>(compressed_data);
    }

    Result<ByteBuffer> decompress_zlib(const ByteBuffer& compressed_data) {
        if (compressed_data.empty()) {
            return Result<ByteBuffer>(ByteBuffer{});
        }

        z_stream strm;
        memset(&strm, 0, sizeof(strm));

        if (inflateInit(&strm) != Z_OK) {
            return Result<ByteBuffer>(ErrorCode::DECOMPRESSION_FAILED, "Failed to initialize zlib");
        }

        // Estimate output size (assume 4x expansion for worst case)
        size_t estimated_size = compressed_data.size() * 4;
        ByteBuffer decompressed_data(estimated_size);

        strm.next_in = const_cast<Bytef*>(&compressed_data[0]);
        strm.avail_in = compressed_data.size();
        strm.next_out = &decompressed_data[0];
        strm.avail_out = decompressed_data.size();

        int ret;
        do {
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_END) {
                break;
            }

            if (ret != Z_OK && ret != Z_BUF_ERROR) {
                inflateEnd(&strm);
                return Result<ByteBuffer>(ErrorCode::DECOMPRESSION_FAILED, "Decompression failed");
            }

            // Need more output space
            if (strm.avail_out == 0) {
                size_t current_size = decompressed_data.size();
                decompressed_data.resize(current_size * 2);
                strm.next_out = &decompressed_data[current_size];
                strm.avail_out = current_size;
            }
        } while (ret != Z_STREAM_END);

        inflateEnd(&strm);

        // Resize to actual decompressed size
        decompressed_data.resize(strm.total_out);
        return Result<ByteBuffer>(decompressed_data);
    }

#ifdef HAVE_LZ4
    Result<ByteBuffer> compress_lz4(const ByteBuffer& data, int level) {
        if (data.empty()) {
            return Result<ByteBuffer>(ByteBuffer{});
        }

        // LZ4 compression level (1-16, where 16 is LZ4HC)
        int compression_level = (level > 0 && level <= 16) ? level : 9; // Default to 9

        // Estimate output size (LZ4 usually needs some overhead)
        size_t max_dst_size = LZ4_compressBound(data.size());
        ByteBuffer compressed_data(max_dst_size);

        int compressed_size;
        if (compression_level >= 12) {
            // Use LZ4HC for higher compression ratios
            compressed_size = LZ4_compress_HC(
                reinterpret_cast<const char*>(&data[0]),
                reinterpret_cast<char*>(&compressed_data[0]),
                data.size(),
                max_dst_size,
                compression_level
            );
        } else {
            // Use fast LZ4 compression
            compressed_size = LZ4_compress_default(
                reinterpret_cast<const char*>(&data[0]),
                reinterpret_cast<char*>(&compressed_data[0]),
                data.size(),
                max_dst_size
            );
        }

        if (compressed_size <= 0) {
            return Result<ByteBuffer>(ErrorCode::COMPRESSION_FAILED, "LZ4 compression failed");
        }

        // Resize to actual compressed size
        compressed_data.resize(compressed_size);
        return Result<ByteBuffer>(compressed_data);
    }

    Result<ByteBuffer> decompress_lz4(const ByteBuffer& compressed_data) {
        if (compressed_data.empty()) {
            return Result<ByteBuffer>(ByteBuffer{});
        }

        // Estimate output size (LZ4 doesn't store original size, so we need to guess)
        // For safety, assume 4x expansion which is conservative for most data
        size_t estimated_size = compressed_data.size() * 4;
        ByteBuffer decompressed_data(estimated_size);

        int decompressed_size = LZ4_decompress_safe(
            reinterpret_cast<const char*>(&compressed_data[0]),
            reinterpret_cast<char*>(&decompressed_data[0]),
            compressed_data.size(),
            estimated_size
        );

        if (decompressed_size < 0) {
            return Result<ByteBuffer>(ErrorCode::DECOMPRESSION_FAILED, "LZ4 decompression failed");
        }

        // Resize to actual decompressed size
        decompressed_data.resize(decompressed_size);
        return Result<ByteBuffer>(decompressed_data);
    }
#else
    // Stub implementations when LZ4 is not available
    Result<ByteBuffer> compress_lz4(const ByteBuffer& data, int level) {
        return Result<ByteBuffer>(ErrorCode::NOT_IMPLEMENTED, "LZ4 support not available - please install liblz4-dev");
    }

    Result<ByteBuffer> decompress_lz4(const ByteBuffer& compressed_data) {
        return Result<ByteBuffer>(ErrorCode::NOT_IMPLEMENTED, "LZ4 support not available - please install liblz4-dev");
    }
#endif

    CompressionAlgorithm algorithm_;
};

CompressionManager::CompressionManager(CompressionAlgorithm algorithm)
    : impl_(std::make_unique<Impl>(algorithm)) {
}

CompressionManager::~CompressionManager() = default;

Result<ByteBuffer> CompressionManager::compress(const ByteBuffer& data, int level) {
    switch (impl_->algorithm_) {
        case CompressionAlgorithm::ZLIB:
        case CompressionAlgorithm::GZIP:
            return impl_->compress_zlib(data, level);
        case CompressionAlgorithm::LZ4:
            return impl_->compress_lz4(data, level);
        case CompressionAlgorithm::NONE:
            return Result<ByteBuffer>(data);
        default:
            return Result<ByteBuffer>(ErrorCode::NOT_IMPLEMENTED, "Compression algorithm not implemented");
    }
}

Result<ByteBuffer> CompressionManager::decompress(const ByteBuffer& compressed_data) {
    switch (impl_->algorithm_) {
        case CompressionAlgorithm::ZLIB:
        case CompressionAlgorithm::GZIP:
            return impl_->decompress_zlib(compressed_data);
        case CompressionAlgorithm::LZ4:
            return impl_->decompress_lz4(compressed_data);
        case CompressionAlgorithm::NONE:
            return Result<ByteBuffer>(compressed_data);
        default:
            return Result<ByteBuffer>(ErrorCode::NOT_IMPLEMENTED, "Decompression algorithm not implemented");
    }
}

Result<void> CompressionManager::set_algorithm(CompressionAlgorithm algorithm) {
    impl_->algorithm_ = algorithm;
    return Result<void>();
}

CompressionAlgorithm CompressionManager::get_algorithm() const {
    return impl_->algorithm_;
}

Result<size_t> CompressionManager::estimate_compressed_size(const ByteBuffer& data,
                                                           CompressionAlgorithm algorithm) {
    if (data.empty()) {
        return Result<size_t>(0);
    }

    switch (algorithm) {
        case CompressionAlgorithm::NONE:
            return Result<size_t>(data.size());
        case CompressionAlgorithm::ZLIB:
        case CompressionAlgorithm::GZIP:
            // Estimate ~50% compression ratio
            return Result<size_t>(data.size() / 2 + 128);
        case CompressionAlgorithm::LZ4:
#ifdef HAVE_LZ4
            // LZ4 compression bound
            return Result<size_t>(LZ4_compressBound(data.size()));
#else
            // Fallback estimate for LZ4 when not available
            return Result<size_t>(data.size() + 64);
#endif
        default:
            return Result<size_t>(ErrorCode::NOT_IMPLEMENTED, "Algorithm not supported for size estimation");
    }
}

bool CompressionManager::should_compress(const ByteBuffer& data, size_t threshold, CompressionAlgorithm algorithm) {
    return data.size() >= threshold && algorithm != CompressionAlgorithm::NONE;
}

} // namespace umicp
