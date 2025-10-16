<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Builder for OperationSchema
 */
class OperationSchemaBuilder
{
    private ?string $title = null;
    private ?string $description = null;
    private ?array $output_schema = null;
    private ?array $annotations = null;

    public function __construct(
        private readonly string $name,
        private readonly array $input_schema
    ) {
    }

    public function withTitle(string $title): self
    {
        $this->title = $title;
        return $this;
    }

    public function withDescription(string $description): self
    {
        $this->description = $description;
        return $this;
    }

    public function withOutputSchema(array $schema): self
    {
        $this->output_schema = $schema;
        return $this;
    }

    public function withAnnotations(array $annotations): self
    {
        $this->annotations = $annotations;
        return $this;
    }

    public function build(): OperationSchema
    {
        return new OperationSchema(
            $this->name,
            $this->input_schema,
            $this->title,
            $this->description,
            $this->output_schema,
            $this->annotations
        );
    }
}

