"""HTTP/2 client implementation."""

import httpx
from umicp.envelope import Envelope
from umicp.types import TransportStats


class HttpClient:
    """Async HTTP/2 client."""

    def __init__(self, base_url: str) -> None:
        """Initialize HTTP client."""
        self.base_url = base_url
        self.stats = TransportStats()
        self._client = httpx.AsyncClient(http2=True)

    async def send(self, envelope: Envelope) -> None:
        """Send envelope via HTTP POST."""
        response = await self._client.post(
            f"{self.base_url}/message",
            json=envelope.to_dict()
        )
        response.raise_for_status()
        self.stats.messages_sent += 1

    async def close(self) -> None:
        """Close client."""
        await self._client.aclose()

