/**
 * UMICP HTTP Client usando SDK oficial
 *
 * Conecta a servidores UMICP via HTTP usando o SDK @hivehub/umicp-sdk
 */

import { Envelope, OperationType } from '@hivehub/umicp-sdk';

export interface HttpClientConfig {
    baseUrl: string;
    timeout?: number;
}

interface UmicpRequest {
    method: string;
    payload: any;
    metadata: Record<string, string>;
}

export class HttpClient {
    private config: HttpClientConfig;
    private requestId = 0;

    constructor(config: HttpClientConfig) {
        this.config = config;
    }

    /**
     * No-op connect for HTTP (stateless)
     */
    async connect(): Promise<void> {
        // HTTP is stateless, just verify the server is reachable
        try {
            const response = await fetch(`${this.config.baseUrl}/discover`, {
                method: 'GET',
                signal: AbortSignal.timeout(this.config.timeout || 5000),
            });

            if (!response.ok) {
                throw new Error(`Server not reachable: ${response.status}`);
            }
        } catch (error) {
            throw new Error(`Failed to connect to ${this.config.baseUrl}: ${error}`);
        }
    }

    /**
     * Execute a UMICP call via HTTP POST usando SDK oficial
     */
    async call(request: UmicpRequest): Promise<any> {
        this.requestId++;

        // Build UMICP Envelope usando SDK
        const envelope = new Envelope();
        envelope.setFrom('umicp-mcp-bridge');
        envelope.setTo('synap-server');
        envelope.setOperation(OperationType.DATA);  // Use enum

        // Set capabilities
        const capabilities = {
            operation: request.method,
            ...request.payload,
        };
        envelope.setCapabilities(capabilities);

        try {
            // Serialize envelope to JSON
            const envelopeJson = envelope.serialize();

            const response = await fetch(this.config.baseUrl, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: envelopeJson,
                signal: AbortSignal.timeout(this.config.timeout || 30000),
            });

            if (!response.ok) {
                const errorText = await response.text();
                throw new Error(`HTTP ${response.status}: ${errorText}`);
            }

            const resultText = await response.text();

            // Parse response envelope
            const responseEnvelope = Envelope.deserialize(resultText);

            // Check if it's an error response
            const caps = responseEnvelope.getCapabilities();  // Use getter
            if (caps && caps.error) {
                throw new Error(caps.error as string);
            }

            // Return capabilities as result
            return caps || {};
        } catch (error) {
            if (error instanceof Error) {
                throw error;
            }
            throw new Error(String(error));
        }
    }

    /**
     * No-op disconnect for HTTP (stateless)
     */
    async disconnect(): Promise<void> {
        // HTTP is stateless, nothing to disconnect
    }

    /**
     * Check if connected (always true for HTTP)
     */
    isConnected(): boolean {
        return true;
    }
}

