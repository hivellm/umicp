#!/usr/bin/env python3
"""WebSocket client example."""

import asyncio
from umicp import WebSocketClient, EnvelopeBuilder, OperationType


async def main():
    """Demonstrate WebSocket client."""
    print("=== UMICP Python - WebSocket Client Example ===\n")

    # Create client
    print("1. Creating WebSocket client...")
    client = WebSocketClient("ws://localhost:8080")

    try:
        # Connect
        print("2. Connecting to server...")
        await client.connect()
        print("   ✅ Connected!")

        # Send message
        print("\n3. Sending message...")
        envelope = EnvelopeBuilder() \
            .from_id("python-client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .capability("message", "Hello from Python!") \
            .capability("timestamp", "2025-10-10T12:00:00Z") \
            .build()

        await client.send(envelope)
        print("   ✅ Message sent!")

        # Get statistics
        print("\n4. Statistics:")
        stats = client.get_stats()
        print(f"   Messages sent: {stats.messages_sent}")
        print(f"   Bytes sent: {stats.bytes_sent}")
        print(f"   Messages received: {stats.messages_received}")
        print(f"   Errors: {stats.errors}")

    except Exception as e:
        print(f"   ❌ Error: {e}")

    finally:
        # Disconnect
        print("\n5. Disconnecting...")
        await client.disconnect()
        print("   ✅ Disconnected!")

    print("\n✅ Example completed!")


if __name__ == "__main__":
    asyncio.run(main())

