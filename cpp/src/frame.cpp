/**
 * UMICP Frame Implementation
 * Binary data plane message handling
 */

#include "umicp_types.h"
#include "frame.h"
#include <cstring>
#include <memory>

namespace umicp {

// Frame implementation
class FrameImpl {
public:
    Frame frame;

    FrameImpl() {
        frame.header.version = UMICP_VERSION_MAJOR;
        frame.header.type = 0;
        frame.header.flags = 0;
        frame.header.stream_id = 0;
        frame.header.sequence = 0;
        frame.header.length = 0;
    }

    Result<ByteBuffer> serialize() {
        const size_t total_size = UMICP_FRAME_HEADER_SIZE + frame.payload.size();
        ByteBuffer buffer(total_size);

        // Write header (16 bytes)
        size_t offset = 0;

        // Version (1 byte)
        buffer[offset++] = frame.header.version;

        // Type (1 byte)
        buffer[offset++] = frame.header.type;

        // Flags (2 bytes, little-endian)
        buffer[offset++] = frame.header.flags & 0xFF;
        buffer[offset++] = (frame.header.flags >> 8) & 0xFF;

        // Stream ID (8 bytes, little-endian)
        for (int i = 0; i < 8; i++) {
            buffer[offset++] = (frame.header.stream_id >> (i * 8)) & 0xFF;
        }

        // Sequence (4 bytes, little-endian)
        for (int i = 0; i < 4; i++) {
            buffer[offset++] = (frame.header.sequence >> (i * 8)) & 0xFF;
        }

        // Length (4 bytes, little-endian) - write payload size
        uint32_t payload_size = static_cast<uint32_t>(frame.payload.size());
        for (int i = 0; i < 4; i++) {
            buffer[offset++] = (payload_size >> (i * 8)) & 0xFF;
        }

        // Copy payload
        if (!frame.payload.empty()) {
            std::memcpy(&buffer[offset], frame.payload.data(), frame.payload.size());
        }

        return Result<ByteBuffer>(buffer);
    }

    Result<void> deserialize(const ByteBuffer& data) {
        if (data.size() < UMICP_FRAME_HEADER_SIZE) {
            return Result<void>(ErrorCode::INVALID_FRAME, "Frame data too small for header");
        }

        size_t offset = 0;

        // Read header
        frame.header.version = data[offset++];
        frame.header.type = data[offset++];

        // Flags (2 bytes, little-endian)
        frame.header.flags = data[offset++];
        frame.header.flags |= static_cast<uint16_t>(data[offset++]) << 8;

        // Stream ID (8 bytes, little-endian)
        frame.header.stream_id = 0;
        for (int i = 0; i < 8; i++) {
            frame.header.stream_id |= static_cast<uint64_t>(data[offset++]) << (i * 8);
        }

        // Sequence (4 bytes, little-endian)
        frame.header.sequence = 0;
        for (int i = 0; i < 4; i++) {
            frame.header.sequence |= static_cast<uint32_t>(data[offset++]) << (i * 8);
        }

        // Length (4 bytes, little-endian)
        frame.header.length = 0;
        for (int i = 0; i < 4; i++) {
            frame.header.length |= static_cast<uint32_t>(data[offset++]) << (i * 8);
        }

        // Read payload
        if (frame.header.length > 0) {
            if (data.size() < UMICP_FRAME_HEADER_SIZE + frame.header.length) {
                return Result<void>(ErrorCode::INVALID_FRAME, "Frame payload length mismatch");
            }
            frame.payload.assign(
                data.begin() + UMICP_FRAME_HEADER_SIZE,
                data.begin() + UMICP_FRAME_HEADER_SIZE + frame.header.length
            );
        } else {
            frame.payload.clear();
        }

        return Result<void>();
    }

    bool has_flag(FrameFlags flag) const {
        return (frame.header.flags & flag) != 0;
    }

    void set_flag(FrameFlags flag) {
        frame.header.flags |= flag;
    }

    void clear_flag(FrameFlags flag) {
        frame.header.flags &= ~flag;
    }

    bool is_compressed() const {
        return has_flag(FrameFlags::COMPRESSED_GZIP) ||
               has_flag(FrameFlags::COMPRESSED_BROTLI);
    }

    bool is_encrypted() const {
        return has_flag(FrameFlags::ENCRYPTED_XCHACHA20);
    }

    bool is_fragmented() const {
        return has_flag(FrameFlags::FRAGMENT_START) ||
               has_flag(FrameFlags::FRAGMENT_CONTINUE) ||
               has_flag(FrameFlags::FRAGMENT_END);
    }

    Result<void> validate() {
        if (frame.header.version != UMICP_VERSION_MAJOR) {
            return Result<void>(ErrorCode::INVALID_FRAME, "Unsupported frame version");
        }

        if (frame.payload.size() != frame.header.length) {
            return Result<void>(ErrorCode::INVALID_FRAME, "Payload length mismatch");
        }

        return Result<void>();
    }
};

// FrameBuilder implementation
FrameBuilder::FrameBuilder() : impl_(std::make_unique<FrameImpl>()) {}

FrameBuilder::~FrameBuilder() = default;

FrameBuilder& FrameBuilder::type(uint8_t type) {
    impl_->frame.header.type = type;
    return *this;
}

FrameBuilder& FrameBuilder::stream_id(uint64_t stream_id) {
    impl_->frame.header.stream_id = stream_id;
    return *this;
}

FrameBuilder& FrameBuilder::sequence(uint32_t sequence) {
    impl_->frame.header.sequence = sequence;
    return *this;
}

FrameBuilder& FrameBuilder::flags(uint16_t flags) {
    impl_->frame.header.flags = flags;
    return *this;
}

FrameBuilder& FrameBuilder::payload(const ByteBuffer& data) {
    impl_->frame.payload = data;
    impl_->frame.header.length = static_cast<uint32_t>(data.size());
    return *this;
}

FrameBuilder& FrameBuilder::compressed(FrameFlags compression_flag) {
    if (compression_flag == FrameFlags::COMPRESSED_GZIP ||
        compression_flag == FrameFlags::COMPRESSED_BROTLI) {
        impl_->set_flag(compression_flag);
    }
    return *this;
}

FrameBuilder& FrameBuilder::encrypted() {
    impl_->set_flag(FrameFlags::ENCRYPTED_XCHACHA20);
    return *this;
}

FrameBuilder& FrameBuilder::fragmented(FrameFlags fragment_flag) {
    if (fragment_flag == FrameFlags::FRAGMENT_START ||
        fragment_flag == FrameFlags::FRAGMENT_CONTINUE ||
        fragment_flag == FrameFlags::FRAGMENT_END) {
        impl_->set_flag(fragment_flag);
    }
    return *this;
}

Result<Frame> FrameBuilder::build() {
    auto validation = impl_->validate();
    if (!validation.is_success()) {
        return Result<Frame>(validation.code, validation.error_message.value_or("Validation failed"));
    }
    return Result<Frame>(impl_->frame);
}

// FrameProcessor implementation
Result<ByteBuffer> FrameProcessor::serialize(const Frame& frame) {
    FrameImpl impl;
    impl.frame = frame;
    return impl.serialize();
}

Result<Frame> FrameProcessor::deserialize(const ByteBuffer& data) {
    FrameImpl impl;
    auto result = impl.deserialize(data);
    if (!result.is_success()) {
        return Result<Frame>(result.code, result.error_message.value_or("Deserialization failed"));
    }
    return Result<Frame>(impl.frame);
}

bool FrameProcessor::has_flag(const Frame& frame, FrameFlags flag) {
    FrameImpl impl;
    impl.frame = frame;
    return impl.has_flag(flag);
}

bool FrameProcessor::is_compressed(const Frame& frame) {
    FrameImpl impl;
    impl.frame = frame;
    return impl.is_compressed();
}

bool FrameProcessor::is_encrypted(const Frame& frame) {
    FrameImpl impl;
    impl.frame = frame;
    return impl.is_encrypted();
}

bool FrameProcessor::is_fragmented(const Frame& frame) {
    FrameImpl impl;
    impl.frame = frame;
    return impl.is_fragmented();
}

} // namespace umicp
