<?php

declare(strict_types=1);

namespace UMICP\Core;

use FFI;
use FFI\CData;
use UMICP\Exception\SerializationException;
use UMICP\FFI\FFIBridge;
use UMICP\FFI\Traits\AutoCleanup;

/**
 * UMICP Frame - Low-level frame structure for protocol communication
 *
 * @package UMICP\Core
 */
class Frame
{
    use AutoCleanup;

    private CData $nativeFrame;
    private FFIBridge $ffi;

    private int $type = 0;
    private int $streamId = 0;
    private int $sequence = 0;
    private int $flags = 0;

    public function __construct(
        int $type = 0,
        int $streamId = 0,
        int $sequence = 0,
        int $flags = 0,
        bool $compressed = false,
        bool $encrypted = false
    ) {
        $this->ffi = FFIBridge::getInstance();
        $this->nativeFrame = $this->ffi->createFrame();

        $this->registerCleanup(fn() => $this->ffi->destroyFrame($this->nativeFrame));

        $this->setType($type);
        $this->setStreamId($streamId);
        $this->setSequence($sequence);

        $flags = $this->calculateFlags($flags, $compressed, $encrypted);
        $this->setFlags($flags);
    }

    private function calculateFlags(int $baseFlags, bool $compressed, bool $encrypted): int
    {
        $flags = $baseFlags;
        if ($compressed) $flags |= 0x01;
        if ($encrypted) $flags |= 0x02;
        return $flags;
    }

    public function getType(): int
    {
        return $this->type;
    }

    public function setType(int $type): self
    {
        $this->type = $type;
        $this->ffi->getFFI()->umicp_frame_set_type($this->nativeFrame, $type);
        return $this;
    }

    public function getStreamId(): int
    {
        return $this->streamId;
    }

    public function setStreamId(int $streamId): self
    {
        $this->streamId = $streamId;
        $this->ffi->getFFI()->umicp_frame_set_stream_id($this->nativeFrame, $streamId);
        return $this;
    }

    public function getSequence(): int
    {
        return $this->sequence;
    }

    public function setSequence(int $sequence): self
    {
        $this->sequence = $sequence;
        $this->ffi->getFFI()->umicp_frame_set_sequence($this->nativeFrame, $sequence);
        return $this;
    }

    public function getFlags(): int
    {
        return $this->flags;
    }

    public function setFlags(int $flags): self
    {
        $this->flags = $flags;
        $this->ffi->getFFI()->umicp_frame_set_flags($this->nativeFrame, $flags);
        return $this;
    }

    public function isCompressed(): bool
    {
        return ($this->flags & 0x01) !== 0;
    }

    public function setCompressed(bool $compressed): self
    {
        if ($compressed) {
            $this->flags |= 0x01;
        } else {
            $this->flags &= ~0x01;
        }
        $this->setFlags($this->flags);
        return $this;
    }

    public function isEncrypted(): bool
    {
        return ($this->flags & 0x02) !== 0;
    }

    public function setEncrypted(bool $encrypted): self
    {
        if ($encrypted) {
            $this->flags |= 0x02;
        } else {
            $this->flags &= ~0x02;
        }
        $this->setFlags($this->flags);
        return $this;
    }

    public function serialize(): string
    {
        $cStr = $this->ffi->getFFI()->umicp_frame_serialize($this->nativeFrame);

        if ($cStr === null || FFI::isNull($cStr)) {
            throw new SerializationException('Failed to serialize frame');
        }

        return FFI::string($cStr);
    }

    public static function deserialize(string $data): self
    {
        $ffi = FFIBridge::getInstance();
        $nativeFrame = $ffi->getFFI()->umicp_frame_deserialize($data);

        if ($nativeFrame === null || FFI::isNull($nativeFrame)) {
            throw new SerializationException('Failed to deserialize frame');
        }

        $frame = new self();

        // Extract values from native frame
        $frame->type = $ffi->getFFI()->umicp_frame_get_type($nativeFrame);
        $frame->streamId = $ffi->getFFI()->umicp_frame_get_stream_id($nativeFrame);
        $frame->sequence = $ffi->getFFI()->umicp_frame_get_sequence($nativeFrame);
        $frame->flags = $ffi->getFFI()->umicp_frame_get_flags($nativeFrame);

        return $frame;
    }

    public function toArray(): array
    {
        return [
            'type' => $this->type,
            'streamId' => $this->streamId,
            'sequence' => $this->sequence,
            'flags' => $this->flags,
            'compressed' => $this->isCompressed(),
            'encrypted' => $this->isEncrypted(),
        ];
    }
}

