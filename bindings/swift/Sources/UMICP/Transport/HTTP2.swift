import Foundation

#if canImport(FoundationNetworking)
import FoundationNetworking
#endif

/// HTTP/2 transport for UMICP with async/await support
@available(macOS 12.0, iOS 15.0, *)
public struct UMICPHTTP2 {
    private let baseURL: URL
    private let session: URLSession
    
    public init(baseURL: URL) {
        self.baseURL = baseURL
        
        let configuration = URLSessionConfiguration.default
        configuration.httpAdditionalHeaders = [
            "Content-Type": "application/json",
            "Accept": "application/json"
        ]
        configuration.httpMaximumConnectionsPerHost = 10
        
        self.session = URLSession(configuration: configuration)
    }
    
    /// Send envelope via HTTP/2 POST
    public func send(envelope: Envelope, endpoint: String = "/umicp") async throws -> Envelope {
        let url = baseURL.appendingPathComponent(endpoint)
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        
        let json = try envelope.serialize()
        request.httpBody = json.data(using: .utf8)
        
        let (data, response) = try await session.data(for: request)
        
        guard let httpResponse = response as? HTTPURLResponse else {
            throw UMICPError.transportError("Invalid response type")
        }
        
        guard (200...299).contains(httpResponse.statusCode) else {
            throw UMICPError.transportError("HTTP error: \(httpResponse.statusCode)")
        }
        
        guard let responseText = String(data: data, encoding: .utf8) else {
            throw UMICPError.serializationError("Invalid UTF-8 response")
        }
        
        return try Envelope.deserialize(from: responseText)
    }
    
    /// Send GET request and receive envelope
    public func get(endpoint: String) async throws -> Envelope {
        let url = baseURL.appendingPathComponent(endpoint)
        var request = URLRequest(url: url)
        request.httpMethod = "GET"
        
        let (data, response) = try await session.data(for: request)
        
        guard let httpResponse = response as? HTTPURLResponse else {
            throw UMICPError.transportError("Invalid response type")
        }
        
        guard (200...299).contains(httpResponse.statusCode) else {
            throw UMICPError.transportError("HTTP error: \(httpResponse.statusCode)")
        }
        
        guard let responseText = String(data: data, encoding: .utf8) else {
            throw UMICPError.serializationError("Invalid UTF-8 response")
        }
        
        return try Envelope.deserialize(from: responseText)
    }
}

