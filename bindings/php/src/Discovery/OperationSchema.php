<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Operation schema compatible with MCP JSON Schema
 *
 * @package UMICP\Discovery
 */
class OperationSchema
{
    /**
     * @param string $name Operation name
     * @param array<string, mixed> $inputSchema JSON Schema for input parameters
     * @param string|null $title Human-readable operation title
     * @param string|null $description Operation description
     * @param array<string, mixed>|null $outputSchema JSON Schema for output/response
     * @param array<string, mixed>|null $annotations Additional metadata annotations
     */
    public function __construct(
        public readonly string $name,
        public readonly array $inputSchema,
        public readonly ?string $title = null,
        public readonly ?string $description = null,
        public readonly ?array $outputSchema = null,
        public readonly ?array $annotations = null
    ) {
        if (empty($name)) {
            throw new \InvalidArgumentException('Operation name cannot be empty');
        }
    }

    /**
     * Convert to array for JSON serialization
     *
     * @return array<string, mixed>
     */
    public function toArray(): array
    {
        $result = [
            'name' => $this->name,
            'input_schema' => $this->inputSchema,
        ];

        if ($this->title !== null) {
            $result['title'] = $this->title;
        }
        if ($this->description !== null) {
            $result['description'] = $this->description;
        }
        if ($this->outputSchema !== null) {
            $result['output_schema'] = $this->outputSchema;
        }
        if ($this->annotations !== null) {
            $result['annotations'] = $this->annotations;
        }

        return $result;
    }
}
