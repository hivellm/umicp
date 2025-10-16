<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Operation schema compatible with MCP JSON Schema
 */
class OperationSchema
{
    public function __construct(
        public readonly string $name,
        public readonly array $input_schema,
        public readonly ?string $title = null,
        public readonly ?string $description = null,
        public readonly ?array $output_schema = null,
        public readonly ?array $annotations = null
    ) {
    }

    /**
     * Convert to array for JSON serialization
     */
    public function toArray(): array
    {
        $result = [
            'name' => $this->name,
            'input_schema' => $this->input_schema,
        ];

        if ($this->title !== null) {
            $result['title'] = $this->title;
        }

        if ($this->description !== null) {
            $result['description'] = $this->description;
        }

        if ($this->output_schema !== null) {
            $result['output_schema'] = $this->output_schema;
        }

        if ($this->annotations !== null) {
            $result['annotations'] = $this->annotations;
        }

        return $result;
    }
}

