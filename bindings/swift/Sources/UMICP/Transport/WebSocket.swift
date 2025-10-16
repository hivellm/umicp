import Foundation

#if canImport(FoundationNetworking)
import FoundationNetworking
#endif

/// WebSocket transport for UMICP with async/await support
@available(macOS 12.0, iOS 15.0, *)
public actor UMICPWebSocket {
    private var webSocketTask: URLSessionWebSocketTask?
    private let url: URL
    private let session: URLSession
    private var isConnected: Bool = false
    private var messageHandler: ((Envelope) -> Void)?
    private var errorHandler: ((Error) -> Void)?
    
    public init(url: URL, configuration: URLSessionConfiguration = .default) {
        self.url = url
        self.session = URLSession(configuration: configuration)
    }
    
    /// Connect to WebSocket server
    public func connect() async throws {
        guard !isConnected else { return }
        
        webSocketTask = session.webSocketTask(with: url)
        webSocketTask?.resume()
        isConnected = true
        
        // Start receiving messages
        Task {
            await receiveMessages()
        }
    }
    
    /// Disconnect from WebSocket server
    public func disconnect() async {
        webSocketTask?.cancel(with: .goingAway, reason: nil)
        isConnected = false
        webSocketTask = nil
    }
    
    /// Send envelope over WebSocket
    public func send(envelope: Envelope) async throws {
        guard isConnected, let task = webSocketTask else {
            throw UMICPError.transportError("WebSocket not connected")
        }
        
        let json = try envelope.serialize()
        let message = URLSessionWebSocketTask.Message.string(json)
        try await task.send(message)
    }
    
    /// Set message handler
    public func onMessage(_ handler: @escaping (Envelope) -> Void) {
        self.messageHandler = handler
    }
    
    /// Set error handler
    public func onError(_ handler: @escaping (Error) -> Void) {
        self.errorHandler = handler
    }
    
    /// Receive messages in a loop
    private func receiveMessages() async {
        guard let task = webSocketTask else { return }
        
        do {
            let message = try await task.receive()
            
            switch message {
            case .string(let text):
                do {
                    let envelope = try Envelope.deserialize(from: text)
                    messageHandler?(envelope)
                } catch {
                    errorHandler?(error)
                }
            case .data(let data):
                do {
                    guard let text = String(data: data, encoding: .utf8) else {
                        throw UMICPError.serializationError("Invalid UTF-8 data")
                    }
                    let envelope = try Envelope.deserialize(from: text)
                    messageHandler?(envelope)
                } catch {
                    errorHandler?(error)
                }
            @unknown default:
                break
            }
            
            // Continue receiving if still connected
            if isConnected {
                Task {
                    await receiveMessages()
                }
            }
        } catch {
            errorHandler?(error)
            await disconnect()
        }
    }
    
    /// Check connection status
    public func getIsConnected() -> Bool {
        return isConnected
    }
}

