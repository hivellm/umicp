## UMICP Go Bindings - Examples

This directory contains complete working examples demonstrating various UMICP features.

---

## 📝 Available Examples

### 1. Basic Envelope (`basic/`)
**File**: `basic/main.go`  
**Complexity**: ⭐ Beginner  
**Duration**: 1 minute

**Features**:
- Create envelopes with builder pattern
- Serialize/deserialize
- Hash calculation
- Cloning
- Different operation types

**Run**:
```bash
go run examples/basic/main.go
```

---

### 2. WebSocket Communication (`websocket_example/`)
**File**: `websocket_example/main.go`  
**Complexity**: ⭐⭐ Intermediate  
**Duration**: 2 minutes

**Features**:
- WebSocket server setup
- WebSocket client connection
- Request-response pattern
- Event handlers
- Statistics

**Run**:
```bash
go run examples/websocket_example/main.go
```

---

### 3. HTTP/2 Transport (`http_example/`)
**File**: `http_example/main.go`  
**Complexity**: ⭐⭐ Intermediate  
**Duration**: 2 minutes

**Features**:
- HTTP/2 server
- HTTP/2 client
- Request-response over HTTP
- Statistics

**Run**:
```bash
go run examples/http_example/main.go
```

---

### 4. Multiplexed Peer (`peer_example/`)
**File**: `peer_example/main.go`  
**Complexity**: ⭐⭐⭐ Advanced  
**Duration**: 3 minutes

**Features**:
- 3-peer network
- Server + client modes
- Auto-handshake protocol
- Event handling
- Broadcasting
- Statistics

**Run**:
```bash
go run examples/peer_example/main.go
```

---

### 5. Full Mesh Network (`mesh_network/`)
**File**: `mesh_network/main.go`  
**Complexity**: ⭐⭐⭐ Advanced  
**Duration**: 5 minutes

**Features**:
- 3 peers in full mesh
- Each connects to all others
- Broadcast from each node
- Network statistics
- Auto-handshake

**Run**:
```bash
go run examples/mesh_network/main.go
```

---

### 6. Hub-and-Spoke (`hub_spoke/`)
**File**: `hub_spoke/main.go`  
**Complexity**: ⭐⭐⭐ Advanced  
**Duration**: 5 minutes

**Features**:
- Central hub (coordinator)
- Multiple spokes (workers)
- Hub relays messages
- Periodic broadcasts
- Worker status updates

**Run**:
```bash
go run examples/hub_spoke/main.go
```

---

### 7. Federated Learning (`federated_learning/`)
**File**: `federated_learning/main.go`  
**Complexity**: ⭐⭐⭐⭐ Expert  
**Duration**: Continuous

**Features**:
- Coordinator-worker architecture
- Model weight distribution
- Gradient aggregation
- Multi-round training simulation
- Matrix operations

**Run**:
```bash
go run examples/federated_learning/main.go
```

---

## 🎯 Example Use Cases

| Example | Use Case |
|---------|----------|
| **basic** | Learning UMICP basics |
| **websocket_example** | Simple client-server apps |
| **http_example** | REST-like HTTP communication |
| **peer_example** | P2P agent networks |
| **mesh_network** | Distributed systems |
| **hub_spoke** | Coordinator-worker patterns |
| **federated_learning** | ML training distribution |

---

## 🔧 Running Examples

### Run a specific example:
```bash
# Basic
go run examples/basic/main.go

# WebSocket
go run examples/websocket_example/main.go

# HTTP/2
go run examples/http_example/main.go

# Multiplexed Peer
go run examples/peer_example/main.go

# Mesh Network
go run examples/mesh_network/main.go

# Hub-and-Spoke
go run examples/hub_spoke/main.go

# Federated Learning
go run examples/federated_learning/main.go
```

### Run all examples:
```bash
make example-basic
make example-websocket
make example-peer
make example-http
```

---

## 📊 Example Complexity Guide

- ⭐ **Beginner**: Basic concepts, single file
- ⭐⭐ **Intermediate**: Client-server, event handling
- ⭐⭐⭐ **Advanced**: Multiple peers, topologies
- ⭐⭐⭐⭐ **Expert**: Complex workflows, ML use cases

---

**Total Examples**: 7  
**Total Lines**: ~1,500+  
**All Working**: ✅

