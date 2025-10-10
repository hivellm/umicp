/*!
# Peer Module

Multiplexed peer architecture for UMICP.
*/

pub mod connection;
pub mod handshake;
pub mod info;
pub mod websocket_peer;

pub use connection::{ConnectionState, ConnectionStats, PeerConnection};
pub use handshake::{HandshakeMessage, HandshakeProtocol, HandshakeType};
pub use info::{PeerInfo, PeerType};
pub use websocket_peer::{WebSocketPeer, WebSocketPeerConfig};

