/**
 * UMICP Buffer Implementation
 * Efficient buffer handling for binary data
 */

#include "umicp_types.h"
#include "buffer.h"
#include <cstring>
#include <algorithm>
#include <memory>

namespace umicp {

Result<std::unique_ptr<ByteBuffer>> BufferManager::create(size_t capacity) {
    if (capacity == 0) {
        capacity = DEFAULT_CAPACITY;
    }

    if (capacity > MAX_CAPACITY) {
        return Result<std::unique_ptr<ByteBuffer>>(ErrorCode::INVALID_ARGUMENT,
            "Requested capacity exceeds maximum allowed size");
    }

    try {
        auto buffer = std::make_unique<ByteBuffer>();
        buffer->reserve(capacity);
        return Result<std::unique_ptr<ByteBuffer>>(std::move(buffer));
    } catch (const std::bad_alloc&) {
        return Result<std::unique_ptr<ByteBuffer>>(ErrorCode::MEMORY_ALLOCATION,
            "Failed to allocate buffer memory");
    }
}

Result<std::unique_ptr<ByteBuffer>> BufferManager::create_from_data(const uint8_t* data, size_t size) {
    if (!data && size > 0) {
        return Result<std::unique_ptr<ByteBuffer>>(ErrorCode::INVALID_ARGUMENT,
            "Null data pointer provided with non-zero size");
    }

    auto result = create(size);
    if (!result.is_success()) {
        return result;
    }

    std::memcpy(result.value->get()->data(), data, size);
    result.value->get()->resize(size);

    return result;
}

Result<void> BufferManager::resize(ByteBuffer& buffer, size_t new_size) {
    if (new_size > MAX_CAPACITY) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT,
            "Requested size exceeds maximum allowed capacity");
    }

    try {
        buffer.resize(new_size);
        return Result<void>();
    } catch (const std::bad_alloc&) {
        return Result<void>(ErrorCode::MEMORY_ALLOCATION,
            "Failed to resize buffer");
    }
}

Result<void> BufferManager::append(ByteBuffer& buffer, const uint8_t* data, size_t size) {
    if (!data && size > 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT,
            "Null data pointer provided with non-zero size");
    }

    size_t new_size = buffer.size() + size;
    if (new_size > MAX_CAPACITY) {
        return Result<void>(ErrorCode::BUFFER_OVERFLOW,
            "Appending would exceed maximum buffer capacity");
    }

    try {
        buffer.insert(buffer.end(), data, data + size);
        return Result<void>();
    } catch (const std::bad_alloc&) {
        return Result<void>(ErrorCode::MEMORY_ALLOCATION,
            "Failed to append data to buffer");
    }
}

Result<void> BufferManager::prepend(ByteBuffer& buffer, const uint8_t* data, size_t size) {
    if (!data && size > 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT,
            "Null data pointer provided with non-zero size");
    }

    size_t new_size = buffer.size() + size;
    if (new_size > MAX_CAPACITY) {
        return Result<void>(ErrorCode::BUFFER_OVERFLOW,
            "Prepending would exceed maximum buffer capacity");
    }

    try {
        buffer.insert(buffer.begin(), data, data + size);
        return Result<void>();
    } catch (const std::bad_alloc&) {
        return Result<void>(ErrorCode::MEMORY_ALLOCATION,
            "Failed to prepend data to buffer");
    }
}

Result<std::unique_ptr<ByteBuffer>> BufferManager::slice(const ByteBuffer& buffer, size_t offset, size_t length) {
    if (offset >= buffer.size()) {
        return Result<std::unique_ptr<ByteBuffer>>(ErrorCode::INVALID_ARGUMENT,
            "Offset exceeds buffer size");
    }

    size_t available = buffer.size() - offset;
    if (length > available) {
        length = available;
    }

    if (length == 0) {
        return Result<std::unique_ptr<ByteBuffer>>(ErrorCode::INVALID_ARGUMENT,
            "Requested slice length is zero");
    }

    auto result = create(length);
    if (!result.is_success()) {
        return result;
    }

    std::memcpy(result.value->get()->data(), buffer.data() + offset, length);
    result.value->get()->resize(length);

    return result;
}

void BufferManager::zero_fill(ByteBuffer& buffer) {
    std::fill(buffer.begin(), buffer.end(), 0);
}

bool BufferManager::is_empty(const ByteBuffer& buffer) {
    return buffer.empty();
}

size_t BufferManager::get_size(const ByteBuffer& buffer) {
    return buffer.size();
}

size_t BufferManager::get_capacity(const ByteBuffer& buffer) {
    return buffer.capacity();
}

void BufferManager::secure_erase(ByteBuffer& buffer) {
    // Use volatile to prevent compiler optimization
    volatile uint8_t* ptr = buffer.data();
    for (size_t i = 0; i < buffer.size(); ++i) {
        ptr[i] = 0;
    }
}

// RingBuffer implementation
RingBuffer::RingBuffer(size_t capacity)
    : buffer_(capacity)
    , read_pos_(0)
    , write_pos_(0)
    , size_(0) {
}

RingBuffer::~RingBuffer() = default;

Result<void> RingBuffer::write(const uint8_t* data, size_t size) {
    if (!data && size > 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null data pointer");
    }

    if (size > buffer_.size() - size_) {
        return Result<void>(ErrorCode::BUFFER_OVERFLOW, "Not enough space in ring buffer");
    }

    for (size_t i = 0; i < size; ++i) {
        buffer_[write_pos_] = data[i];
        write_pos_ = (write_pos_ + 1) % buffer_.size();
    }

    size_ += size;
    return Result<void>();
}

Result<size_t> RingBuffer::read(uint8_t* buffer, size_t max_size) {
    if (!buffer) {
        return Result<size_t>(ErrorCode::INVALID_ARGUMENT, "Null buffer pointer");
    }

    size_t read_size = std::min(max_size, size_);

    for (size_t i = 0; i < read_size; ++i) {
        buffer[i] = buffer_[read_pos_];
        read_pos_ = (read_pos_ + 1) % buffer_.size();
    }

    size_ -= read_size;
    return Result<size_t>(read_size);
}

size_t RingBuffer::available() const {
    return size_;
}

size_t RingBuffer::capacity() const {
    return buffer_.size();
}

bool RingBuffer::is_empty() const {
    return size_ == 0;
}

bool RingBuffer::is_full() const {
    return size_ == buffer_.size();
}

void RingBuffer::clear() {
    read_pos_ = 0;
    write_pos_ = 0;
    size_ = 0;
    BufferManager::zero_fill(buffer_);
}

} // namespace umicp
