"""
Tests for UMICP Compression module
"""

import pytest
from umicp.compression import Compression, CompressionType, CompressionError


class TestCompressionType:
    """Test CompressionType enum"""

    def test_enum_values(self):
        """Test enum has expected values"""
        assert CompressionType.NONE.value == "none"
        assert CompressionType.GZIP.value == "gzip"
        assert CompressionType.DEFLATE.value == "deflate"
        assert CompressionType.LZ4.value == "lz4"

    def test_enum_count(self):
        """Test enum has correct number of values"""
        assert len(list(CompressionType)) == 4


class TestCompression:
    """Test Compression class"""

    def test_compress_decompress_gzip(self):
        """Test GZIP compression round-trip"""
        # Use larger, repetitive text to ensure compression works
        original = b"Hello, UMICP! " * 50 + b"This is a test message for compression. " * 50

        compressed = Compression.compress(original, CompressionType.GZIP)
        assert compressed is not None
        assert len(compressed) < len(original)  # Should be smaller with larger data

        decompressed = Compression.decompress(compressed, CompressionType.GZIP)
        assert decompressed == original

    def test_compress_decompress_deflate(self):
        """Test DEFLATE compression round-trip"""
        # Use larger, repetitive text to ensure compression works
        original = b"Hello, UMICP! " * 50 + b"This is a test message for deflate compression. " * 50

        compressed = Compression.compress(original, CompressionType.DEFLATE)
        assert compressed is not None
        assert len(compressed) < len(original)  # Should be smaller with larger data

        decompressed = Compression.decompress(compressed, CompressionType.DEFLATE)
        assert decompressed == original

    def test_compress_none(self):
        """Test no compression"""
        data = b"Test data"

        result = Compression.compress(data, CompressionType.NONE)
        assert result == data

        decompressed = Compression.decompress(result, CompressionType.NONE)
        assert decompressed == data

    def test_compress_large_data(self):
        """Test compression of large repetitive data"""
        # Create large repetitive data
        line = b"This is line %d of test data.\n"
        data = b"".join([line % i for i in range(1000)])

        compressed = Compression.compress(data, CompressionType.GZIP)
        assert len(compressed) < len(data) / 2  # Should compress significantly

        decompressed = Compression.decompress(compressed, CompressionType.GZIP)
        assert decompressed == data

    def test_compress_empty_data(self):
        """Test compression of empty data"""
        data = b""

        compressed = Compression.compress(data, CompressionType.GZIP)
        assert compressed is not None

        decompressed = Compression.decompress(compressed, CompressionType.GZIP)
        assert len(decompressed) == 0

    def test_compress_invalid_data(self):
        """Test compression with invalid data"""
        with pytest.raises(ValueError):
            Compression.compress("not bytes", CompressionType.GZIP)  # type: ignore

    def test_compress_invalid_type(self):
        """Test compression with invalid type"""
        data = b"test"
        with pytest.raises(ValueError):
            Compression.compress(data, "invalid")  # type: ignore

    def test_decompress_invalid_data(self):
        """Test decompression of invalid data"""
        invalid_data = b"not compressed data"

        with pytest.raises(CompressionError):
            Compression.decompress(invalid_data, CompressionType.GZIP)

    def test_compress_lz4_not_implemented(self):
        """Test LZ4 raises not implemented error"""
        data = b"test"

        with pytest.raises(CompressionError, match="not yet implemented"):
            Compression.compress(data, CompressionType.LZ4)

    def test_decompress_lz4_not_implemented(self):
        """Test LZ4 decompression raises not implemented error"""
        data = b"test"

        with pytest.raises(CompressionError, match="not yet implemented"):
            Compression.decompress(data, CompressionType.LZ4)

    def test_compression_ratio(self):
        """Test compression ratio calculation"""
        ratio = Compression.get_compression_ratio(1000, 500)
        assert ratio == 2.0

        ratio = Compression.get_compression_ratio(1000, 250)
        assert ratio == 4.0

        ratio = Compression.get_compression_ratio(1000, 0)
        assert ratio == 0.0

    def test_is_beneficial(self):
        """Test beneficial compression detection"""
        # 50% compression is beneficial
        assert Compression.is_beneficial(1000, 500)

        # 15% compression is beneficial
        assert Compression.is_beneficial(1000, 850)

        # 5% compression is not beneficial (threshold is 10%)
        assert not Compression.is_beneficial(1000, 950)

        # No compression is not beneficial
        assert not Compression.is_beneficial(1000, 1000)

        # Expansion is not beneficial
        assert not Compression.is_beneficial(1000, 1100)

    def test_is_beneficial_custom_threshold(self):
        """Test beneficial compression with custom threshold"""
        # With 80% threshold (20% savings required)
        assert Compression.is_beneficial(1000, 700, threshold=0.8)
        assert not Compression.is_beneficial(1000, 850, threshold=0.8)

    def test_round_trip_unicode(self):
        """Test compression with Unicode data"""
        original = "Special chars: 你好世界 🌍 émojis 😀 symbols: @#$%^&*()".encode('utf-8')

        compressed = Compression.compress(original, CompressionType.GZIP)
        decompressed = Compression.decompress(compressed, CompressionType.GZIP)

        assert decompressed == original
        assert decompressed.decode('utf-8') == "Special chars: 你好世界 🌍 émojis 😀 symbols: @#$%^&*()"

    def test_gzip_vs_deflate(self):
        """Test GZIP vs DEFLATE compression"""
        # Use larger, repetitive text to ensure compression works effectively
        text = b"This is a test message that will be compressed using different algorithms. " * 100

        gzip_compressed = Compression.compress(text, CompressionType.GZIP)
        deflate_compressed = Compression.compress(text, CompressionType.DEFLATE)

        # Both should compress significantly with larger data
        assert len(gzip_compressed) < len(text)
        assert len(deflate_compressed) < len(text)

        # DEFLATE should be slightly smaller (no GZIP headers)
        assert len(deflate_compressed) < len(gzip_compressed)

        # Both should decompress correctly
        gzip_decompressed = Compression.decompress(gzip_compressed, CompressionType.GZIP)
        deflate_decompressed = Compression.decompress(deflate_compressed, CompressionType.DEFLATE)

        assert gzip_decompressed == text
        assert deflate_decompressed == text

    def test_multiple_compression_cycles(self):
        """Test multiple compression/decompression cycles"""
        original = b"Test data for multiple compression cycles"

        # Compress and decompress multiple times
        for _ in range(5):
            compressed = Compression.compress(original, CompressionType.GZIP)
            decompressed = Compression.decompress(compressed, CompressionType.GZIP)
            assert decompressed == original

