#!/usr/bin/env python3
"""
Test UMICP integration with Vectorizer service

This script demonstrates how to use the UMICP Python SDK
with the Vectorizer service which uses the /umicp endpoint.
"""

import asyncio
import sys
from pathlib import Path

# Add umicp to path
sys.path.insert(0, str(Path(__file__).parent / "python"))

from umicp.transport.http_client import HttpClient
from umicp.envelope import Envelope
from umicp.types import OperationType


async def test_vectorizer_connection():
    """Test basic connection to Vectorizer"""
    print("🔌 Testing Vectorizer connection...")

    # Create client with /umicp endpoint (Vectorizer-specific)
    client = HttpClient(
        base_url="http://localhost:8000",
        path="/umicp"  # Vectorizer uses /umicp instead of /message
    )

    try:
        # Create a test envelope
        envelope = Envelope(
            from_="test-client",
            to="vectorizer",
            operation=OperationType.REQUEST,
            message_id="test-001",
            capabilities={
                "operation": "discover",
                "type": "operations"
            }
        )

        print(f"📤 Sending envelope to http://localhost:8000/umicp")
        print(f"   From: {envelope.from_}")
        print(f"   To: {envelope.to}")
        print(f"   Operation: {envelope.operation}")

        # Send envelope
        await client.send(envelope)

        print("✅ Successfully sent envelope to Vectorizer!")
        print(f"📊 Stats: {client.stats}")

        return True

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

    finally:
        await client.close()


async def test_custom_operation():
    """Test a custom operation with Vectorizer"""
    print("\n🛠️  Testing custom operation...")

    client = HttpClient(
        base_url="http://localhost:8000",
        path="/umicp"
    )

    try:
        envelope = Envelope(
            from_="test-client",
            to="vectorizer",
            operation=OperationType.DATA,
            message_id="test-002",
            capabilities={
                "action": "query",
                "collection": "test_collection",
                "query": "test query"
            }
        )

        print(f"📤 Sending custom operation...")
        await client.send(envelope)
        print("✅ Custom operation successful!")

        return True

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

    finally:
        await client.close()


async def main():
    """Run all tests"""
    print("=" * 60)
    print("UMICP Vectorizer Integration Test")
    print("=" * 60)

    # Test 1: Basic connection
    result1 = await test_vectorizer_connection()

    # Test 2: Custom operation
    result2 = await test_custom_operation()

    # Summary
    print("\n" + "=" * 60)
    print("Test Summary:")
    print(f"  Connection Test: {'✅ PASS' if result1 else '❌ FAIL'}")
    print(f"  Operation Test:  {'✅ PASS' if result2 else '❌ FAIL'}")
    print("=" * 60)

    return 0 if (result1 and result2) else 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)

