import * as net from 'net';

/**
 * Simple UMICP Echo Server for Testing
 *
 * This is a minimal UMICP server implementation for testing the bridge.
 * It listens on port 8080 and echoes back any request it receives.
 */

const UMICP_MAGIC = Buffer.from([0x55, 0x4D, 0x49, 0x43]); // "UMIC"
const UMICP_VERSION = 1;

enum MessageType {
  REQUEST = 0x01,
  RESPONSE = 0x02,
  ERROR = 0x03,
}

const server = net.createServer((socket) => {
  console.log('Client connected');

  let buffer = Buffer.alloc(0);

  socket.on('data', (chunk) => {
    buffer = Buffer.concat([buffer, chunk]);

    // Try to parse messages
    while (buffer.length >= 16) {
      const messageLength = tryParseMessage(buffer);
      if (messageLength === -1) break;

      const message = buffer.slice(0, messageLength);
      buffer = buffer.slice(messageLength);

      handleMessage(socket, message);
    }
  });

  socket.on('end', () => {
    console.log('Client disconnected');
  });

  socket.on('error', (err) => {
    console.error('Socket error:', err);
  });
});

function tryParseMessage(buffer: Buffer): number {
  if (buffer.length < 16) return -1;

  if (!buffer.slice(0, 4).equals(UMICP_MAGIC)) {
    throw new Error('Invalid UMICP magic bytes');
  }

  const payloadLength = buffer.readUInt32LE(8);
  const metadataLength = buffer.readUInt32LE(12);
  const totalLength = 16 + payloadLength + metadataLength;

  if (buffer.length < totalLength) return -1;

  return totalLength;
}

function handleMessage(socket: net.Socket, buffer: Buffer): void {
  try {
    // Parse header
    const version = buffer.readUInt8(4);
    const messageType = buffer.readUInt8(5);
    const requestId = buffer.readUInt16LE(6);
    const payloadLength = buffer.readUInt32LE(8);
    const metadataLength = buffer.readUInt32LE(12);

    // Extract payload and metadata
    const payload = buffer.slice(16, 16 + payloadLength);
    const metadata = buffer.slice(16 + payloadLength, 16 + payloadLength + metadataLength);

    let data: any = {};
    if (payloadLength > 0) {
      data = JSON.parse(payload.toString('utf8'));
    }

    let metadataObj: Record<string, string> = {};
    if (metadataLength > 0) {
      metadataObj = JSON.parse(metadata.toString('utf8'));
    }

    console.log('Received request:', {
      requestId,
      method: metadataObj.method,
      data,
    });

    // Create echo response
    const response = {
      echo: data,
      receivedMethod: metadataObj.method,
      timestamp: new Date().toISOString(),
    };

    // Send response
    sendResponse(socket, requestId, response, metadataObj);
  } catch (error) {
    console.error('Error handling message:', error);
  }
}

function sendResponse(
  socket: net.Socket,
  requestId: number,
  data: any,
  originalMetadata: Record<string, string>
): void {
  const payloadBuffer = Buffer.from(JSON.stringify(data), 'utf8');
  const metadataBuffer = Buffer.from(JSON.stringify(originalMetadata), 'utf8');

  const header = Buffer.alloc(16);
  UMICP_MAGIC.copy(header, 0);
  header.writeUInt8(UMICP_VERSION, 4);
  header.writeUInt8(MessageType.RESPONSE, 5);
  header.writeUInt16LE(requestId, 6);
  header.writeUInt32LE(payloadBuffer.length, 8);
  header.writeUInt32LE(metadataBuffer.length, 12);

  const message = Buffer.concat([header, payloadBuffer, metadataBuffer]);
  socket.write(message);
}

const PORT = 8080;
server.listen(PORT, () => {
  console.log(`UMICP Echo Server listening on port ${PORT}`);
  console.log('Ready to receive UMICP requests');
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\nShutting down server...');
  server.close(() => {
    console.log('Server closed');
    process.exit(0);
  });
});


