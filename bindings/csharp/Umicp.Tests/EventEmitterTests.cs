using Xunit;
using Umicp.SDK.Events;

namespace Umicp.Tests;

public class EventEmitterTests
{
    [Fact]
    public void On_ShouldRegisterEventHandler()
    {
        // Arrange
        var emitter = new EventEmitter();
        var called = false;

        // Act
        emitter.On(EventType.Connect, evt => called = true);
        emitter.Emit(EventType.Connect);

        // Assert
        Assert.True(called);
    }

    [Fact]
    public void Once_ShouldCallHandlerOnlyOnce()
    {
        // Arrange
        var emitter = new EventEmitter();
        var callCount = 0;

        // Act
        emitter.Once(EventType.Connect, evt => callCount++);
        emitter.Emit(EventType.Connect);
        emitter.Emit(EventType.Connect);

        // Assert
        Assert.Equal(1, callCount);
    }

    [Fact]
    public void Off_ShouldUnregisterEventHandler()
    {
        // Arrange
        var emitter = new EventEmitter();
        var called = false;
        Action<UmicpEvent> handler = evt => called = true;

        // Act
        emitter.On(EventType.Connect, handler);
        emitter.Off(EventType.Connect, handler);
        emitter.Emit(EventType.Connect);

        // Assert
        Assert.False(called);
    }

    [Fact]
    public void Emit_ShouldPassDataToHandlers()
    {
        // Arrange
        var emitter = new EventEmitter();
        string? receivedValue = null;

        emitter.On(EventType.Message, evt =>
        {
            if (evt.Data.TryGetValue("test", out var value))
                receivedValue = value.ToString();
        });

        // Act
        emitter.Emit(EventType.Message, new Dictionary<string, object>
        {
            ["test"] = "hello"
        });

        // Assert
        Assert.Equal("hello", receivedValue);
    }

    [Fact]
    public void ListenerCount_ShouldReturnCorrectCount()
    {
        // Arrange
        var emitter = new EventEmitter();

        // Act
        emitter.On(EventType.Connect, evt => { });
        emitter.On(EventType.Connect, evt => { });
        var count = emitter.ListenerCount(EventType.Connect);

        // Assert
        Assert.Equal(2, count);
    }

    [Fact]
    public void RemoveAllListeners_ShouldRemoveAllHandlers()
    {
        // Arrange
        var emitter = new EventEmitter();
        emitter.On(EventType.Connect, evt => { });
        emitter.On(EventType.Disconnect, evt => { });

        // Act
        emitter.RemoveAllListeners();

        // Assert
        Assert.Equal(0, emitter.ListenerCount(EventType.Connect));
        Assert.Equal(0, emitter.ListenerCount(EventType.Disconnect));
    }
}

