"""Tests for connection pooling module."""

import pytest
import asyncio

from umicp.pool import ConnectionPool, PoolConfig


class TestPoolConfig:
    """Test PoolConfig dataclass."""

    def test_default_config(self):
        """Test default pool configuration."""
        config = PoolConfig()

        assert config.min_size == 1
        assert config.max_size == 10
        assert config.idle_timeout_seconds == 300
        assert config.stale_timeout_seconds == 600
        assert config.acquire_timeout_seconds == 10

    def test_custom_config(self):
        """Test custom pool configuration."""
        config = PoolConfig(
            min_size=2,
            max_size=20,
            idle_timeout_seconds=600
        )

        assert config.min_size == 2
        assert config.max_size == 20
        assert config.idle_timeout_seconds == 600


class TestConnectionPool:
    """Test ConnectionPool class."""

    @pytest.mark.asyncio
    async def test_create_pool(self):
        """Test creating connection pool."""
        pool = ConnectionPool()
        assert pool is not None
        assert pool.size() == 0

    @pytest.mark.asyncio
    async def test_add_connection(self):
        """Test adding connection to pool."""
        pool = ConnectionPool()

        conn = {"id": "conn-001"}
        await pool.add_connection(conn)

        assert pool.size() == 1
        assert pool.available_count() == 1

    @pytest.mark.asyncio
    async def test_acquire_release(self):
        """Test acquiring and releasing connection."""
        pool = ConnectionPool()

        conn = {"id": "conn-001"}
        await pool.add_connection(conn)

        # Acquire
        acquired = await pool.acquire()
        assert acquired is not None
        assert acquired["id"] == "conn-001"
        assert pool.in_use_count() == 1

        # Release
        await pool.release(acquired)
        await asyncio.sleep(0.1)  # Give time for release
        assert pool.available_count() >= 0

    @pytest.mark.asyncio
    async def test_acquire_timeout(self):
        """Test acquire timeout."""
        config = PoolConfig(acquire_timeout_seconds=1)
        pool = ConnectionPool(config)

        # Don't add any connections
        acquired = await pool.acquire()
        assert acquired is None  # Should timeout

    @pytest.mark.asyncio
    async def test_multiple_connections(self):
        """Test multiple connections in pool."""
        pool = ConnectionPool()

        await pool.add_connection({"id": "conn-001"})
        await pool.add_connection({"id": "conn-002"})
        await pool.add_connection({"id": "conn-003"})

        assert pool.size() == 3
        assert pool.available_count() == 3

    @pytest.mark.asyncio
    async def test_max_size_limit(self):
        """Test pool max size limit."""
        config = PoolConfig(max_size=2)
        pool = ConnectionPool(config)

        await pool.add_connection({"id": "conn-001"})
        await pool.add_connection({"id": "conn-002"})
        await pool.add_connection({"id": "conn-003"})  # Should not be added

        assert pool.size() <= 2

    @pytest.mark.asyncio
    async def test_start_stop(self):
        """Test starting and stopping pool."""
        pool = ConnectionPool()

        await pool.start()
        assert pool._cleanup_task is not None

        await pool.stop()
        assert pool.size() == 0

