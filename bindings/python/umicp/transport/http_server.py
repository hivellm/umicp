"""HTTP/2 server implementation."""

from aiohttp import web
from umicp.envelope import Envelope
from umicp.types import TransportStats


class HttpServer:
    """Async HTTP/2 server."""

    def __init__(self, host: str = "0.0.0.0", port: int = 8080) -> None:
        """Initialize HTTP server."""
        self.host = host
        self.port = port
        self.stats = TransportStats()
        self.app = web.Application()
        self.app.router.add_post("/message", self._handle_message)

    async def _handle_message(self, request: web.Request) -> web.Response:
        """Handle incoming message."""
        data = await request.json()
        envelope = Envelope.from_dict(data)
        self.stats.messages_received += 1
        return web.Response(text="OK")

    async def start(self) -> None:
        """Start server."""
        runner = web.AppRunner(self.app)
        await runner.setup()
        site = web.TCPSite(runner, self.host, self.port)
        await site.start()

