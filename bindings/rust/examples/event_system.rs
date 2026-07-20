/*!
# Event System Example

Demonstrates EventEmitter with multiple subscribers.
*/

#[cfg(feature = "websocket")]
use std::sync::Arc;
#[cfg(feature = "websocket")]
use parking_lot::RwLock;
#[cfg(feature = "websocket")]
use tokio::time::{sleep, Duration};
#[cfg(feature = "websocket")]
use umicp_core::{EventEmitter, EventType, EventData, Envelope, OperationType, PeerInfo};
#[cfg(feature = "websocket")]
use serde_json::json;

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("🎭 Event System Example\n");

    // Create event emitter
    let emitter = EventEmitter::new();

    // Counter for tracking events
    let message_count = Arc::new(RwLock::new(0));
    let connect_count = Arc::new(RwLock::new(0));

    // Register message listener #1
    let msg_count_1 = Arc::clone(&message_count);
    emitter.on(
        EventType::Message,
        Arc::new(move |data| {
            if let EventData::Message { envelope, peer_id } = data {
                println!("[Listener 1] 📨 Message from {}: {} -> {}",
                    peer_id, envelope.from(), envelope.to());
                *msg_count_1.write() += 1;
            }
        }),
    );

    // Register message listener #2
    let msg_count_2 = Arc::clone(&message_count);
    emitter.on(
        EventType::Message,
        Arc::new(move |data| {
            if let EventData::Message { envelope, .. } = data {
                if let Some(caps) = envelope.capabilities() {
                    if let Some(msg) = caps.get("content") {
                        println!("[Listener 2] 💬 Content: {}", msg);
                    }
                }
                *msg_count_2.write() += 1;
            }
        }),
    );

    // Register peer connect listener
    let conn_count = Arc::clone(&connect_count);
    emitter.on(
        EventType::PeerConnect,
        Arc::new(move |data| {
            if let EventData::PeerConnect { peer_id, info } = data {
                println!("[Connection] ✅ Peer connected: {}", peer_id);
                println!("             Type: {:?}", info.peer_type);
                println!("             Capabilities: {:?}", info.capabilities);
                *conn_count.write() += 1;
            }
        }),
    );

    // Register one-time error listener
    emitter.once(
        EventType::Error,
        Arc::new(|data| {
            if let EventData::Error { message, peer_id } = data {
                println!("[Error - Once] ⚠️ Error: {}", message);
                if let Some(id) = peer_id {
                    println!("               Peer: {}", id);
                }
            }
        }),
    );

    println!("✓ {} listeners registered\n",
        emitter.listener_count(EventType::Message) +
        emitter.listener_count(EventType::PeerConnect) +
        emitter.listener_count(EventType::Error)
    );

    // Simulate events
    println!("🎬 Simulating events...\n");

    // Event 1: Peer connection
    sleep(Duration::from_millis(500)).await;
    let peer_info = PeerInfo::client("peer-001", "ws://localhost:8080")
        .with_capability("storage")
        .with_capability("compute")
        .with_metadata("region", "us-east");

    emitter.emit_peer_connect("peer-001".to_string(), peer_info);
    sleep(Duration::from_millis(300)).await;

    // Event 2: Message (will trigger 2 listeners)
    let envelope = Envelope::builder()
        .from("peer-001")
        .to("server")
        .operation(OperationType::Data)
        .capability("content", json!("Hello from peer!"))
        .build()?;

    emitter.emit_message(envelope, "peer-001".to_string());
    sleep(Duration::from_millis(300)).await;

    // Event 3: Another message
    let envelope2 = Envelope::builder()
        .from("peer-002")
        .to("server")
        .operation(OperationType::Data)
        .capability("content", json!("Second message"))
        .build()?;

    emitter.emit_message(envelope2, "peer-002".to_string());
    sleep(Duration::from_millis(300)).await;

    // Event 4: Error (one-time listener)
    emitter.emit_error("Connection timeout".to_string(), Some("peer-003".to_string()));
    sleep(Duration::from_millis(300)).await;

    // Event 5: Another error (should NOT trigger one-time listener)
    println!("\n[Note] Emitting second error (one-time listener won't fire)...");
    emitter.emit_error("Network error".to_string(), None);
    sleep(Duration::from_millis(300)).await;

    // Event 6: Handshake complete
    let peer_info2 = PeerInfo::client("peer-004", "ws://localhost:8081")
        .with_capability("processing")
        .with_version("1.0.0");

    emitter.emit_handshake_complete("peer-004".to_string(), peer_info2);
    sleep(Duration::from_millis(300)).await;

    // Event 7: Async emit
    println!("\n🚀 Testing async emit...");
    let envelope3 = Envelope::builder()
        .from("peer-async")
        .to("server")
        .operation(OperationType::Data)
        .capability("content", json!("Async message"))
        .build()?;

    emitter.emit_async(EventType::Message, EventData::Message {
        envelope: envelope3,
        peer_id: "peer-async".to_string(),
    }).await;

    sleep(Duration::from_millis(500)).await;

    // Show statistics
    println!("\n📊 Event Statistics:");
    println!("   Message events: {}", message_count.read());
    println!("   Connect events: {}", connect_count.read());
    println!("   Registered event types: {:?}", emitter.event_types());

    for event_type in emitter.event_types() {
        println!("   - {:?}: {} listeners", event_type, emitter.listener_count(event_type));
    }

    // Remove all message listeners
    println!("\n🧹 Removing all message listeners...");
    emitter.off(EventType::Message);
    println!("✓ Message listeners: {}", emitter.listener_count(EventType::Message));

    // Final message (should not trigger any listeners)
    println!("\n[Test] Emitting message after removing listeners...");
    let envelope4 = Envelope::builder()
        .from("final")
        .to("server")
        .operation(OperationType::Data)
        .capability("content", json!("Final message"))
        .build()?;

    emitter.emit_message(envelope4, "final".to_string());

    println!("\n✓ No listeners fired (as expected)");
    println!("\n✅ Event system demonstration complete!");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example event_system --features websocket");
}

