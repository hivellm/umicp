/**
 * UMICP Buffer Management Header
 * Efficient buffer handling for binary data
 */

#ifndef UMICP_BUFFER_H
#define UMICP_BUFFER_H

#include "umicp_types.h"
#include <memory>
#include <vector>

namespace umicp {

// Buffer manager class
class BufferManager {
public:
    static Result<std::unique_ptr<ByteBuffer>> create(size_t capacity);
    static Result<std::unique_ptr<ByteBuffer>> create_from_data(const uint8_t* data, size_t size);
    static Result<void> resize(ByteBuffer& buffer, size_t new_size);
    static Result<void> append(ByteBuffer& buffer, const uint8_t* data, size_t size);
    static Result<void> prepend(ByteBuffer& buffer, const uint8_t* data, size_t size);
    static Result<std::unique_ptr<ByteBuffer>> slice(const ByteBuffer& buffer, size_t offset, size_t length);

    // Utility functions
    static void zero_fill(ByteBuffer& buffer);
    static bool is_empty(const ByteBuffer& buffer);
    static size_t get_size(const ByteBuffer& buffer);
    static size_t get_capacity(const ByteBuffer& buffer);

    // Memory management helpers
    static void secure_erase(ByteBuffer& buffer); // Overwrite with zeros for security

private:
    static constexpr size_t DEFAULT_CAPACITY = 4096;
    static constexpr size_t MAX_CAPACITY = 100 * 1024 * 1024; // 100MB
};

// Ring buffer for streaming operations (future extension)
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity);
    ~RingBuffer();

    Result<void> write(const uint8_t* data, size_t size);
    Result<size_t> read(uint8_t* buffer, size_t max_size);
    size_t available() const;
    size_t capacity() const;
    bool is_empty() const;
    bool is_full() const;
    void clear();

private:
    std::vector<uint8_t> buffer_;
    size_t read_pos_;
    size_t write_pos_;
    size_t size_;
};

} // namespace umicp

#endif // UMICP_BUFFER_H
