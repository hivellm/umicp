/**
 * UMICP Frame Header
 * Binary data plane message handling
 */

#ifndef UMICP_FRAME_H
#define UMICP_FRAME_H

#include "umicp_types.h"
#include <memory>

namespace umicp {

// Forward declaration
class FrameImpl;

// Frame builder pattern
class FrameBuilder {
public:
    FrameBuilder();
    ~FrameBuilder();

    FrameBuilder& type(uint8_t type);
    FrameBuilder& stream_id(uint64_t stream_id);
    FrameBuilder& sequence(uint32_t sequence);
    FrameBuilder& flags(uint16_t flags);
    FrameBuilder& payload(const ByteBuffer& data);
    FrameBuilder& compressed(FrameFlags compression_flag);
    FrameBuilder& encrypted();
    FrameBuilder& fragmented(FrameFlags fragment_flag);

    Result<Frame> build();

private:
    std::unique_ptr<FrameImpl> impl_;
};

// Frame processor for serialization/validation
class FrameProcessor {
public:
    static Result<ByteBuffer> serialize(const Frame& frame);
    static Result<Frame> deserialize(const ByteBuffer& data);

    static bool has_flag(const Frame& frame, FrameFlags flag);
    static bool is_compressed(const Frame& frame);
    static bool is_encrypted(const Frame& frame);
    static bool is_fragmented(const Frame& frame);
};

} // namespace umicp

#endif // UMICP_FRAME_H
