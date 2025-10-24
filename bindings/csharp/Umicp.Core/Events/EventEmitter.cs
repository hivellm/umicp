namespace Umicp.SDK.Events;

/// <summary>
/// Event types for UMICP
/// </summary>
public enum EventType
{
    Connect,
    Disconnect,
    Message,
    Error,
    DataReceived,
    DataSent,
    PeerConnected,
    PeerDisconnected,
    Handshake,
    Timeout
}

/// <summary>
/// Event data for UMICP events
/// </summary>
public class UmicpEvent
{
    public EventType Type { get; set; }
    public DateTime Timestamp { get; set; } = DateTime.UtcNow;
    public Dictionary<string, object> Data { get; set; } = new();

    public UmicpEvent(EventType type)
    {
        Type = type;
    }

    public UmicpEvent(EventType type, Dictionary<string, object> data)
    {
        Type = type;
        Data = data;
    }
}

/// <summary>
/// Event emitter for UMICP - provides observer pattern implementation
/// </summary>
public class EventEmitter
{
    private readonly Dictionary<EventType, List<Action<UmicpEvent>>> _handlers = new();
    private readonly object _lock = new();

    /// <summary>
    /// Subscribe to an event
    /// </summary>
    public void On(EventType eventType, Action<UmicpEvent> handler)
    {
        lock (_lock)
        {
            if (!_handlers.ContainsKey(eventType))
                _handlers[eventType] = new List<Action<UmicpEvent>>();

            _handlers[eventType].Add(handler);
        }
    }

    /// <summary>
    /// Subscribe to an event (one-time only)
    /// </summary>
    public void Once(EventType eventType, Action<UmicpEvent> handler)
    {
        Action<UmicpEvent>? wrappedHandler = null;
        wrappedHandler = (evt) =>
        {
            handler(evt);
            if (wrappedHandler != null)
                Off(eventType, wrappedHandler);
        };

        On(eventType, wrappedHandler);
    }

    /// <summary>
    /// Unsubscribe from an event
    /// </summary>
    public void Off(EventType eventType, Action<UmicpEvent> handler)
    {
        lock (_lock)
        {
            if (_handlers.TryGetValue(eventType, out var handlers))
            {
                handlers.Remove(handler);

                if (handlers.Count == 0)
                    _handlers.Remove(eventType);
            }
        }
    }

    /// <summary>
    /// Unsubscribe all handlers for an event type
    /// </summary>
    public void RemoveAllListeners(EventType? eventType = null)
    {
        lock (_lock)
        {
            if (eventType.HasValue)
                _handlers.Remove(eventType.Value);
            else
                _handlers.Clear();
        }
    }

    /// <summary>
    /// Emit an event to all subscribers
    /// </summary>
    public void Emit(EventType eventType, Dictionary<string, object>? data = null)
    {
        var evt = new UmicpEvent(eventType, data ?? new Dictionary<string, object>());

        List<Action<UmicpEvent>> handlers;
        lock (_lock)
        {
            if (!_handlers.TryGetValue(eventType, out var handlerList))
                return;

            // Create a copy to avoid modification during iteration
            handlers = new List<Action<UmicpEvent>>(handlerList);
        }

        // Execute handlers outside the lock to avoid deadlocks
        foreach (var handler in handlers)
        {
            try
            {
                handler(evt);
            }
            catch (Exception ex)
            {
                // Log error but don't propagate to prevent one handler from affecting others
                Console.Error.WriteLine($"Error in event handler for {eventType}: {ex.Message}");
            }
        }
    }

    /// <summary>
    /// Emit an event asynchronously
    /// </summary>
    public async Task EmitAsync(EventType eventType, Dictionary<string, object>? data = null)
    {
        await Task.Run(() => Emit(eventType, data));
    }

    /// <summary>
    /// Get listener count for an event type
    /// </summary>
    public int ListenerCount(EventType eventType)
    {
        lock (_lock)
        {
            return _handlers.TryGetValue(eventType, out var handlers) ? handlers.Count : 0;
        }
    }

    /// <summary>
    /// Get all event types that have listeners
    /// </summary>
    public EventType[] EventTypes()
    {
        lock (_lock)
        {
            return _handlers.Keys.ToArray();
        }
    }
}

