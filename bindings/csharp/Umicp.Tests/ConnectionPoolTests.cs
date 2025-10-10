using Xunit;
using Umicp.Core.Pool;

namespace Umicp.Tests;

public class ConnectionPoolTests
{
    [Fact]
    public void PoolConfig_ShouldInitializeWithDefaults()
    {
        // Arrange & Act
        var config = new PoolConfig();

        // Assert
        Assert.Equal(1, config.MinConnections);
        Assert.Equal(10, config.MaxConnections);
        Assert.Equal(TimeSpan.FromSeconds(30), config.ConnectionTimeout);
        Assert.Equal(TimeSpan.FromMinutes(5), config.IdleTimeout);
        Assert.Equal(TimeSpan.FromSeconds(10), config.AcquireTimeout);
        Assert.True(config.ValidateOnAcquire);
        Assert.True(config.AutoReconnect);
    }

    [Fact]
    public void PoolConfig_ShouldValidateMinConnections()
    {
        // Arrange
        var config = new PoolConfig { MinConnections = -1 };

        // Act & Assert
        Assert.Throws<ArgumentException>(() => config.Validate());
    }

    [Fact]
    public void PoolConfig_ShouldValidateMaxConnections()
    {
        // Arrange
        var config = new PoolConfig
        {
            MinConnections = 10,
            MaxConnections = 5
        };

        // Act & Assert
        Assert.Throws<ArgumentException>(() => config.Validate());
    }

    [Fact]
    public void PoolConfig_ShouldValidateConnectionTimeout()
    {
        // Arrange
        var config = new PoolConfig
        {
            ConnectionTimeout = TimeSpan.Zero
        };

        // Act & Assert
        Assert.Throws<ArgumentException>(() => config.Validate());
    }

    [Fact]
    public void PoolConfig_ShouldPassValidation()
    {
        // Arrange
        var config = new PoolConfig
        {
            MinConnections = 2,
            MaxConnections = 10,
            ConnectionTimeout = TimeSpan.FromSeconds(30)
        };

        // Act & Assert (should not throw)
        config.Validate();
    }

    [Fact]
    public void ConnectionPool_ShouldInitialize()
    {
        // Arrange & Act
        using var pool = new ConnectionPool("ws://localhost:8080");

        // Assert
        Assert.NotNull(pool.Events);
        Assert.Equal(0, pool.TotalConnections);
        Assert.Equal(0, pool.AvailableConnections);
        Assert.Equal(0, pool.ActiveConnections);
    }

    [Fact]
    public void ConnectionPool_ShouldInitializeWithCustomConfig()
    {
        // Arrange
        var config = new PoolConfig
        {
            MinConnections = 3,
            MaxConnections = 15
        };

        // Act
        using var pool = new ConnectionPool("ws://localhost:8080", config);

        // Assert
        Assert.NotNull(pool);
    }

    [Fact]
    public void ConnectionPool_ShouldRejectInvalidConfig()
    {
        // Arrange
        var config = new PoolConfig
        {
            MinConnections = -1
        };

        // Act & Assert
        Assert.Throws<ArgumentException>(() => 
            new ConnectionPool("ws://localhost:8080", config)
        );
    }
}

