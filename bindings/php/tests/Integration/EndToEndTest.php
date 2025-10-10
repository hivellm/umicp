<?php

declare(strict_types=1);

namespace UMICP\Tests\Integration;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\Frame;
use UMICP\Core\OperationType;
use UMICP\Core\PayloadType;
use UMICP\Core\EncodingType;
use UMICP\Core\PayloadHint;
use UMICP\FFI\Config;

/**
 * End-to-end integration tests
 *
 * @group integration
 * @group e2e
 */
class EndToEndTest extends TestCase
{
    public function testCompleteWorkflow(): void
    {
        // 1. Create envelope
        $envelope = new Envelope(
            from: 'coordinator',
            to: 'worker-001',
            operation: OperationType::DATA,
            messageId: 'workflow-001',
            capabilities: [
                'task' => 'federated_learning',
                'epoch' => '1'
            ]
        );

        // 2. Add payload hint
        $hint = new PayloadHint(
            type: PayloadType::VECTOR,
            size: 512,
            encoding: EncodingType::FLOAT32,
            count: 128
        );
        $envelope->setPayloadHint($hint);

        // 3. Serialize
        $json = $envelope->serialize();
        $this->assertNotEmpty($json);

        // 4. Simulate network transfer
        $received = Envelope::deserialize($json);

        // 5. Validate
        $this->assertTrue($received->validate());

        // 6. Process (matrix operation)
        $matrix = new Matrix();
        $weights = array_fill(0, 128, 0.5);
        $gradients = array_fill(0, 128, 0.1);

        $updated = $matrix->vectorAdd($weights, $gradients);

        // 7. Verify result
        $this->assertCount(128, $updated);
        $this->assertEqualsWithDelta(0.6, $updated[0], 0.0001);

        // 8. Create response
        $response = new Envelope(
            from: 'worker-001',
            to: 'coordinator',
            operation: OperationType::ACK,
            messageId: 'response-001',
            capabilities: [
                'status' => 'completed',
                'original_task' => $received->getCapability('task')
            ]
        );

        // 9. Verify response
        $this->assertEquals('federated_learning', $response->getCapability('original_task'));
        $this->assertTrue($response->validate());
    }

    public function testMultiAgentScenario(): void
    {
        $agents = ['alpha', 'beta', 'gamma'];
        $envelopes = [];

        // Each agent creates a message
        foreach ($agents as $agent) {
            $envelopes[$agent] = new Envelope(
                from: "agent-$agent",
                to: 'coordinator',
                operation: OperationType::REQUEST,
                messageId: "$agent-request-" . uniqid(),
                capabilities: [
                    'agent_id' => $agent,
                    'status' => 'ready'
                ]
            );
        }

        // Coordinator processes all
        $responses = [];
        foreach ($envelopes as $agentId => $envelope) {
            $this->assertTrue($envelope->validate());

            $responses[$agentId] = new Envelope(
                from: 'coordinator',
                to: $envelope->getFrom(),
                operation: OperationType::RESPONSE,
                capabilities: [
                    'task' => 'assigned',
                    'agent' => $agentId
                ]
            );
        }

        $this->assertCount(3, $responses);
        foreach ($responses as $response) {
            $this->assertTrue($response->validate());
        }
    }

    public function testDataPipeline(): void
    {
        // Simulate data processing pipeline
        $matrix = new Matrix();

        // Step 1: Raw data
        $rawData = [1.0, 2.0, 3.0, 4.0, 5.0];

        // Step 2: Normalize
        $normalized = $matrix->vectorNormalize($rawData);
        $magnitude = $matrix->vectorMagnitude($normalized);
        $this->assertEqualsWithDelta(1.0, $magnitude, 0.0001);

        // Step 3: Transform (scale)
        $transformed = $matrix->vectorScale($normalized, 10.0);

        // Step 4: Package in envelope
        $envelope = new Envelope(
            from: 'preprocessor',
            to: 'model',
            operation: OperationType::DATA,
            capabilities: [
                'data_type' => 'embedding',
                'dimension' => (string) count($transformed),
                'processed' => 'true'
            ]
        );

        // Step 5: Serialize and transfer
        $json = $envelope->serialize();
        $received = Envelope::deserialize($json);

        // Step 6: Verify integrity
        $this->assertTrue($received->validate());
        $this->assertEquals('embedding', $received->getCapability('data_type'));
        $this->assertEquals('5', $received->getCapability('dimension'));
    }

    public function testConfigurationIntegration(): void
    {
        try {
            // Access configuration
            Config::load();

            $libPath = Config::get('ffi.lib_path');
            $this->assertNotNull($libPath);

            $timeout = Config::get('transport.default_timeout');
            $this->assertIsInt($timeout);

            $port = Config::get('server.default_port');
            $this->assertIsInt($port);

            // Test dot notation
            $this->assertTrue(Config::has('ffi.lib_path'));
            $this->assertFalse(Config::has('nonexistent.key'));

        } catch (\RuntimeException $e) {
            $this->markTestSkipped('Config file not found');
        }
    }

    public function testFrameWithEnvelopeIntegration(): void
    {
        $frame = new Frame(
            type: 1,
            streamId: 100,
            sequence: 1,
            compressed: true
        );

        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::DATA,
            capabilities: [
                'frame_type' => (string) $frame->getType(),
                'stream_id' => (string) $frame->getStreamId()
            ]
        );

        $this->assertEquals((string) $frame->getType(), $envelope->getCapability('frame_type'));
        $this->assertEquals((string) $frame->getStreamId(), $envelope->getCapability('stream_id'));
    }
}

