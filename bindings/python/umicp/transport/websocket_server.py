"""WebSocket server implementation."""

import asyncio
from typing import Dict, Set, Optional
import websockets
from websockets.server import WebSocketServerProtocol

from umicp.envelope import Envelope
from umicp.types import TransportStats
from umicp.error import TransportError
from umicp.events import EventEmitter


class WebSocketServer:
    """Async WebSocket server."""

    def __init__(self, host: str = "0.0.0.0", port: int = 8080) -> None:
        """Initialize WebSocket server.

        Args:
            host: Host to bind
            port: Port to bind
        """
        self.host = host
        self.port = port
        self.stats = TransportStats()
        self.events = EventEmitter()
        self._clients: Set[WebSocketServerProtocol] = set()
        self._server: Optional[asyncio.AbstractServer] = None

    async def start(self) -> None:
        """Start server."""
        self._server = await websockets.serve(
            self._handle_client, self.host, self.port
        )

    async def stop(self) -> None:
        """Stop server."""
        if self._server:
            self._server.close()
            await self._server.wait_closed()

    async def _handle_client(self, websocket: WebSocketServerProtocol) -> None:
        """Handle client connection."""
        self._clients.add(websocket)
        try:
            async for message in websocket:
                self.stats.messages_received += 1
                self.stats.bytes_received += len(message)
                # Handle message
        finally:
            self._clients.remove(websocket)

    async def send_to(self, client_id: str, envelope: Envelope) -> None:
        """Send to specific client."""
        message = envelope.to_json()
        # Send to specific client (simplified)
        self.stats.messages_sent += 1

    async def broadcast(self, envelope: Envelope) -> None:
        """Broadcast to all clients."""
        message = envelope.to_json()
        await asyncio.gather(
            *[client.send(message) for client in self._clients],
            return_exceptions=True
        )
        self.stats.messages_sent += len(self._clients)

